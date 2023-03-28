#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <thread>

#include "ShaderProgram.h"
#include "BoundingBox.h"
#include "Camera.h"
#include "MarchingCubes.h"

const int width = 1400, height = 1400;
const float min = -5, max = 5;

Camera camera(45, 45, 20, glm::vec3{ 0, 0, 0 });

float f1(float x, float y, float z) {
	return 0.25f*y - sin(x)*cos(z);
}

float f2(float x, float y, float z) {
	return sin(x) * cos(y) * sin(z);
}

std::map<int, bool> keys;  // maps keycode to pressed status

void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos) {
	camera.cursor_callback(window, xpos, ypos);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		keys[GLFW_KEY_UP] = true;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE)
		keys[GLFW_KEY_UP] = false;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		keys[GLFW_KEY_DOWN] = true;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE)
		keys[GLFW_KEY_DOWN] = false;
}

int main() {
	//Initialize our keys
	keys[GLFW_KEY_UP] = false;
	keys[GLFW_KEY_DOWN] = false;

	GLFWwindow *window;

	if (!glfwInit()) {
		std::cout << "GLFW failed to init" << std::endl;
		return -1;
	}

	window = glfwCreateWindow(width, height, "Marching cubes", NULL, NULL);

	if (!window) {
		std::cout << "Window failed to init" << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_cursor_callback);
	glfwSetKeyCallback(window, key_callback);

	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW failed to init" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	ShaderProgram bounding_shader("shaders/BoundingShader.vert", "shaders/BoundingShader.frag");
	ShaderProgram marching_shader("shaders/MarchingShader.vert", "shaders/MarchingShader.frag");
	BoundingBox boundingBox(min, max);

	std::thread t{ MarchingCubes::init, f1, 0, min, max, 0.065f};

	glm::mat4 proj = glm::perspective(45.0f, (float)width / height, 0.05f, 100.0f);
	glm::vec3 lightDir{ -1, -1, -1 };

	double current = glfwGetTime();
	double last = glfwGetTime();
	double delta = 0;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClearColor(0.08f, 0.09f, 0.11f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Delta time calculations
		current = glfwGetTime();
		delta = current - last;
		last = current;

		camera.update(delta);
		MarchingCubes::update();

		// Update the lighting shader's view matrix and light direction
		glUseProgram(marching_shader.ID);
		marching_shader.setUniformMatrix4fv("view", camera.getViewMatrix());
		marching_shader.setUniform3fv("lightDir", lightDir);

		glm::mat4 mvp = proj * camera.getViewMatrix();

		// Rendering
		boundingBox.render(bounding_shader, mvp);
		MarchingCubes::render(marching_shader, mvp);

		glfwSwapBuffers(window);
	}

	t.detach();
	glfwTerminate();
	return 1;
}