#include "../Utils/tgaimage.h"
#include "../Utils/model.h"
#include "../Utils/geometry.h"

#define _USE_MATH_DEFINES // need to define to use M_PI.
#include <math.h>

#include "GL_Global.h"
#include "GL_Line.h"
#include "GL_Triangle.h"
#include "GL_Transform.h"
#include "GL_Shader.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

namespace
{
	int Width = 800;
	int Height = 800;

	Vec3f Camera(0, 0, 3);
	Vec3f Eye(1, 1, 3);
	Vec3f Center(0, 0, 0);

	//*************************************************************************
	// Line/Triangle/Model Draw Test
	//*************************************************************************

	void DrawPixelTest(TGAImage& InImage)
	{
		//draw pixel
		InImage.set(52, 41, red);
	}

	void DrawLineTest(TGAImage& InImage)
	{
		// draw line
		Line::DrawLine(13, 20, 80, 40, InImage, white);
		Line::DrawLine(20, 13, 40, 80, InImage, red);
		Line::DrawLine(80, 40, 13, 20, InImage, red);
	}

	void DrawModelWireFrameTest(int InWidth, int InHeight, TGAImage& InImage)
	{
		// parse model file .obj using utils class Model.
		Model ModelData("F:\\workdir\\personal\\Rasterizer\\Resource\\african_head.obj");
		for (int FaceIndex = 0; FaceIndex < ModelData.nfaces(); FaceIndex++)
		{
			std::vector<int> FaceData = ModelData.face(FaceIndex);
			// face data should contain 3 vertex
			// draw 3 lines of each face.
			for (int index = 0; index < 3; index++)
			{
				Vec3f FaceVertex0 = ModelData.vert(FaceData[index]);
				Vec3f FaceVertex1 = ModelData.vert(FaceData[(index+1)%3]);

				int X0 = (FaceVertex0.x + 1.)*InWidth / 2.;
				int Y0 = (FaceVertex0.y + 1.)*InHeight / 2.;
				int X1 = (FaceVertex1.x + 1.)*InWidth / 2.;
				int Y1 = (FaceVertex1.y + 1.)*InHeight / 2.;

				Line::DrawLine(X0, Y0, X1, Y1, InImage, white);
			}
		}
	}

	void DrawTriangleTest(TGAImage& InImage)
	{
		Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
		Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
		Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };

		//Line::DrawTriangle2D(t0[0], t0[1], t0[2], InImage, red);
		//Line::DrawTriangle2D(t1[0], t1[1], t1[2], InImage, white);
		//Line::DrawTriangle2D(t2[0], t2[1], t2[2], InImage, green);

		//Line::DrawAndFillTriangle2D_LineSweep(t0[0], t0[1], t0[2], InImage, red);
		//Line::DrawAndFillTriangle2D_LineSweep(t1[0], t1[1], t1[2], InImage, white);
		//Line::DrawAndFillTriangle2D_LineSweep(t2[0], t2[1], t2[2], InImage, green);
		
