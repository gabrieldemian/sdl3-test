#version 460 core
#pragma shader_stage(fragment)

layout (location = 0) out vec3 color;

void main() {
	color =
		vec3(
			gl_FragCoord.x / 640,
			gl_FragCoord.y / 480,
			gl_FragCoord.z / 640
		);
}
