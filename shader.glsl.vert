#version 460 core
#pragma shader_stage(vertex)

vec4 pos[3] = {
	vec4(-0.5, -0.5, 0., 1.),
	vec4(0., 0.5, 0., 1),
	vec4(0.5, -0.5, 0., 1.),
};

void main() {
	gl_Position = pos[gl_VertexIndex];
	// gl_Position.y += sin(millis/1000. + gl_Position.x * 8.) / 8.;
}
