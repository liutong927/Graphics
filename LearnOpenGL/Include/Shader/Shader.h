#pragma once

#include <glad/glad.h>
#include <string>
#include <glm/fwd.hpp>

class Shader
{
public:
	// the program ID
	unsigned int ID = 0;

	// construct shader object to read from shader file and build them.
	Shader(const GLchar* InVertexPath, const GLchar* InFragmentPath);

	// activate the shader.
	void Use();

	void SetBool(const std::string& InName, bool InValue);
	void SetInt(const std::string& InName, int InValue);
	void SetFloat(const std::string& InName, float InValue);
	void SetMat4(const std::string& InName, glm::mat4 InValue);

private:

};
