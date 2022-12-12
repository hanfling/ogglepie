/*
* OpenGL Example - Using binary SPIR-V shaders (GL_ARB_gl_spirv)
*
* Extension: https://www.opengl.org/registry/specs/ARB/gl_spirv.txt
* Supported GPUs: http://opengl.gpuinfo.org/gl_listreports.php?listreportsbyextension=GL_ARB_gl_spirv
* 
* Copyright (C) 2015 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <array>

#define GLEW_STATIC
#include "../3rd/glew/include/GL/glew.h"

#include "../3rd/glfw/WIN32/include/GLFW/glfw3.h" // Include file is identical for WIN32 and WIN64.

#define GLM_FORCE_RADIANS
#include "../3rd/glm/glm/glm.hpp"
#include "../3rd/glm/glm/gtc/matrix_transform.hpp"

const std::string appTitle = "OpenGL example - GL_ARB_gl_spirv";

class OpenGLExample
{
public:
	GLFWwindow* window;

	OpenGLExample(GLFWwindow *window)
	{
		this->window = window;
	}

	void draw(double deltaT)
	{
		double frameTimeStart = glfwGetTime();

		glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glfwSwapBuffers(window);
	}
};

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main()
{
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );

	GLFWwindow* window = glfwCreateWindow(1280, 720, appTitle.c_str(), NULL, NULL);
	
	if (!window)
	{
		std::cerr << "Failed to open GLFW window" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(window);

	//Set callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	//If GLEW hasn't initialized
	if (err != GLEW_OK)
	{
		std::cerr << "GLEW Error: " <<  glewGetErrorString(err) << std::endl;
		exit(-1);
	}

	std::cout << "GL_VENDOR:   " << glGetString(GL_VENDOR  ) << std::endl;
	std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "GL_VERSION:  " << glGetString(GL_VERSION ) << std::endl;

	GLint ImplementationColorReadType=0, ImplementationColorReadFormat=0;
	glGetIntegerv( GL_IMPLEMENTATION_COLOR_READ_TYPE,   &ImplementationColorReadType   );
	glGetIntegerv( GL_IMPLEMENTATION_COLOR_READ_FORMAT, &ImplementationColorReadFormat );

	//std::cout << std::boolalpha << "SPIRV supported: " << (GLEW_ARB_gl_spirv==GL_TRUE) << std::endl;

	if (!GLEW_ARB_gl_spirv)
	{
		std::cerr << "This examples requires support for SPIR-V (GL_ARB_gl_spirv)!" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	glfwSwapInterval(0);

	OpenGLExample example(window);

	glDisable(GL_CULL_FACE);

	double lastFPStime = glfwGetTime();
	double lastFrameTime = glfwGetTime();
	int frameCounter = 0;

	//Main Loop
	do
	{
		double thisFPStime = glfwGetTime();
		frameCounter++;

		if (thisFPStime - lastFPStime >= 1.0)
		{
			lastFPStime = thisFPStime;

			std::string windowTitle = appTitle +" (";
			windowTitle += std::to_string(frameCounter);
			windowTitle += " fps) - 2016 by Sascha Willems (www.saschawillems.de)";
			const char* windowCaption = windowTitle.c_str();
			glfwSetWindowTitle(window, windowCaption);

			frameCounter = 0;
		}

		double currTime = glfwGetTime();
		example.draw(currTime - lastFrameTime);
		lastFrameTime = currTime;

		//Get and organize events, like keyboard and mouse input, window resizing, etc...
		glfwPollEvents();


	} //Check if the ESC key had been pressed or if the window had been closed
	while (!glfwWindowShouldClose(window));

	//Close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	//Finalize and clean up GLFW
	glfwTerminate();

	exit(EXIT_SUCCESS);
}
