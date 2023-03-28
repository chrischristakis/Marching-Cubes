#ifndef MARCHINGCUBES_H
#define MARCHINGCUBES_H
#include <vector>
#include <functional>
#include <glm/mat4x4.hpp>
#include "ShaderProgram.h"

namespace MarchingCubes {

	extern glm::vec3 base_color;

	void init(std::function<float(float, float, float)> f, float isovalue,
		float min, float max, float stepsize);

	void update();
	void render(ShaderProgram& shader, glm::mat4 mvp);

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		Vertex(glm::vec3 position, glm::vec3 normal) :
			position(position), normal(normal) {}
	};

	struct Triangle {
		Vertex v1, v2, v3;
	};
};

#endif