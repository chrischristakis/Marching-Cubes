#version 330 core

in vec3 fragpos_view;
in vec3 norm_view;

out vec4 fragColor;

uniform vec3 modelColor;
uniform vec3 lightDir;

void main() {
	
	//Ambient
	vec3 ambient = vec3(0.2, 0.2, 0.2);

	//Diffuse
	vec3 lightdir = normalize(-lightDir);  // Reflected, since direction is stated in the opposite way
	vec3 normal = normalize(norm_view);

	float diffuse_coefficient = max(0, dot(normal, lightdir));
	vec3 diffuse = modelColor * diffuse_coefficient;

	//Specular
	vec3 specularColor = vec3(1, 1, 1);
	vec3 reflection = reflect(-lightdir, normal);
	vec3 fragpos = normalize(-fragpos_view);
	float specular_coefficient = pow(max(0, dot(reflection, fragpos)), 64);
	vec3 specular = specular_coefficient * specularColor;

	vec3 resultColor = ambient + diffuse + specular;

	fragColor = vec4(resultColor, 1);
}