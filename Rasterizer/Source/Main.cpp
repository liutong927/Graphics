#include "../Utils/tgaimage.h"
#include "../Utils/model.h"
#include "Line.h"
#include "../Utils/geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

namespace
{
	int Width = 800;
	int Height = 800;

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
					TGAColor(Intensity * 255, Intensity * 255, Intensity * 255, 255));
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
}

int main(int argc, char** argv) 
{
	TGAImage image(800, 800, TGAImage::RGB);

	//DrawLineTest(image);
	//DrawModelWireFrameTest(Width, Height, image);
	//DrawTriangleTest(image);
	DrawModelFlatShadingTest(800, 800, image);
	//RasterizeWithYBufferTest(image);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");

	return 0;
}

