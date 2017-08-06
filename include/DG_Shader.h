#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include "DG_Mesh.h"
#include "DG_Shader.h"

namespace DG
{
	void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage)
	{
		GLint success = 0;
		GLchar error[1024] = { 0 };

		if (isProgram)
			glGetProgramiv(shader, flag, &success);
		else
			glGetShaderiv(shader, flag, &success);

		if (success == GL_FALSE)
		{
			if (isProgram)
				glGetProgramInfoLog(shader, sizeof(error), NULL, error);
			else
				glGetShaderInfoLog(shader, sizeof(error), NULL, error);

			std::cerr << errorMessage << ": '" << error << "'" << std::endl;
		}
	}

	// loads shader code from file
	std::string loadShaderCode(const std::string& filename) {
		std::ifstream file;
		file.open((filename).c_str());

		std::string shd_code;
		std::string line;

		if (file.is_open())
		{
			while (file.good())
			{
				std::getline(file, line);
				shd_code.append(line + "\n");
			}
		}
		else
		{
			std::cerr << "Unable to load shader: " << filename << std::endl;
		}

		return shd_code;
	}



	class Shader
	{
	public:

		Shader() {};

		Shader(const std::string vertexShaderFile,
			   const std::string fragmentShaderFile)
		{
			// read code from files
			std::string _vs_code = loadShaderCode(vertexShaderFile);
			std::string _fs_code = loadShaderCode(fragmentShaderFile);
			GLint vs_code_length = _vs_code.length();
			GLint fs_code_length = _fs_code.length();
			const char* vs_code = _vs_code.c_str();
			const char* fs_code = _fs_code.c_str();

			// vertex shader
			vs = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vs, 1, &vs_code, &vs_code_length);
			glCompileShader(vs);
			CheckShaderError(vs, GL_COMPILE_STATUS, false, "ERROR: Shader compilation failed!");

			// fragment shader
			fs = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fs, 1, &fs_code, &fs_code_length);
			glCompileShader(fs);
			CheckShaderError(fs, GL_COMPILE_STATUS, false, "ERROR: Shader compilation failed!");

			// create shader program
			shader_pgm = glCreateProgram();
			glAttachShader(shader_pgm, fs);
			glAttachShader(shader_pgm, vs);
			glLinkProgram(shader_pgm);
			CheckShaderError(shader_pgm, GL_LINK_STATUS, true, "ERROR: Shader linking failed!");
			glValidateProgram(shader_pgm);
			CheckShaderError(shader_pgm, GL_VALIDATE_STATUS, true, "ERROR: Shader program is invalid!");

			// uniforms
			// GLuint color_location_id = glGetUniformLocation(shader_pgm, "inputColor");
			mvp_id = glGetUniformLocation(shader_pgm, "modViewProj");
		}

		void bind()
		{
			glUseProgram(shader_pgm);
		}

		void update(Camera& camera, Primitive& primitive)
		{
			glm::mat4 mvp = camera.getProjection() * camera.getView() * primitive.transform.getModel();
			glUniformMatrix4fv(mvp_id, 1, GL_FALSE, glm::value_ptr(mvp));
		}

	private:

		GLuint vs = 0;
		GLuint fs = 0;
		GLuint shader_pgm = 0;
		GLuint mvp_id = 0;
	};

}


#endif

