#include "../Utils/tgaimage.h"
#include "../Utils/model.h"
#include "Line.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

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
				Vec3f FaceVertex1 = ModelData.vert(FaceData[index]);
				Vec3f FaceVertex2 = ModelData.vert(FaceData[(index+1)%3]);
				Line::DrawLine((FaceVertex1.x+1.)*InWidth/2., (FaceVertex1.y + 1.)*InHeight / 2., 
					(FaceVertex2.x + 1.)*InWidth / 2., (FaceVertex2.y + 1.)*InHeight / 2., InImage, white);
			}
		}
	}
}

int main(int argc, char** argv) 
{
	TGAImage image(Width, Height, TGAImage::RGB);

	//DrawLineTest(image);
	DrawModelWireFrameTest(Width, Height, image);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");

	return 0;
}

