#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H
#include <glm/mat4x4.hpp>
#include <vector>
#include "ShaderProgram.h"

class BoundingBox {
private:
	struct Vertex {
		float x, y, z;
		float r, g, b;
		Vertex(float x, float y, float z, float r, float g, float b) :
			x(x), y(y), z(z), r(r), g(g), b(b) {}
		Vertex(float x, float y, float z) :
			x(x), y(y), z(z), r(1), g(1), b(1) {}
	};

	struct Line {
		Vertex v1, v2;
	};

	GLuint VAO;
	std::vector<Line> lines;
public:
	BoundingBox(float min, float max);
	
	void render(ShaderProgram& shader, glm::mat4 mvp);
};

#endif