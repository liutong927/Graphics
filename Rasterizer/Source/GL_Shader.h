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
		// this normal map is stored in model coordinates, NOT tangent space.
		// to get normal in projection space, we need to recompute normal, it is inverse transposed matrix to keep it still "normal".
		Vec3f TransformNormal =  Transform::Matrix2Vec(Uniform_MIT*Transform::Vec2Matrix(ModelData->normal(InterpolatedUV))).normalize();
		// for light vector, we apply projection transform to it, note it is different from normal vector transform.
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

		// store triangle's vertices in view space.
		VaryingTriangle[InVertexIndex] = Transform::Matrix2Vec(Uniform_M*Transform::Vec2Matrix(FaceVertex));

		FaceVertex = Transform::Matrix2Vec(VPMatrix*Uniform_M*Transform::Vec2Matrix(FaceVertex));

		// here stores vertex normals from view space.
		VaryingNormals[InVertexIndex] = Transform::Matrix2VecForV(Uniform_MIT*Transform::Vec2Matrix(ModelData->norm(InFaceIndex, InVertexIndex), 0.f)).normalize();

		VaryingUVs[InVertexIndex] = ModelData->uv(InFaceIndex, InVertexIndex);
		return FaceVertex;
	}

	virtual bool Fragment(Vec3f InBarycentric, TGAColor& OutColor) override
	{
		// todo: can simplify the code by introduce matrix computation here
		// (UVs-2*3matrix, then directly multiply with Vec3f-3*1matrix, thus InterpolatedUV-2*1matrix[vec2f]).
		Vec2f InterpolatedUV;
		InterpolatedUV.x = VaryingUVs[0].x * InBarycentric.x +
			VaryingUVs[1].x * InBarycentric.y +
			VaryingUVs[2].x * InBarycentric.z;
		InterpolatedUV.y = VaryingUVs[0].y * InBarycentric.x +
			VaryingUVs[1].y * InBarycentric.y +
			VaryingUVs[2].y * InBarycentric.z;

		// note here pixel's normal is interpolated, then used to compute light intensity.
		Vec3f InterpolatedNormal;
		InterpolatedNormal.x = VaryingNormals[0].x * InBarycentric.x +
			VaryingNormals[1].x * InBarycentric.y +
			VaryingNormals[2].x * InBarycentric.z;
		InterpolatedNormal.y = VaryingNormals[0].y * InBarycentric.x +
			VaryingNormals[1].y * InBarycentric.y +
			VaryingNormals[2].y * InBarycentric.z;
		InterpolatedNormal.z = VaryingNormals[0].z * InBarycentric.x +
			VaryingNormals[1].z * InBarycentric.y +
			VaryingNormals[2].z * InBarycentric.z;
		//InterpolatedNormal.normalize();

		// now we need transform pixel normal in normal map from tangent space to world space.
		// so first we need to know how tangent space basis(TBN coordinates) represented in world space.
		// then TBN matrix(3*3) [tranform tangent basis in world] multiply with Normal in tangent space, to get normal in world.

		// we have world vertex coordinates vt0, vt1, vt2, and its uv coordinates uv0, uv1, uv2.
		// note uv coordinates is defined in tangent space. u is along tangent direction, v is bitangent direction.
		// triangle's two edge (vt1-vt0) and (vt2-vt0) can be described in TBN coordinates. N is vertical to triangle's plane.
		// (vt1-vt0) = (u1-u0)*T + (v1-v0)*B + 0*N = (uv1.x-uv0.x)*T + (uv1.y-uv0.y)*B
		// (vt2-vt0) = (u2-u0)*T + (v2-v0)*B + 0*N = (uv2.x-uv0.x)*T + (uv2.y-uv0.y)*B
		// pixel's normal(InterpolatedNormal) is as N axis, so
		// InterpolatedNormal = 0*T + 0*B + N
		// now we can construct matrix to compute basis for TBN. and also since we need all vertices of this triangle,
		// we need to store them in vertex shader stage.
		// the matrix is like: 3 vector in world(3*3) = 3 vector in uv(tangent) space(3*3) * TBN basis in world(3*3)

		// we have triangle in world coordinates. Tri_W
		// we must normalize matrix, why?
		Matrix TriangleInWorld(3, 3);
		TriangleInWorld.SetRow(0, (VaryingTriangle[1] - VaryingTriangle[0]).normalize());
		TriangleInWorld.SetRow(1, (VaryingTriangle[2] - VaryingTriangle[0]).normalize());
		TriangleInWorld.SetRow(2, InterpolatedNormal.normalize());

		// also have triangle in tangent space. Tri_T
		Matrix TriangleTangent(3, 3);
		TriangleTangent.SetRow(0, Vec3f(VaryingUVs[1] - VaryingUVs[0]).normalize());
		TriangleTangent.SetRow(1, Vec3f(VaryingUVs[2] - VaryingUVs[0]).normalize());
		TriangleTangent.SetRow(2, Vec3f(0, 0, 1));// note here TBN is ortho coordinate, N axis is (0,0,1) in this frame.

		// Define TBN_toW, Tri_W = Tri_T * TBN_toW. // why not Tri_W =  TBN_toW * Tri_T????
		// TBN_toW = (Tri_T)^-1 * Tri_W.
		Matrix TBN(3, 3);
		TBN = TriangleTangent.Inverse()*TriangleInWorld;

		Matrix NormalInWorldM(1, 3);
		// compute normal data in world coordinate, note matrix multiplication order matters!!
		NormalInWorldM = Transform::Vec2Matrix13(ModelData->normal(InterpolatedUV))*TBN;
		Vec3f NormalInWorld(NormalInWorldM[0][0], NormalInWorldM[0][1], NormalInWorldM[0][2]);
		NormalInWorld.normalize();

		// don't forget to transform light to view space.
		Vec3f TransformLight = Transform::Matrix2VecForV(Uniform_M*Transform::Vec2Matrix(LightDir, 0.f)).normalize();
		float Intensity = std::max(0.f, NormalInWorld*TransformLight);
		//float Intensity = std::max(0.f, InterpolatedNormal*TransformLight);// this one using interpolated normal for pixel, but not use normal map data.
		TGAColor BaseColor = ModelData->diffuse(InterpolatedUV);
		//TGAColor BaseColor = TGAColor(255, 255, 255);
		OutColor = BaseColor*Intensity;

		// why after all this, we still get black triangle in model???

		return false;
	}

private:
	Vec2f VaryingUVs[3];
	Vec3f VaryingNormals[3];

	Vec3f VaryingTriangle[3];
};
