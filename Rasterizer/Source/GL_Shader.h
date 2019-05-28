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
		// still compute light intensity per vertex.
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
class GouraudShader_Diffuse :public IShader
{
public:
	virtual ~GouraudShader_Diffuse() {};

	virtual Vec3f Vertex(int InFaceIndex, int InVertexIndex) override
	{
		Vec3f FaceVertex = ModelData->vert(InFaceIndex, InVertexIndex);
		FaceVertex = Transform::Matrix2Vec(VPMatrix*Projection*ModelView*Transform::Vec2Matrix(FaceVertex));
		Vec3f VertexNormal = ModelData->norm(InFaceIndex, InVertexIndex);
		// still compute light intensity per vertex.
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

// Gouraud Shader with normal mapping/specular mapping.
// previous method is to use normal data of each vertex and interpolate pixel's normal.
// if normal data of each pixel is stored in normal map, then we can use directly.
class GouraudShader_NormalMapping :public IShader
{
public:
	virtual ~GouraudShader_NormalMapping() {};

	virtual Vec3f Vertex(int InFaceIndex, int InVertexIndex) override
	{
		Vec3f FaceVertex = ModelData->vert(InFaceIndex, InVertexIndex);
		FaceVertex = Transform::Matrix2Vec(VPMatrix*Projection*ModelView*Transform::Vec2Matrix(FaceVertex));

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

		// transform normals from normal map
		// note normal map here is stored per pixel...so obtain pixel's normal directly and compute light intensity.
		Vec3f TransformNormal =  Transform::Matrix2Vec(Uniform_MIT*Transform::Vec2Matrix(ModelData->normal(InterpolatedUV))).normalize();
		Vec3f TransformLight = Transform::Matrix2Vec(Uniform_M*Transform::Vec2Matrix(LightDir)).normalize();

		// phong light model
		float AmbientLight = 5.;

		// specular mapping texture stores the value of each pixel's glossy factor.
		// compute reflected light
		Vec3f ReflectedLight = (TransformNormal*(TransformNormal*TransformLight*2.f) - TransformLight).normalize();
		float SpecularIntensity = std::pow(std::max(0.f, ReflectedLight.z), ModelData->specular(InterpolatedUV));
		// diffuse intensity
		float DiffuseIntensity = std::max(0.f, TransformNormal*TransformLight);

		for (int Idx = 0; Idx < 3; Idx++)
		{
			OutColor.bgra[Idx] = std::min<float>(AmbientLight + BaseColor.bgra[Idx] * (DiffuseIntensity + 0.6*SpecularIntensity), 255);
		}
		return false;
	}

private:
	Vec2f UVs[3];
};

// Phong Shading.
// Gouraud shading is calculate light per vertex and then do interpolation.
// Phong shading is calculate light per pixel.
class PhongShader :public IShader
{
public:
	virtual ~PhongShader() {};

	virtual Vec3f Vertex(int InFaceIndex, int InVertexIndex) override
	{
		Vec3f FaceVertex = ModelData->vert(InFaceIndex, InVertexIndex);
		FaceVertex = Transform::Matrix2Vec(VPMatrix*Projection*ModelView*Transform::Vec2Matrix(FaceVertex));

		Normals[InVertexIndex] = Transform::Matrix2Vec(Uniform_MIT*Transform::Vec2Matrix(ModelData->norm(InFaceIndex, InVertexIndex))).normalize();

		UVs[InVertexIndex] = ModelData->uv(InFaceIndex, InVertexIndex);
		return FaceVertex;
	}

	virtual bool Fragment(Vec3f InBarycentric, TGAColor& OutColor) override
	{
		// todo: can simplify the code by introduce matrix compuation here
		// (UVs-2*3matrix, then directly multiply with Vec3f-3*1matrix, thus InterpolatedUV-2*1matrix[vec2f]).
		Vec2f InterpolatedUV;
		InterpolatedUV.x = UVs[0].x * InBarycentric.x +
			UVs[1].x * InBarycentric.y +
			UVs[2].x * InBarycentric.z;
		InterpolatedUV.y = UVs[0].y * InBarycentric.x +
			UVs[1].y * InBarycentric.y +
			UVs[2].y * InBarycentric.z;

		// note here pixel's normal is interpolated, then used to compute light intensity.
		Vec3f InterpolatedNormal;
		InterpolatedNormal.x = Normals[0].x * InBarycentric.x +
			Normals[1].x * InBarycentric.y +
			Normals[2].x * InBarycentric.z;
		InterpolatedNormal.y = Normals[0].y * InBarycentric.x +
			Normals[1].y * InBarycentric.y +
			Normals[2].y * InBarycentric.z;
		InterpolatedNormal.z = Normals[0].z * InBarycentric.x +
			Normals[1].z * InBarycentric.y +
			Normals[2].z * InBarycentric.z;

		InterpolatedNormal.normalize();
		float Intensity = std::max(0.f, InterpolatedNormal*LightDir);

		TGAColor BaseColor = ModelData->diffuse(InterpolatedUV);
		OutColor = BaseColor*Intensity;

		return false;
	}

private:
	Vec2f UVs[3];
	Vec3f Normals[3];
};


// TODO:Phong Shading. using normal map tangent space.
// Gouraud shading is calculate light per vertex and then do interpolation.
// Phong shading is calculate light per pixel.
class PhongShader_TangentSpace :public IShader
{
public:
	virtual ~PhongShader_TangentSpace() {};

	virtual Vec3f Vertex(int InFaceIndex, int InVertexIndex) override
	{
		Vec3f FaceVertex = ModelData->vert(InFaceIndex, InVertexIndex);
		FaceVertex = Transform::Matrix2Vec(VPMatrix*Projection*ModelView*Transform::Vec2Matrix(FaceVertex));

		UVs[InVertexIndex] = ModelData->uv(InFaceIndex, InVertexIndex);
		return FaceVertex;
	}

	virtual bool Fragment(Vec3f InBarycentric, TGAColor& OutColor) override
	{
		return false;
	}

private:
	Vec2f UVs[3];
};

