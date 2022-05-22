#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	GLuint program;
	/// <summary>
	/// Creates a shader program based on the provided file paths for the vertex and fragment shaders.
	/// </summary>
	/// <param name="vertexShaderFilePath">Vertex shader file path</param>
	/// <param name="fragmentShaderFilePath">Fragment shader file path</param>
	Shader(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, const std::string& geometryShaderFilePath = nullptr)
	{
		GLuint vertexShader = CreateShaderFromFile(GL_VERTEX_SHADER, vertexShaderFilePath);
		GLuint fragmentShader = CreateShaderFromFile(GL_FRAGMENT_SHADER, fragmentShaderFilePath);
		GLuint geometryShader = CreateShaderFromFile(GL_GEOMETRY_SHADER, geometryShaderFilePath);

		program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glAttachShader(program, geometryShader);

		glLinkProgram(program);

		glDetachShader(program, vertexShader);
		glDeleteShader(vertexShader);
		glDetachShader(program, fragmentShader);
		glDeleteShader(fragmentShader);
		glDetachShader(program, geometryShader);
		glDeleteShader(geometryShader);

		// Check shader program link status
		GLint linkStatus;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) 
		{
			char infoLog[512];
			GLsizei infoLogLen = sizeof(infoLog);
			glGetProgramInfoLog(program, infoLogLen, &infoLogLen, infoLog);
			std::cerr << "program link error: " << infoLog << std::endl;
		}
	}


	/// <summary>
	/// Creates a shader based on the provided shader type and the path to the file containing the shader source.
	/// </summary>
	/// <param name="shaderType">Shader type</param>
	/// <param name="shaderFilePath">Path to the file containing the shader source</param>
	/// <returns>OpenGL handle to the created shader</returns>
	GLuint CreateShaderFromFile(const GLuint& shaderType, const std::string& shaderFilePath)
	{
		std::ifstream shaderFile(shaderFilePath);
		if (shaderFile.fail())
		{
			std::cerr << "Unable to open shader file: " << shaderFilePath << std::endl;
			return 0;
		}

		std::string shaderSource;
		std::string temp;
		while (std::getline(shaderFile, temp))
		{
			shaderSource += temp + "\n";
		}
		shaderFile.close();

		return CreateShaderFromSource(shaderType, shaderSource);
	}

	/// <summary>
	/// Creates a shader based on the provided shader type and the string containing the shader source.
	/// </summary>
	/// <param name="shaderType">Shader type</param>
	/// <param name="shaderSource">Shader source string</param>
	/// <returns>OpenGL handle to the created shader</returns>
	GLuint CreateShaderFromSource(const GLuint& shaderType, const std::string& shaderSource)
	{
		GLuint shader = glCreateShader(shaderType);

		const char* shaderSourceCStr = shaderSource.c_str();
		GLint shaderSourceLen = static_cast<GLint>(shaderSource.length());
		glShaderSource(shader, 1, &shaderSourceCStr, &shaderSourceLen);
		glCompileShader(shader);

		// Check compilation status
		GLint compileStatus;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus == GL_FALSE)
		{
			char infoLog[512];
			GLsizei infoLogLen = sizeof(infoLog);
			glGetShaderInfoLog(shader, infoLogLen, &infoLogLen, infoLog);
			std::cerr << "shader compilation error: " << infoLog << std::endl;
		}

		return shader;
	}

	/// <summary>
	/// Switches current shader program to this.
	/// </summary>
	void use() 
	{
		glUseProgram(program);
	}

	/// <summary>
	/// Deletes the current program (for cleanup).
	/// </summary>
	void clean()
	{
		glDeleteProgram(program);
	}
};
#endif