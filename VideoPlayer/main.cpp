#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>


#include "shader_manager.h"
#include "texture_2d.h"
#include "video_player.h"
#include "video_loader.h"
#include "glm/ext/matrix_clip_space.hpp"

bool setup();
void exitApp();

GLFWwindow *window;
int width = 1280;
int height = 720;

int main(int *argc, char *argv)
{	
	if (!setup()) exitApp();

	shader_manager &manager = shader_manager::get_instance();

	manager.load("Shaders/shader.vert", "Shaders/shader.frag");

	int video_width, video_height;
	video_loader loader;
	loader.open_file("Assets/coh2.mp4", &video_width, &video_height);

	uint8_t *data;
	loader.get_frame(&data);

	float vertices[] = {
		-1.0f, -1.0f, 0.0f,   0.0f, 1.0f, //0.0f, 0.0f, // sol alt
		 1.0f, -1.0f, 0.0f,   1.0f, 1.0f, //0.0f, 1.0f, // sað alt
		-1.0f,  1.0f, 0.0f,   0.0f, 0.0f, //0.0f, 1.0f,  // sol üst
		 1.0f,  1.0f, 0.0f,	  1.0f, 0.0f, //1.0f, 1.0f  // sað üst
	};

	uint32_t indices[] = {
		0, 1, 2,
		1, 3, 2
	};

	GLuint vao, vbo, ebo;

	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	texture_2d texture{};
	texture.load_texture(data, video_width, video_height);

	manager.use(0);
	manager.set_int("is_texture_bound", 1);
	manager.set_int("texture1", 0);

	glViewport(0, 0, video_width, video_height);

	do
	{
		glClear(GL_COLOR_BUFFER_BIT);

		loader.get_frame(&data);
		texture.load_texture(data, video_width, video_height);
		
		texture.bind();

		manager.use(0);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glfwSwapBuffers(window);
		glfwPollEvents();
	} while (!glfwWindowShouldClose(window));

	exitApp();
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	loader.close();

	return 0;
}

bool setup()
{
	if (!glfwInit())
	{
		std::cout << "Couldn't init glfw.";
		return false;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Video Player", nullptr, nullptr);

	if (!window)
	{
		std::cout << "Couldn't create window.";
		return false;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Couldn't init glew.";
		return false;
	}

	glViewport(0, 0, width, height);

	return true;
}

void exitApp()
{
	glfwTerminate();
	exit(-1);
}
