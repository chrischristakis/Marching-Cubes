#include "BoundingBox.h"

BoundingBox::BoundingBox(float min, float max) {

	// Create (x1,y1,x2,y2) lines for obunding box
	// x axis
	lines.emplace_back(Line{ Vertex{ min, min, min, 1, 0, 0 },    // The magic numbers are just RGB values
							 Vertex{ max, min, min, 1, 0, 0 } });
	// y axis
	lines.emplace_back(Line{ Vertex{ min, min, min, 0, 1, 0 },
							 Vertex{ min, max, min, 0, 1, 0 } });
	// z axis
	lines.emplace_back(Line{ Vertex{ min, min, min, 0.3f, 0.3f, 1 },
							 Vertex{ min, min, max, 0.3f, 0.3f, 1 } });
	// Remaining cube lines
	lines.emplace_back(Line{ Vertex{ min, min, max },
							 Vertex{ max, min, max } });
	lines.emplace_back(Line{ Vertex{ max, min, max },
							 Vertex{ max, min, min } });
	lines.emplace_back(Line{ Vertex{ min, min, max },
							 Vertex{ min, max, max } });
	lines.emplace_back(Line{ Vertex{ max, min, max },
							 Vertex{ max, max, max } });
	lines.emplace_back(Line{ Vertex{ max, min, min },
							 Vertex{ max, max, min } });
	lines.emplace_back(Line{ Vertex{ min, max, min },
							 Vertex{ min, max, max } });
	lines.emplace_back(Line{ Vertex{ min, max, max },
							 Vertex{ max, max, max } });
	lines.emplace_back(Line{ Vertex{ max, max, max },
							 Vertex{ max, max, min } });
	lines.emplace_back(Line{ Vertex{ max, max, min },
							 Vertex{ min, max, min } });

	// Now, generate VAO, VBO info for this box.
	GLuint VBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, lines.size() * 2 * sizeof(Vertex), &lines[0], GL_STATIC_DRAW);  // 2 verts per line.

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::r));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

const int AXIS_LINE_VERT_COUNT = 6; // 3 lines for axis lines, 2 verts per line
void BoundingBox::render(ShaderProgram& shader, glm::mat4 mvp) {
	glUseProgram(shader.ID);
	glBindVertexArray(VAO);

	shader.setUniformMatrix4fv("mvp", mvp);
	glLineWidth(5.0f);
	glDrawArrays(GL_LINES, 0, AXIS_LINE_VERT_COUNT);  // axis lines, which is first 3 lines, and 2 verts per line.
	glLineWidth(1.0f);
	glDrawArrays(GL_LINES, AXIS_LINE_VERT_COUNT, (lines.size() * 2) - AXIS_LINE_VERT_COUNT); // remaining lines

	glUseProgram(0);
	glBindVertexArray(0);
}