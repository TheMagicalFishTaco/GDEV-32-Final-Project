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
void mouse_input(GLFWwindow *window, double xPos, double yPos);
void scroll_zoom(GLFWwindow* window, double xOffset, double yOffset);
void processInput(GLFWwindow *window);

// camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float deltaTime = 0.0f;
float lastframe = 0.0f;
bool followCameraIsEnabled = false;

// mouse input variables
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
float fov = 45.0f;

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

	// hide the cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// call the mouse input method whenever the cursor is moved on the window
	glfwSetCursorPosCallback(window, mouse_input);

	//call the scroll zoom method whenever the user scrolls
	glfwSetScrollCallback(window, scroll_zoom);

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
	Model Moon("Models/Moon/scene.gltf");
	
	// WALL FOR SHADOW DEBUG
	// Model Wall("Models/Wall/scene.gltf");

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
	glBindTexture(GL_TEXTURE_CUBE_MAP, fboTex);

	GLuint shadowWidth = 1024, shadowHeight = 1024;
	// makes empty texture for each face of the cube map
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

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
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastframe;
		lastframe = currentFrame;

		processInput(window);

		// Clear the color and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float aspect = (float)shadowWidth / (float)shadowHeight;
		float near = 1.0f;
		float far = 50.0f;
		glm::vec3 lightPos = glm::vec3(0.0f);

		// makes projection matrices for each face of the cube map
		glm::mat4 projectionMatrixLight = glm::perspective(glm::radians(90.0f), aspect, near, far);
		std::vector<glm::mat4> viewMatrixLight;
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		viewMatrixLight.push_back(projectionMatrixLight *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		//FIRST PASS
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, shadowWidth, shadowHeight);
		shadowShader.use();
		

		// Passing shadow uniforms
		for (int i = 0; i < 6; ++i)
		{
			std::string shadowMatrixName = "shadowMatrices[" + std::to_string(i) + "]";
			const char* convertedShadowMatrixName = shadowMatrixName.c_str();
			GLint shadowMatrixUniformLocation = glGetUniformLocation(shadowShader.program, convertedShadowMatrixName);
			glUniformMatrix4fv(shadowMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(viewMatrixLight[i]));
		}

		GLint farPlaneUniformLocation = glGetUniformLocation(shadowShader.program, "farPlane");
		glUniform1f(farPlaneUniformLocation, far);
		GLint lightPosUniformLocation = glGetUniformLocation(shadowShader.program, "lightPos");
		glUniform3f(lightPosUniformLocation, 0.0f, 0.0f, 0.0f);
		GLint modelMatrixUniformLocation = glGetUniformLocation(shadowShader.program, "modelMatrix");
		
		// Avoid drawing Sun because it's not supposed to cast a shadow

		glm::mat4 modelMatrix = glm::mat4(1.0f);

		modelMatrix = glm::rotate(modelMatrix, glm::radians(float(5 * glfwGetTime())), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 5.0f));	
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-113.4f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(float(25 * glfwGetTime())), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(modelMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		
		Earth.Draw(shadowShader);

		modelMatrix = glm::mat4(1.0f);

		modelMatrix = glm::rotate(modelMatrix, glm::radians(float(5 * glfwGetTime())), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 5.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(float(25 * glfwGetTime())), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.75f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.15f, 0.15f, 0.15f));
		glUniformMatrix4fv(modelMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		Moon.Draw(shadowShader);

		// DEBUG WALL FOR SHADOWS
		// modelMatrix = glm::mat4(1.0f);
		// modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		// modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		// modelMatrix = glm::translate(modelMatrix, glm::vec3(-10.0f, 0.0f, 0.0f));
		// modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -5.0f));
		// glUniformMatrix4fv(modelMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		
		// Wall.Draw(shadowShader);
		

		//SECOND PASS
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_CUBE_MAP, fboTex);

		//---View Matrix---
		glm::mat4 viewMatrix;

		viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		//---Perspective Matrix---
		glm::mat4 perspectiveMatrix = glm::perspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1f, 150.0f);

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

		modelMatrix = glm::mat4(1.0f);

		modelMatrix = glm::rotate(modelMatrix, glm::radians(float(5 * glfwGetTime())), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 5.0f));	
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-113.4f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(float(25 * glfwGetTime())), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));

		modelMatrixUniformLocation = glGetUniformLocation(mainShader.program, "modelMatrix");
		glUniformMatrix4fv(modelMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		glm:: mat4 earthModelMatrix = modelMatrix;
		glm::mat4 mvpMatrix;
		mvpMatrix = perspectiveMatrix * viewMatrix * modelMatrix;

		GLint mvpMatrixUniformLocation = glGetUniformLocation(mainShader.program, "mvpMatrix");
		glUniformMatrix4fv(mvpMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));

		// Earth
		Earth.Draw(mainShader);

		//---Transformation Matrix for the Model (Moon)---
		modelMatrix = glm::mat4(1.0f);

		modelMatrix = glm::rotate(modelMatrix, glm::radians(float(5 * glfwGetTime())), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 5.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(float(25 * glfwGetTime())), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.75f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.15f, 0.15f, 0.15f));
		glUniformMatrix4fv(modelMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		mvpMatrix = perspectiveMatrix * viewMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));

		Moon.Draw(mainShader);

		// DEBUG WALL FOR SHADOWS
		// modelMatrix = glm::mat4(1.0f);
		// modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		// modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		// modelMatrix = glm::translate(modelMatrix, glm::vec3(-10.0f, 0.0f, 0.0f));
		// modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -5.0f));
		// glUniformMatrix4fv(modelMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		
		// mvpMatrix = perspectiveMatrix * viewMatrix * modelMatrix;
		// glUniformMatrix4fv(mvpMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));

		// Wall.Draw(mainShader);

		// Lighting uniforms
		GLint eyePosUniformLocation = glGetUniformLocation(mainShader.program, "eyePos");
		glUniform3f(eyePosUniformLocation, cameraPos.x,  cameraPos.y,  cameraPos.z);
		farPlaneUniformLocation = glGetUniformLocation(mainShader.program, "farPlane");
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

// Mouse Input
void mouse_input(GLFWwindow *window, double xpos, double ypos)
{
	// if it is the first time you receive mouse input, update the last x and y variables with the current cursor location
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	// calculate the offset between the last and current frame
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	// apply mouse sensitivity so it isn't too jittery
	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	// add to the global yaw and pitch variables
	yaw += xoffset;
	pitch += yoffset;

	// constrains the pitch so you can't look straight up or down
	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}

	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}

	// calculate the direction vectors and update the cameraFront variable accordingly
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void scroll_zoom(GLFWwindow* window, double xOffset, double yOffset)
{
	fov -= (float) yOffset;
	if (fov < 1.0f)
	{
		fov = 1.0f;
	}
	if (fov > 45.0f)
	{
		fov = 45.0f;
	}

}
// Keyboard input
void processInput(GLFWwindow* window)
{
	float cameraSpeed = 2.5f * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) && followCameraIsEnabled == false)
	{
		cameraPos += cameraSpeed * cameraFront;
	}
	if ((glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) && followCameraIsEnabled == false)
	{
		cameraPos -= cameraSpeed * cameraFront;
	}
	if ((glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) && followCameraIsEnabled == false)
	{
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if ((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) && followCameraIsEnabled == false)
	{
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		followCameraIsEnabled = !followCameraIsEnabled;
	}
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
