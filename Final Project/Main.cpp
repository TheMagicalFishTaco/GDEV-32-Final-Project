// Quick note: GLAD needs to be included first before GLFW.
// Otherwise, GLAD will complain about gl.h being already included.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

// Local header files for shaders and models
#include "Shader.h"
#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Necessary to import models
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
// ---------------
// Function declarations
// ---------------

/// <summary>
/// Function for handling the event when the size of the framebuffer changed.
/// </summary>
/// <param name="window">Reference to the window</param>
/// <param name="width">New width</param>
/// <param name="height">New height</param>
void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height);

/// <summary>
/// Main function.
/// </summary>
/// <returns>An integer indicating whether the program ended successfully or not.
/// A value of 0 indicates the program ended succesfully, while a non-zero value indicates
/// something wrong happened during execution.</returns>
int main()
{
	// Initialize GLFW
	int glfwInitStatus = glfwInit();
	if (glfwInitStatus == GLFW_FALSE)
	{
		std::cerr << "Failed to initialize GLFW!" << std::endl;
		return 1;
	}

	// Tell GLFW that we prefer to use OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Tell GLFW that we prefer to use the modern OpenGL
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Tell GLFW to create a window
	float windowWidth = 1366;
	float windowHeight = 768;
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Co Valenzuela Final Project", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
		return 1;
	}

	// Tell GLFW to use the OpenGL context that was assigned to the window that we just created
	glfwMakeContextCurrent(window);

	// Register the callback function that handles when the framebuffer size has changed
	glfwSetFramebufferSizeCallback(window, FramebufferSizeChangedCallback);

	// Tell GLAD to load the OpenGL function pointers
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		return 1;
	}

	// Create the shader programs
	Shader mainShader("main.vsh", "main.fsh");
	Shader lightShader("light.vsh", "light.fsh");
	Shader shadowShader("shadow.vsh", "shadow.fsh", "shadow.gsh");

	// Get the model(s)
	Model Earth("Models/Earth/scene.gltf");
	Model Sun("Models/Sun/scene.gltf");

	// Tell OpenGL the dimensions of the region where stuff will be drawn.
	// For now, tell OpenGL to use the whole screen
	glViewport(0, 0, windowWidth, windowHeight);
	
	// Test depth
	glEnable(GL_DEPTH_TEST);

	GLuint fbo;
	glGenFramebuffers(1, &fbo);


	//Binding texture to FBO
	GLuint fboTex;
	glGenTextures(1, &fboTex);
	glBindTexture(GL_TEXTURE_2D, fboTex);

	GLuint shadowWidth = 1024, shadowHeight = 1024;
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fboTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//To check status of FBO
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error! Framebuffer not complete!" << std::endl;
	}

	// Render loop
	while (!glfwWindowShouldClose(window))
	{
		// Clear the color and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float aspect = (float)shadowWidth / (float)shadowHeight;
		float near = 1.0f;
		float far = 25.0f;
		glm::vec3 lightPos = glm::vec3(0.0f);

		glm::mat4 projectionMatrixLight = glm::perspective(glm::radians(90.0f), aspect, near, far);
		std::vector<glm::mat4> viewMatrixLight;
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

		//FIRST PASS
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, 1024, 1024);
		shadowShader.use();
		
		for (unsigned int i = 0; i < 6; ++i)
		{
			//simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
			std::string shadowMatrixName = "shadowMatrices[" + std::to_string(i) + "]";
			GLint shadowMatrixUniformLocation = glGetUniformLocation(shadowShader.program, shadowMatrixName.c_str());
			glUniformMatrix4fv(shadowMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(viewMatrixLight[i]));
		}

		GLint farPlaneUniformLocation = glGetUniformLocation(mainShader.program, "farPlane");
		glUniform1f(farPlaneUniformLocation, far);
		GLint lightPosUniformLocation = glGetUniformLocation(mainShader.program, "lightPos");
		glUniform3f(lightPosUniformLocation, 0.0f, 0.0f, 0.0f);
		GLint eyePosUniformLocation = glGetUniformLocation(mainShader.program, "eyePos");
		glUniform3f(eyePosUniformLocation, 0.0f, 1.0f, 5.5f);

		//---View Matrix---
		glm::mat4 viewMatrix;

		viewMatrix = glm::lookAt(glm::vec3(0.0f, 1.0f, 5.5f),
								glm::vec3(0.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, 0.0f));

		//---Perspective Matrix---
		glm::mat4 perspectiveMatrix = glm::perspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 1.0f, 150.0f);

		// Shader Program for the Sun
		lightShader.use();

		//---Transformation Matrix for the Model (Sun)---

		glm::mat4 modelMatrixLight = glm::mat4(1.0f);
		modelMatrixLight = glm::scale(modelMatrixLight, glm::vec3(0.15f, 0.15f, 0.15f));
		modelMatrixLight = glm::rotate(modelMatrixLight, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 mvpMatrixLight;
		mvpMatrixLight = perspectiveMatrix * viewMatrix * modelMatrixLight;

		GLint mvpLightMatrixUniformLocation = glGetUniformLocation(lightShader.program, "mvpMatrixLight");
		glUniformMatrix4fv(mvpLightMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrixLight));
		
		// Sun
		Sun.Draw(lightShader);

		// Use the shader program that we created
		mainShader.use();

		//---Transformation Matrix for the Model (Earth)---

		glm::mat4 modelMatrix = glm::mat4(1.0f);

		modelMatrix = glm::rotate(modelMatrix, glm::radians(float(50*glfwGetTime())), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 3.0f));	
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-113.4f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(float(25 * glfwGetTime())), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));

		GLint modelMatrixUniformLocation = glGetUniformLocation(mainShader.program, "modelMatrix");
		glUniformMatrix4fv(modelMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		glm::mat4 mvpMatrix;
		mvpMatrix = perspectiveMatrix * viewMatrix * modelMatrix;

		GLint mvpMatrixUniformLocation = glGetUniformLocation(mainShader.program, "mvpMatrix");
		glUniformMatrix4fv(mvpMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));

		// Earth
		Earth.Draw(mainShader);

		// Uniform that passes lightColor needed for ambient lighting
		GLint lightColorUniformLocation = glGetUniformLocation(mainShader.program, "lightColor");
		glUniform3f(lightColorUniformLocation, 1.0f, 1.0f, 1.0f);

		// Uniform that passes lightPos needed for diffuse lighting
		GLint lightPosUniformLocation = glGetUniformLocation(mainShader.program, "lightPos");
		glUniform3f(lightPosUniformLocation, 0.0f, 0.0f, 0.0f);

		// Uniform that passes eyePosition needed for specular lighting
		GLint eyePosUniformLocation = glGetUniformLocation(mainShader.program, "eyePos");
		glUniform3f(eyePosUniformLocation, 0.0f, 1.0f, 5.5f);
		GLint farPlaneUniformLocation = glGetUniformLocation(mainShader.program, "farPlane");
		glUniform1f(farPlaneUniformLocation, far);

		GLint ambientUniformLocation = glGetUniformLocation(mainShader.program, "pointLight.ambient");
		glUniform3f(ambientUniformLocation, 0.1f, 0.1f, 0.1f);
		GLint pointLightDiffuseUniformLocation = glGetUniformLocation(mainShader.program, "pointLight.diffuse");
		glUniform3f(pointLightDiffuseUniformLocation, 1.0f, 1.0f, 1.0f);
		GLint pointLightSpecularUniformLocation = glGetUniformLocation(mainShader.program, "pointLight.specular");
		glUniform3f(pointLightSpecularUniformLocation, 0.5f, 0.5f, 0.5f);

		GLint pointLightPosUniformLocation = glGetUniformLocation(mainShader.program, "pointLight.position");
		glUniform3f(pointLightPosUniformLocation, 0.0f, 0.0f, 0.0f);


		// Tell GLFW to swap the screen buffer with the offscreen buffer
		glfwSwapBuffers(window);

		// Tell GLFW to process window events (e.g., input events, window closed events, etc.)
		glfwPollEvents();
	}

	// --- Cleanup ---
	// Make sure to delete the shader program
	mainShader.clean();
	lightShader.clean();

	// Remember to tell GLFW to clean itself up before exiting the application
	glfwTerminate();

	return 0;
}

/// <summary>
/// Function for handling the event when the size of the framebuffer changed.
/// </summary>
/// <param name="window">Reference to the window</param>
/// <param name="width">New width</param>
/// <param name="height">New height</param>
void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height)
{
	// Whenever the size of the framebuffer changed (due to window resizing, etc.),
	// update the dimensions of the region to the new size
	glViewport(0, 0, width, height);
}
