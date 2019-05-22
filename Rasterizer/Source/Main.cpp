#include "../Utils/tgaimage.h"
#include "../Utils/model.h"
#include "Line.h"
#include "../Utils/geometry.h"

#define _USE_MATH_DEFINES // need to define to use M_PI.
#include <math.h>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

namespace
{
	int Width = 800;
	int Height = 800;
	int Depth = 255;

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
		
		Line::DrawAndFillTriangle2D_BoundingBox(t0, InImage, red);
		Line::DrawAndFillTriangle2D_BoundingBox(t1, InImage, white);
		Line::DrawAndFillTriangle2D_BoundingBox(t2, InImage, green);
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

				Line::DrawAndFillTriangle3DWithZBuffer_BoundingBox(Triangle, UV, ZBuffer, InImage,
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

	// 4*1 matrix to vec3f.
	Vec3f Matrix2Vec(Matrix InMatrix)
	{
		return Vec3f(InMatrix[0][0] / InMatrix[3][0], InMatrix[1][0] / InMatrix[3][0], InMatrix[2][0] / InMatrix[3][0]);
	}

	// vec3f to 4*1 matrix.
	Matrix Vec2Matrix(Vec3f InVec)
	{
		Matrix Result(4, 1);
		Result[0][0] = InVec.x;
		Result[1][0] = InVec.y;
		Result[2][0] = InVec.z;
		Result[3][0] = 1.f;

		return Result;
	}

	//
	Matrix Viewport(int X, int Y, int Width, int Height)
	{
		Matrix Result(Matrix::Identity(4));

		Result[0][3] = X + Width / 2.f;
		Result[1][3] = Y + Height / 2.f;
		Result[2][3] = Depth / 2.f;

		Result[0][0] = Width / 2.f;
		Result[1][1] = Height / 2.f;
		Result[2][2] = Depth / 2.f;
		return Result;
	}

	// Vec3f convert to translation matrix
	Matrix Translation(Vec3f InVec)
	{
		Matrix Translation(Matrix::Identity(4));
		Translation[0][3] = InVec.x;
		Translation[1][3] = InVec.y;
		Translation[2][3] = InVec.z;

		return Translation;
	}

	// Vec3f convert to scale matrix
	Matrix Scale(Vec3f InVec)
	{
		Matrix Scale(Matrix::Identity(4));
		Scale[0][0] = InVec.x;
		Scale[1][1] = InVec.y;
		Scale[2][2] = InVec.z;

		return Scale;
	}

	// Zoom by apply same scaling.
	Matrix Zoom(float InFactor)
	{
		Vec3f ZoomVec(InFactor, InFactor, InFactor);
		return Scale(ZoomVec);
	}

	// Vec3f convert to rotation matrix
	// rotate along x-axis.
	Matrix RotationX(float InCosAngle, float InSinAngle)
	{
		Matrix Rotation(Matrix::Identity(4));

		Rotation[1][1] = Rotation[2][2] = InCosAngle;
		Rotation[1][2] = -InSinAngle;
		Rotation[2][1] = InSinAngle;

		return Rotation;
	}

	// rotate along y-axis.
	Matrix RotationY(float InCosAngle, float InSinAngle)
	{
		Matrix Rotation(Matrix::Identity(4));

		Rotation[0][0] = Rotation[2][2] = InCosAngle;
		Rotation[0][2] = -InSinAngle;
		Rotation[2][0] = InSinAngle;

		return Rotation;
	}

	// rotate along z-axis.
	Matrix RotationZ(float InCosAngle, float InSinAngle)
	{
		Matrix Rotation(Matrix::Identity(4));

		Rotation[0][0] = Rotation[1][1] = InCosAngle;
		Rotation[0][1] = -InSinAngle;
		Rotation[1][0] = InSinAngle;

		return Rotation;
	}

	void MatrixOperationTest(TGAImage& InImage)
	{
		// draw axis
		Vec3f XAxis(1.f, 0.f, 0.f);
		Vec3f YAxis(0.f, 1.f, 0.f);
		Vec3f Origin(0.f, 0.f, 0.f);

		Matrix VPMatrix(Viewport(Width / 4, Height / 4, Width / 2, Height / 2));
		XAxis = Matrix2Vec(VPMatrix*Vec2Matrix(XAxis));
		YAxis = Matrix2Vec(VPMatrix*Vec2Matrix(YAxis));
		Origin = Matrix2Vec(VPMatrix*Vec2Matrix(Origin));

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
		Matrix Transform1(Translation(Vec3f(1, 0, 0)));
		Matrix Transform2(Scale(Vec3f(2, 2, 2)));
		Matrix Transform3(RotationZ(cos(M_PI / 180.f * 30), sin(M_PI / 180.f * 30)));

		// notice order of same transforms matters.
		// first translate then rotate is different from first rotate then translate!
		Matrix Transform = Transform3*Transform1;

		V1 = Matrix2Vec(VPMatrix*Transform*Vec2Matrix(V1));
		V2 = Matrix2Vec(VPMatrix*Transform*Vec2Matrix(V2));
		V3 = Matrix2Vec(VPMatrix*Transform*Vec2Matrix(V3));
		V4 = Matrix2Vec(VPMatrix*Transform*Vec2Matrix(V4));

		// draw square line
		Line::DrawLine(V1.x, V1.y, V2.x, V2.y, InImage, white);
		Line::DrawLine(V2.x, V2.y, V3.x, V3.y, InImage, white);
		Line::DrawLine(V3.x, V3.y, V4.x, V4.y, InImage, white);
		Line::DrawLine(V4.x, V4.y, V1.x, V1.y, InImage, white);
	}

	// considering vertex's z value of triangle.
	void DrawModelWithPerspectiveProjection(int InWidth, int InHeight, TGAImage& InImage)
	{

	}
}

int main(int argc, char** argv) 
{
	TGAImage image(800, 800, TGAImage::RGB);

	//DrawLineTest(image);
	//DrawModelWireFrameTest(Width, Height, image);
	//DrawTriangleTest(image);
	//DrawModelFlatShadingTest(800, 800, image);
	//RasterizeWithYBufferTest(image);

	MatrixOperationTest(image);
	//DrawModelWithPerspectiveProjection(800, 800, image);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");

	return 0;
}

