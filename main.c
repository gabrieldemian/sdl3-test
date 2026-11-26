#include <stdio.h>
#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>

#define WIDTH  640
#define HEIGHT 480

static SDL_Window *window				 = NULL;
static SDL_GPUDevice *device			 = NULL;
static uint8_t *shader_vert_code		 = NULL;
static uint8_t *shader_frag_code		 = NULL;
static SDL_GPUGraphicsPipeline *pipeline = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
	SDL_SetAppMetadata("i hate graphics api", "1.0", "i hate graphics api");

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, "vulkan");
	if (!device) {
		SDL_Log("ERROR: Could not create GPU device: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	window = SDL_CreateWindow("myname", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
	if (!window) {
		SDL_Log("CreateWindow failed: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if (!SDL_ClaimWindowForGPUDevice(device, window)) {
		fprintf(stderr, "ERROR: Could not claim window for device\n");
	};

	size_t shader_vert_code_size;
	size_t shader_frag_code_size;

	shader_vert_code = SDL_LoadFile("shader.spv.vert", &shader_vert_code_size);
	shader_frag_code = SDL_LoadFile("shader.spv.frag", &shader_frag_code_size);

	const SDL_GPUShaderCreateInfo vertinfo = {
		.code_size	= shader_vert_code_size,
		.code		= shader_vert_code,
		.entrypoint = "main",
		.format		= SDL_GPU_SHADERFORMAT_SPIRV,
		.stage		= SDL_GPU_SHADERSTAGE_VERTEX,
	};
	const SDL_GPUShaderCreateInfo fraginfo = {
		.code_size	= shader_frag_code_size,
		.code		= shader_frag_code,
		.entrypoint = "main",
		.format		= SDL_GPU_SHADERFORMAT_SPIRV,
		.stage		= SDL_GPU_SHADERSTAGE_FRAGMENT,
	};

	SDL_GPUShader *vert_shader = SDL_CreateGPUShader(device, &vertinfo);
	SDL_GPUShader *frag_shader = SDL_CreateGPUShader(device, &fraginfo);

	const SDL_GPUGraphicsPipelineCreateInfo info = {
		.vertex_shader	 = vert_shader,
		.fragment_shader = frag_shader,
		.primitive_type	 = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.target_info	 = {
							.num_color_targets		   = 1,
							.color_target_descriptions = (SDL_GPUColorTargetDescription[]){
				{.format = SDL_GetGPUSwapchainTextureFormat(device, window)}}}
	  };

	pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);

	SDL_ReleaseGPUShader(device, vert_shader);
	SDL_ReleaseGPUShader(device, frag_shader);

	return SDL_APP_CONTINUE;
}

// Runs once per frame
SDL_AppResult SDL_AppIterate(void *appstate) {
	SDL_GPUCommandBuffer *cmd_buf = SDL_AcquireGPUCommandBuffer(device);
	if (!cmd_buf) {
		fprintf(stderr, "ERROR: Could not acquire GPU command buffer: %s\n",
				SDL_GetError());
	};

	SDL_GPUTexture *swapchaintex;

	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buf, window, &swapchaintex,
											   NULL, NULL)) {
		SDL_Log("WaitAndAcquire... failed: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if (!swapchaintex) {
		return SDL_APP_FAILURE;
	}

	SDL_GPUColorTargetInfo color_target_info = {0};
	color_target_info.texture				 = swapchaintex;
	color_target_info.load_op				 = SDL_GPU_LOADOP_CLEAR;
	color_target_info.store_op				 = SDL_GPU_STOREOP_STORE;
	color_target_info.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f};

	SDL_GPURenderPass *rp =
		SDL_BeginGPURenderPass(cmd_buf, &color_target_info, 1, NULL);

	SDL_BindGPUGraphicsPipeline(rp, pipeline);

	// --- draw ---

	SDL_DrawGPUPrimitives(rp, 3, 1, 0, 0);

	// --- draw ---

	SDL_EndGPURenderPass(rp);
	SDL_SubmitGPUCommandBuffer(cmd_buf);
	return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
	if (event->type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS;
	}
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
	SDL_ReleaseWindowFromGPUDevice(device, window);
	SDL_DestroyWindow(window);
	SDL_DestroyGPUDevice(device);
	SDL_free(shader_vert_code);
}
