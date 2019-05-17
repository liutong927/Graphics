#include "../Utils/tgaimage.h"
#include "../Utils/model.h"
#include "Line.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

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

		Line::DrawAndFillTriangle2D(t0[0], t0[1], t0[2], InImage, red);
		Line::DrawAndFillTriangle2D(t1[0], t1[1], t1[2], InImage, white);
		Line::DrawAndFillTriangle2D(t2[0], t2[1], t2[2], InImage, green);
	}
}

int main(int argc, char** argv) 
{
	TGAImage image(200, 200, TGAImage::RGB);

	//DrawLineTest(image);
	//DrawModelWireFrameTest(Width, Height, image);
	DrawTriangleTest(image);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");

	return 0;
}

