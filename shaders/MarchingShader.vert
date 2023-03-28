#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 fragpos_view;
out vec3 norm_view;
out vec3 lightdir_view;

uniform mat4 mvp;
uniform mat4 view;

void main() {
	gl_Position = mvp * vec4(aPos, 1);

	// Lighting calculations (in view space)
	fragpos_view = vec3(view * vec4(aPos, 0)); // Transform into view space.
	norm_view = vec3(inverse(transpose(view)) * vec4(aNormal, 0)); // God I was stuck on this for a while, but w component must be 0 here, otherwise its translated.
}