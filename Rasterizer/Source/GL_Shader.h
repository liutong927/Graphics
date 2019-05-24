#pragma once
#include "..\Utils\geometry.h"
#include "..\Utils\tgaimage.h"

// Shader interface
class IShader
{
public:
	virtual ~IShader();
	// Vertex shader is to transform the coordinates of the vertices and prepare data for the fragment shader.
	// So Vertex shader is manipulate vertex of triangle.
	virtual Vec3f Vertex(int InFaceIndex, int InVertexIndex) = 0;
	// Fragment shader is to determine the color of the current pixel and discard current pixel by returning true.
	// Fragment shader is manipulate pixel's color.
	virtual bool Fragment(Vec3f InBarycentric, TGAColor& OutColor) = 0;
};

// Flat Shader
class FlatShader :public IShader
{
public:
	virtual ~FlatShader();

	virtual Vec3f Vertex(int InFaceIndex, int InVertexIndex) override
	{
		
	}

	virtual bool Fragment(Vec3f InBarycentric, TGAColor& OutColor) override
	{
		
	}
};

// Gouraud Shader
class GouraudShader :public IShader
{
public:
	virtual ~GouraudShader();

	virtual Vec3f Vertex(int InFaceIndex, int InVertexIndex) override
	{
	}

	virtual bool Fragment(Vec3f InBarycentric, TGAColor& OutColor) override
	{

	}
};

