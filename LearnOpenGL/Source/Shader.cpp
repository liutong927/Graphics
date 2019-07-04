#include <Shader/Shader.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace std;

namespace
{
	void CheckCompileErrors(unsigned int InShader, const string& InType)
	{
		int success;
		char infoLog[1024];

		if (InType == "PROGRAM")
		{
			glGetProgramiv(InShader, GL_LINK_STATUS, &success);
			if (!success) 
			{
				glGetProgramInfoLog(InShader, 1024, nullptr, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " <<
					InType << "\n" << infoLog << "\n -- --------------------------------------------------- -- " <<
					std::endl;
			}
		}
		else
		{
			glGetShaderiv(InShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(InShader, 1024, nullptr, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << 
					InType << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << 
					std::endl;
			}
		}
	}
}

Shader::Shader(const GLchar* InVertexPath, const GLchar* InFragmentPath)
{
	// read from shader file.
	string VertexCode;
	string FragmentCode;
	ifstream VShaderFile;
	ifstream FShaderFile;

	// ensure ifstream objects can throw exceptions:
	VShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
	FShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
	try
	{
		// open files
		VShaderFile.open(InVertexPath);
		FShaderFile.open(InFragmentPath);

		stringstream VShaderStream, FShaderStream;
		// read file's buffer contents into streams
		VShaderStream << VShaderFile.rdbuf();
		FShaderStream << FShaderFile.rdbuf();
		// close file handlers
		VShaderFile.close();
		FShaderFile.close();
		// convert stream into string
		VertexCode = VShaderStream.str();
		FragmentCode = FShaderStream.str();
	}
	catch (ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	const char* VShaderCode = VertexCode.c_str();
	const char* FShaderCode = FragmentCode.c_str();

	// compile shaders
	unsigned int VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &VShaderCode, nullptr);
	glCompileShader(VertexShader);
	CheckCompileErrors(VertexShader, "VERTEX");

	unsigned int FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &FShaderCode, nullptr);
	glCompileShader(FragmentShader);
	CheckCompileErrors(VertexShader, "FRAGMENT");

	// link shaders
	ID = glCreateProgram();
	glAttachShader(ID, VertexShader);
	glAttachShader(ID, FragmentShader);
	glLinkProgram(ID);
	CheckCompileErrors(ID, "PROGRAM");

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
}

void Shader::Use()
{
	glUseProgram(ID);
}

void Shader::SetBool(const std::string& InName, bool InValue)
{
	glUniform1i(glGetUniformLocation(ID, InName.c_str()), (int)InValue);
}

void Shader::SetInt(const std::string& InName, int InValue)
{
	glUniform1i(glGetUniformLocation(ID, InName.c_str()), InValue);
}

void Shader::SetFloat(const std::string& InName, float InValue)
{
	glUniform1f(glGetUniformLocation(ID, InName.c_str()), InValue);
}

void Shader::SetMat4(const std::string& InName, glm::mat4 InValue)
{
	glUniformMatrix4fv(glGetUniformLocation(ID, InName.c_str()), 1, GL_FALSE, glm::value_ptr(InValue));
}