		Triangle::DrawAndFillTriangle2D_BoundingBox(t0, InImage, red);
		Triangle::DrawAndFillTriangle2D_BoundingBox(t1, InImage, white);
		Triangle::DrawAndFillTriangle2D_BoundingBox(t2, InImage, green);
	}

	void DrawModelFlatShadingTest(int InWidth, int InHeight, TGAImage& InImage)
	{
		// parse model file .obj using utils class Model.
		Model ModelData("F:\\workdir\\personal\\Rasterizer\\Resource\\african_head.obj");

		float* ZBuffer = new float[InWidth*InHeight];
		for (int Index = 0; Index < InWidth*InHeight; Index++)
		{
			ZBuffer[Index] = -std::numeric_limits<float>::max();
		}
		
		Vec3f LightDir(0, 0, -1);
		for (int FaceIndex = 0; FaceIndex < ModelData.nfaces(); FaceIndex++)
		{
			std::vector<int> FaceData = ModelData.face(FaceIndex);
			// face data should contain 3 vertex
			// draw 3 lines of each face.
			Vec3f Triangle[3];
			Vec3f TriangleWorld[3];
			for (int index = 0; index < 3; index++)
			{
				Vec3f FaceVertex0 = ModelData.vert(FaceData[index]);

				int X0 = (FaceVertex0.x + 1.)*InWidth / 2.;
				int Y0 = (FaceVertex0.y + 1.)*InHeight / 2.;

				Triangle[index] = Vec3f(X0, Y0, FaceVertex0.z);
				TriangleWorld[index] = FaceVertex0;
			}

			Vec3f Normal = cross(TriangleWorld[2] - TriangleWorld[0], TriangleWorld[1] - TriangleWorld[0]);
			Normal.normalize();

			// dot product is negative, remove the face (back-face culling).
			float Intensity = LightDir*Normal;
			if (Intensity > 0)
			{
				//Line::DrawAndFillTriangle2D_BoundingBox(Triangle, InImage,
				//	TGAColor(Intensity * 255, Intensity * 255, Intensity * 255, 255));

				Vec2f UV[3];
				for (int VertexIdx = 0; VertexIdx < 3; VertexIdx++)
				{
					UV[VertexIdx] = ModelData.uv(FaceIndex, VertexIdx);
				}

				Triangle::DrawAndFillTriangle3DWithZBuffer_BoundingBox(Triangle, UV, ZBuffer, InImage,
					TGAColor(Intensity * 255, Intensity * 255, Intensity * 255, 255), ModelData);
			}

			//// random color shading.
			//Line::DrawAndFillTriangle2D_BoundingBox(Triangle, InImage,
			//	TGAColor(std::rand() % 255, std::rand() % 255, std::rand() % 255, 255));
		}

		delete [] ZBuffer;
	}

	void Rasterize(Vec2i InP0, Vec2i InP1, TGAImage& InImage, TGAColor InColor, int* YBuffer)
	{
		if (InP0.x > InP1.x)
		{
			std::swap(InP0, InP1);
		}

		for (int X = InP0.x; X < InP1.x; X++)
		{
			int Y = float(InP1.y - InP0.y) / float(InP1.x - InP0.x)*(X - InP0.x) + InP0.y;
			if (Y > YBuffer[X])
			{
				YBuffer[X] = Y;
				//Line::DrawLine(InP0, InP1, InImage, InColor);
				InImage.set(X, InImage.get_height()/2, InColor);
			}
		}
	}

	// draw 1-D lines with Y buffer
	// assume we have 3 intersect planes, draw them on a plane/a line.
	// the key is to cache an array buffer represents current pixel Y value.
	// the initial value is minus infinity. each time we rasterize, we update 
	// this Y buffer if Y value of to-be-drawn pixel is larger than current Y buffer.
	void RasterizeWithYBufferTest(TGAImage& InImage)
	{
		//// scene "2d mesh"
		//Line::DrawLine(Vec2i(20, 34), Vec2i(744, 400), InImage, red);
		//Line::DrawLine(Vec2i(120, 434), Vec2i(444, 400), InImage, green);
		//Line::DrawLine(Vec2i(330, 463), Vec2i(594, 200), InImage, blue);

		//// screen line
		//Line::DrawLine(Vec2i(10, 10), Vec2i(790, 10), InImage, white);

		int YBuffer[800];

		// initial YBuffer to minus infinity.
		for (int index = 0; index < 800; index++)
		{
			YBuffer[index] = std::numeric_limits<int>::min();
		}

		// Do the rasterize.
		Rasterize(Vec2i(20, 34), Vec2i(744, 400), InImage, red, YBuffer);
		Rasterize(Vec2i(120, 434), Vec2i(444, 400), InImage, green, YBuffer);
		Rasterize(Vec2i(330, 463), Vec2i(594, 200), InImage, blue, YBuffer);
	}

	//*************************************************************************
	// Matrix Operation Test
	//*************************************************************************
	void MatrixOperationTest(TGAImage& InImage)
	{
		// draw axis
		Vec3f XAxis(1.f, 0.f, 0.f);
		Vec3f YAxis(0.f, 1.f, 0.f);
		Vec3f Origin(0.f, 0.f, 0.f);

		Matrix VPMatrix(Transform::Viewport(Width / 4, Height / 4, Width / 2, Height / 2));
		XAxis = Transform::Matrix2Vec(VPMatrix*Transform::Vec2Matrix(XAxis));
		YAxis = Transform::Matrix2Vec(VPMatrix*Transform::Vec2Matrix(YAxis));
		Origin = Transform::Matrix2Vec(VPMatrix*Transform::Vec2Matrix(Origin));

		Line::DrawLine(Origin.x, Origin.y, XAxis.x, XAxis.y, InImage, red);
		Line::DrawLine(Origin.x, Origin.y, YAxis.x, YAxis.y, InImage, green);

		// draw square
		Vec3f V1(0.5f, 0.5f, 0.f);
		Vec3f V2(0.5f, -0.5f, 0.f);
		Vec3f V3(-0.5f, -0.5f, 0.f);
		Vec3f V4(-0.5f, 0.5f, 0.f);

		// original square
		//V1 = Matrix2Vec(VPMatrix*Vec2Matrix(V1));
		//V2 = Matrix2Vec(VPMatrix*Vec2Matrix(V2));
		//V3 = Matrix2Vec(VPMatrix*Vec2Matrix(V3));
		//V4 = Matrix2Vec(VPMatrix*Vec2Matrix(V4));

		// square applied with basic transform
		// Important: Transform needs to be applied after VPMatrix
		Matrix Transform1(Transform::Translation(Vec3f(1, 0, 0)));
		Matrix Transform2(Transform::Scale(Vec3f(2, 2, 2)));
		Matrix Transform3(Transform::RotationZ(cos(M_PI / 180.f * 30), sin(M_PI / 180.f * 30)));

		// notice order of same transforms matters.
		// first translate then rotate is different from first rotate then translate!
		Matrix Transform = Transform3*Transform1;

		V1 = Transform::Matrix2Vec(VPMatrix*Transform*Transform::Vec2Matrix(V1));
		V2 = Transform::Matrix2Vec(VPMatrix*Transform*Transform::Vec2Matrix(V2));
		V3 = Transform::Matrix2Vec(VPMatrix*Transform*Transform::Vec2Matrix(V3));
		V4 = Transform::Matrix2Vec(VPMatrix*Transform*Transform::Vec2Matrix(V4));

		// draw square line
		Line::DrawLine(V1.x, V1.y, V2.x, V2.y, InImage, white);
		Line::DrawLine(V2.x, V2.y, V3.x, V3.y, InImage, white);
		Line::DrawLine(V3.x, V3.y, V4.x, V4.y, InImage, white);
		Line::DrawLine(V4.x, V4.y, V1.x, V1.y, InImage, white);
	}

	// considering vertex's z value of triangle.
	void DrawModelWithPerspectiveProjection(int InWidth, int InHeight, TGAImage& InImage)
	{
		// parse model file .obj using utils class Model.
		Model ModelData("F:\\workdir\\personal\\Rasterizer\\Resource\\african_head.obj");

		float* ZBuffer = new float[InWidth*InHeight];
		for (int Index = 0; Index < InWidth*InHeight; Index++)
		{
			ZBuffer[Index] = -std::numeric_limits<float>::max();
		}

		Matrix ModelView = Transform::LookAt(Eye, Center, Vec3f(0, 1, 0));
		Matrix VPMatrix(Transform::Viewport(Width / 4, Height / 4, Width / 2, Height / 2));
		// construct projection matrix
		Matrix Projection(Matrix::Identity(4));
		// perspective projection or make it orthogonal projection with identity
		//Projection[3][2] = -1. / Camera.z;
		Projection[3][2] = -1. / (Eye - Center).norm();

		Vec3f LightDir(0, 0, -1);
		for (int FaceIndex = 0; FaceIndex < ModelData.nfaces(); FaceIndex++)
		{
			std::vector<int> FaceData = ModelData.face(FaceIndex);
			// face data should contain 3 vertex
			// draw 3 lines of each face.
			Vec3f TriangleScreen[3];
			Vec3f TriangleWorld[3];
			for (int index = 0; index < 3; index++)
			{
				Vec3f FaceVertex = ModelData.vert(FaceData[index]);

				// original vertex's coordinates is between [-1,1], mapping to screen size(image size)
				// (vertex+1)/2->[0,1], and (vertex+1)/2*imagesize->[0,imagesize]
				//int X0 = (FaceVertex.x + 1.)*InWidth / 2.;
				//int Y0 = (FaceVertex.y + 1.)*InHeight / 2.;
				//TriangleScreen[index] = Vec3f(X0, Y0, FaceVertex.z);

				// transform world coordinates to screen coordinates with perspective projection.
				// transform: local to world by model matrix, then world to camera by projection matrix,
				// then camera to screen by viewport matrix.
				TriangleScreen[index] = Transform::Matrix2Vec(VPMatrix*Projection*ModelView*Transform::Vec2Matrix(FaceVertex));

				TriangleWorld[index] = FaceVertex;
			}

			// note here normal is based on triangle, if normal per vertex is given from model file,
			// no need to compute triangle per triangle, just use normal per vertex.
			Vec3f Normal = cross(TriangleWorld[2] - TriangleWorld[0], TriangleWorld[1] - TriangleWorld[0]);
			Normal.normalize();

			// dot product is negative, remove the face (back-face culling).
			float Intensity = LightDir*Normal;
			if (Intensity > 0)
			{
				Vec2f UV[3];
				for (int VertexIdx = 0; VertexIdx < 3; VertexIdx++)
				{
					UV[VertexIdx] = ModelData.uv(FaceIndex, VertexIdx);
				}

				Triangle::DrawAndFillTriangle3DWithZBuffer_BoundingBox(TriangleScreen, UV, ZBuffer, InImage,
					TGAColor(Intensity * 255, Intensity * 255, Intensity * 255, 255), ModelData);
			}
		}

		delete[] ZBuffer;
	}

	// refactor DrawModelWithPerspectiveProjection a bit.
	void DrawModelGouraudShading(int InWidth, int InHeight, TGAImage& InImage)
	{
		// parse model file .obj using utils class Model.
		Model ModelData("F:\\workdir\\personal\\Rasterizer\\Resource\\african_head.obj");

		float* ZBuffer = new float[InWidth*InHeight];
		for (int Index = 0; Index < InWidth*InHeight; Index++)
		{
			ZBuffer[Index] = -std::numeric_limits<float>::max();
		}

		Matrix ModelView = Transform::LookAt(Eye, Center, Vec3f(0, 1, 0));
		Matrix VPMatrix(Transform::Viewport(Width / 4, Height / 4, Width / 2, Height / 2));
		// construct projection matrix
		Matrix Projection(Transform::Projection(-1. / (Eye - Center).norm()));

		// how to decide light direction?
		// note light direction has nothing to do with the camera, it is set by user wish.
		// if camera is on z positive(out of screen), then light direction is (0, 0, -1) which point to screen.
		//Vec3f LightDir = Vec3f(0, 0, -1);
		//Vec3f LightDir = Vec3f(-1, -1, -3).normalize();
		Vec3f LightDir = Vec3f(1, -1, 1).normalize();

		// for each triangle in this model
		for (int FaceIndex = 0; FaceIndex < ModelData.nfaces(); FaceIndex++)
		{
			std::vector<int> FaceData = ModelData.face(FaceIndex);
			Vec3f TriangleScreen[3];
			Vec3f TriangleWorld[3];
			Vec2f UV[3];
			float Intensity[3];
			
			for (int VertexIdx = 0; VertexIdx < 3; VertexIdx++)
			{
				// for each vertex in this triangle, do the transform.
				Vec3f FaceVertex = ModelData.vert(FaceData[VertexIdx]);
				TriangleScreen[VertexIdx] = Transform::Matrix2Vec(VPMatrix*Projection*ModelView*Transform::Vec2Matrix(FaceVertex));
				TriangleWorld[VertexIdx] = FaceVertex;

				// compute vertex intensity.
				Vec3f VertexNormal = ModelData.norm(FaceIndex, VertexIdx);
				Intensity[VertexIdx] = LightDir*VertexNormal;

				// Note here intensity could be negative, in this case it should be zero.
				// handle it in TGAColor operation.
				//if (Intensity[VertexIdx] < 0.f)
				//	Intensity[VertexIdx] = 0.f;

				// uv
				UV[VertexIdx] = ModelData.uv(FaceIndex, VertexIdx);
			}

			// given triangle's transformed vertex/uv/intensity, draw.
			Triangle::DrawAndFillTriangle3D_GouraudShading(TriangleScreen, UV, Intensity, ZBuffer, InImage);
		}

		delete[] ZBuffer;
	}

	void DrawModelByShader(TGAImage& InImage)
	{
		// parse model file .obj using utils class Model.
		ModelData = new Model("F:\\workdir\\personal\\Rasterizer\\Resource\\african_head.obj");
		int InWidth = InImage.get_width();
		int InHeight = InImage.get_height();

		float* ZBuffer = new float[InWidth*InHeight];
		for (int Index = 0; Index < InWidth*InHeight; Index++)
		{
			ZBuffer[Index] = -std::numeric_limits<float>::max();
		}

		ModelView = Transform::LookAt(Eye, Center, Vec3f(0, 1, 0));
		VPMatrix = Transform::Viewport(Width / 4, Height / 4, Width / 2, Height / 2);
		Projection = Transform::Projection(-1. / (Eye - Center).norm());
		LightDir.normalize();

		Uniform_M = Projection*ModelView;
		Uniform_MIT = Uniform_M.Transpose().Inverse();

		//FlatShader Shader;
		//GouraudShader Shader;
		//ToonShader Shader;
		//GouraudShader_Diffuse Shader;
		//GouraudShader_NormalMapping Shader;
		PhongShader Shader;

		// for each triangle in this model
		for (int FaceIndex = 0; FaceIndex < ModelData->nfaces(); FaceIndex++)
		{
			std::vector<int> FaceData = ModelData->face(FaceIndex);
			Vec3f TriangleScreen[3];

			// call each vertex's vertex shader.
			for (int VertexIdx = 0; VertexIdx < 3; VertexIdx++)
			{
				TriangleScreen[VertexIdx] = Shader.Vertex(FaceIndex, VertexIdx);
			}

			// do the rasterization.
			Triangle::DrawAndFillTriangleWithShader(TriangleScreen, Shader, ZBuffer, InImage);
		}

		delete[] ZBuffer;
		delete ModelData;
	}
}

int main(int argc, char** argv) 
{
	TGAImage image(Width, Height, TGAImage::RGB);

	//DrawLineTest(image);
	//DrawModelWireFrameTest(Width, Height, image);
	//DrawTriangleTest(image);
	//DrawModelFlatShadingTest(Width, Height, image);
	//RasterizeWithYBufferTest(image);

	//MatrixOperationTest(image);
	//DrawModelWithPerspectiveProjection(Width, Height, image);
	//DrawModelGouraudShading(Width, Height, image);
	
	DrawModelByShader(image);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");

	return 0;
}

