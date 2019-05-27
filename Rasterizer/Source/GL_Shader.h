#pragma once
#include "..\Utils\geometry.h"
#include "..\Utils\tgaimage.h"
#include "GL_Global.h"
#include <algorithm>
#include "GL_Transform.h"

// Shader interface
class IShader
{
public:
	virtual ~IShader() {};
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
	virtual ~FlatShader() {};

	virtual Vec3f Vertex(int InFaceIndex, int InVertexIndex) override
	{
		Vec3f FaceVertex = ModelData->vert(InFaceIndex, InVertexIndex);
		VaryingTriangle[InVertexIndex] = FaceVertex;
		FaceVertex = Transform::Matrix2Vec(VPMatrix*Projection*ModelView*Transform::Vec2Matrix(FaceVertex));

		return FaceVertex;
	}

	virtual bool Fragment(Vec3f InBarycentric, TGAColor& OutColor) override
	{		
		Vec3f FaceNormal = cross(VaryingTriangle[2] - VaryingTriangle[0], VaryingTriangle[1] - VaryingTriangle[0]).normalize();
		float InterpolatedIntensity = std::max(0.f, LightDir*FaceNormal);
		OutColor = TGAColor(255, 255, 255)*InterpolatedIntensity;
		return false;
	}

private:
	Vec3f VaryingIntensity;
	Vec3f VaryingTriangle[3]; // triangel's vertex to compute face normal(instead of using vertex normal) to get same color of this face (flat shading).
};

// Gouraud Shader
class GouraudShader :public IShader
{
public:
	virtual ~GouraudShader() {};

	virtual Vec3f Vertex(int InFaceIndex, int InVertexIndex) override
	{
		Vec3f FaceVertex = ModelData->vert(InFaceIndex, InVertexIndex);
		FaceVertex = Transform::Matrix2Vec(VPMatrix*Projection*ModelView*Transform::Vec2Matrix(FaceVertex));
		Vec3f VertexNormal = ModelData->norm(InFaceIndex, InVertexIndex);
		VaryingIntensity.raw[InVertexIndex] = std::max(0.f, LightDir*VertexNormal);
		return FaceVertex;
	}

	virtual bool Fragment(Vec3f InBarycentric, TGAColor& OutColor) override
	{
		float InterpolatedIntensity = VaryingIntensity*InBarycentric;
		OutColor = TGAColor(255, 255, 255)*InterpolatedIntensity;
		return false;
	}

private:
	// computed by vertex shader, read by fragment shader.
	// each intensity value of triangle's vertex.
	Vec3f VaryingIntensity;
};

// Toon Shader
class ToonShader :public IShader
{
public:
	virtual ~ToonShader() {};

	virtual Vec3f Vertex(int InFaceIndex, int InVertexIndex) override
	{
		Vec3f FaceVertex = ModelData->vert(InFaceIndex, InVertexIndex);
		FaceVertex = Transform::Matrix2Vec(VPMatrix*Projection*ModelView*Transform::Vec2Matrix(FaceVertex));
		Vec3f VertexNormal = ModelData->norm(InFaceIndex, InVertexIndex);
		VaryingIntensity.raw[InVertexIndex] = std::max(0.f, LightDir*VertexNormal);
		return FaceVertex;
	}

	virtual bool Fragment(Vec3f InBarycentric, TGAColor& OutColor) override
	{
		float InterpolatedIntensity = VaryingIntensity*InBarycentric;
		if (InterpolatedIntensity > .85)
		{
			InterpolatedIntensity = 1;
		}
		else if (InterpolatedIntensity > 0.6)
		{
			InterpolatedIntensity = 0.80;
		}
		else if (InterpolatedIntensity > 0.45)
		{
			InterpolatedIntensity = 0.6;
		}
		else if (InterpolatedIntensity > 0.30)
		{
			InterpolatedIntensity = 0.45;
		}
		else if (InterpolatedIntensity > 0.15)
		{
			InterpolatedIntensity = 0.3;
		}
		else
		{
			InterpolatedIntensity = 0.f;
		}

		OutColor = TGAColor(255, 155, 0)*InterpolatedIntensity;
		return false;
	}

private:
	Vec3f VaryingIntensity;
};

// Gouraud Shader with uv
class FullShader :public IShader
{
public:
	virtual ~FullShader() {};

	virtual Vec3f Vertex(int InFaceIndex, int InVertexIndex) override
	{
		Vec3f FaceVertex = ModelData->vert(InFaceIndex, InVertexIndex);
		FaceVertex = Transform::Matrix2Vec(VPMatrix*Projection*ModelView*Transform::Vec2Matrix(FaceVertex));
		Vec3f VertexNormal = ModelData->norm(InFaceIndex, InVertexIndex);
		VaryingIntensity.raw[InVertexIndex] = std::max(0.f, LightDir*VertexNormal);

		UVs[InVertexIndex] = ModelData->uv(InFaceIndex, InVertexIndex);

		return FaceVertex;
	}

	virtual bool Fragment(Vec3f InBarycentric, TGAColor& OutColor) override
	{
		Vec2f InterpolatedUV;
		InterpolatedUV.x = UVs[0].x * InBarycentric.x +
			UVs[1].x * InBarycentric.y +
			UVs[2].x * InBarycentric.z;
		InterpolatedUV.y = UVs[0].y * InBarycentric.x +
			UVs[1].y * InBarycentric.y +
			UVs[2].y * InBarycentric.z;

		TGAColor BaseColor = ModelData->diffuse(InterpolatedUV);

		float InterpolatedIntensity = VaryingIntensity*InBarycentric;
		OutColor = BaseColor*InterpolatedIntensity;
		return false;
	}

private:
	// computed by vertex shader, read by fragment shader.
	// each intensity value of triangle's vertex.
	Vec3f VaryingIntensity;
	Vec2f UVs[3];
};

