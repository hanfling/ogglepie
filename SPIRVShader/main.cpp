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

// SPIR-V Binaries.
#include "CompiledShaders/VertexShader.h"
#include "CompiledShaders/FragmentShader.h"

const std::string appTitle = "OpenGL example - GL_ARB_gl_spirv";

struct 
{
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
} uboVS;

class OpenGLExample
{
public:
	GLuint Program;
	GLuint VAO;
	GLuint VBO[2];
	GLuint UBO;
	float zoom = -2.0f;
	glm::vec3 rotation = glm::vec3(0.0f);
	GLFWwindow* window;

	OpenGLExample(GLFWwindow *window)
	{
		this->window = window;
	}

	void PrintShaderInfoLog( GLuint shader, const char* Label )
	{
		GLint result = GL_FALSE;
		int logLength;

		glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) 
		{
			GLchar* strInfoLog = new GLchar[logLength + 1];
			glGetShaderInfoLog(shader, logLength, NULL, strInfoLog);

			if ( logLength==1 && *strInfoLog=='\0' )
			{
				std::cout << "!! Single '\\0' character ShaderInfoLog (" << Label << ")" << std::endl;
			}
			else
			{
				std::cout << "ShaderInfolog (" << Label << "): " << strInfoLog << std::endl;
			}
		};
	}

	void PrintProgramInfoLog(GLuint program)
	{
		GLint result = GL_FALSE;
		int logLength;

		glGetProgramiv(program, GL_LINK_STATUS, &result);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		if ( logLength>0 ) 
		{
			GLchar* strInfoLog = new GLchar[logLength + 1];
			glGetProgramInfoLog(program, logLength, NULL, strInfoLog);

			if ( logLength==1 && *strInfoLog=='\0' )
			{
				std::cout << "!! Single '\\0' character ProgramInfoLog." << std::endl;
			}
			else
			{
				std::cout << "ProgramInfoLog: " << strInfoLog << std::endl;
			}
		};
	}

	GLint InitShader( GLenum ShaderType, const uint32_t* ShaderBinary, GLsizei ShaderBinaryLength, GLuint& Shader, const char* ShaderName )
	{
		Shader = glCreateShader( ShaderType );
		glObjectLabel( GL_SHADER, Shader, -1, ShaderName );

		glShaderBinary( 1, &Shader, GL_SHADER_BINARY_FORMAT_SPIR_V, ShaderBinary, ShaderBinaryLength );
		glSpecializeShader( Shader, "main", 0, nullptr, nullptr );

		GLint CompileStatus;
		glGetShaderiv( Shader, GL_COMPILE_STATUS, &CompileStatus );
		if ( CompileStatus!=GL_TRUE )
		{
			std::cerr << "Failed to compile " << ShaderName << std::endl;
		}

		PrintShaderInfoLog( Shader, ShaderName );

		return CompileStatus;
	}

	GLuint InitProgram( const uint32_t* VertexShaderBinary, GLsizei VertexShaderBinaryLength, const uint32_t* FragmentShaderBinary, GLsizei FragmentShaderBinaryLength )
	{
		GLuint vertShader, fragShader;

		GLint result = GL_TRUE;

		result &= InitShader( GL_VERTEX_SHADER,   VertexShaderBinary,   VertexShaderBinaryLength,   vertShader, "VertexShader"   );
		result &= InitShader( GL_FRAGMENT_SHADER, FragmentShaderBinary, FragmentShaderBinaryLength, fragShader, "FragmentShader" );

		if (!result)
		{
			std::cerr << "Could not load all binary shaders required for this program" << std::endl;
			return GL_FALSE;
		}

		//std::cout << "Linking shader program" << std::endl;
		GLuint program = glCreateProgram();
		glAttachShader( program, vertShader );
		glAttachShader( program, fragShader );

		glDeleteShader( vertShader );
		glDeleteShader( fragShader );

		glLinkProgram(program);
		PrintProgramInfoLog(program);

		glDetachShader( program, vertShader );
		glDetachShader( program, fragShader );

		return program;
	}

	void loadAssets()
	{
		Program = InitProgram( VertexShaderBinary, sizeof(VertexShaderBinary), FragmentShaderBinary, sizeof(FragmentShaderBinary) );
	}

	void updateUBO()
	{
		// Update ubo
		// Update matrices
		uboVS.projection = glm::perspective(glm::radians(60.0f), (float)1280 / (float)720, 0.1f, 256.0f);

		uboVS.view = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, zoom));

		uboVS.model = glm::mat4(1.0);
		uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glNamedBufferSubData( UBO, 0, sizeof(uboVS), &uboVS );
	}

	void generateBuffers()
	{
		// Setup vertex data
		const GLfloat vPos[] =
		{
			1.0f, -1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			0.0f,  1.0f, 0.0f
		};
		const GLfloat vCol[] =
		{
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f
		};

		glCreateBuffers(2, VBO);

		// Position
		glNamedBufferStorage( VBO[0], sizeof(vPos), &vPos, 0 );

		// Color
		glNamedBufferStorage( VBO[1], sizeof(vCol), &vCol, 0 );

		// Uniform buffer object
		glCreateBuffers( 1, &UBO );
		glNamedBufferStorage( UBO, sizeof(uboVS), &uboVS, GL_DYNAMIC_STORAGE_BIT );

		// VAO.
		glCreateVertexArrays( 1, &VAO );
		glObjectLabel( GL_VERTEX_ARRAY, VAO, -1, "TriangleVAO" );

		glEnableVertexArrayAttrib( VAO, 0);
		glVertexArrayAttribFormat( VAO, 0, 3, GL_FLOAT, GL_FALSE, 0 );
		glVertexArrayAttribBinding(	VAO, 0, 0 );
		glVertexArrayVertexBuffer( VAO, 0, VBO[0], 0, 3*sizeof(GLfloat) );

		glEnableVertexArrayAttrib( VAO, 1 );
		glVertexArrayAttribFormat( VAO, 1, 3, GL_FLOAT, GL_FALSE, 0 );
		glVertexArrayAttribBinding(	VAO, 1, 1 );
		glVertexArrayVertexBuffer( VAO, 1, VBO[1], 0, 3*sizeof(GLfloat) );

		// UBO.
		updateUBO();
	}

	void draw(double deltaT)
	{
		double frameTimeStart = glfwGetTime();

		glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_DEPTH_TEST);
		glBindBufferBase( GL_UNIFORM_BUFFER, 0, UBO );
		glUseProgram( Program );
		glBindVertexArray( VAO );
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);

		rotation.y += (float)(deltaT * 50.0f);
		updateUBO();
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
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
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

	//std::cout << std::boolalpha << "SPIRV supported: " << (GLEW_ARB_gl_spirv==GL_TRUE) << std::endl;

	if (!GLEW_ARB_gl_spirv)
	{
		std::cerr << "This examples requires support for SPIR-V (GL_ARB_gl_spirv)!" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	// Check whether GL_SHADER_BINARY_FORMAT_SPIR_V is listed inside GL_NUM_SHADER_BINARY_FORMATS.
	bool HasSpirvShaderBinaryFormat = false;
	GLint NumShaderBinaryFormats=0;
	glGetIntegerv( GL_NUM_SHADER_BINARY_FORMATS, &NumShaderBinaryFormats );
	if ( NumShaderBinaryFormats>0 )
	{
		GLint* ShaderBinaryFormats = new GLint[NumShaderBinaryFormats];
		glGetIntegerv( GL_SHADER_BINARY_FORMATS, ShaderBinaryFormats );

		for ( INT iShaderBinaryFormat=0; iShaderBinaryFormat<NumShaderBinaryFormats; iShaderBinaryFormat++ )
		{
			if ( ShaderBinaryFormats[iShaderBinaryFormat]==GL_SHADER_BINARY_FORMAT_SPIR_V )
			{
				HasSpirvShaderBinaryFormat = true;
				break;
			}
		}
	}
	// Put a warning here, as this would have been an oversight in the ICD...
	if ( !HasSpirvShaderBinaryFormat )
	{
		std::cout << "!! SHADER_BINARY_FORMAT_SPIR_V is not include in SHADER_BINARY_FORMATS." << std::endl;
	}

	glfwSwapInterval(0);

	OpenGLExample example(window);

	example.generateBuffers();
	example.loadAssets();

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
