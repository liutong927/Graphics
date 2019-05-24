#pragma once

#include <vector>
#include "../Utils/tgaimage.h"
#include "../Utils/geometry.h"

class Line
{
public:
	// Bresenham algorithm to draw line. Draw line is a necessity for draw wireframe of model.
	// not x0,y0 and x1,y1 are all integer pixel value, the goal is compute all pixel value(int) of this line.
	static std::vector<Vec2i> DrawLine(int InX0, int InY0, int InX1, int InY1, TGAImage &InImage, TGAColor InColor)
	{
		// first attempt, which depends on delta
		//for (float delta = 0.f; delta < 1.f; delta += .1f)
		//{
		//	int X = InX0 + (InX1 - InX0)*delta;
		//	int Y = InY0 + (InY1 - InY0)*delta;
		//	InImage.set(X, Y, InColor);
		//}

		// second attempt, increase x axis by pixel,
		// but it has fatal error for assume that x1 is bigger than x0.
		// so for line(10, 10, 20, 20) it can be drawn, but same line but reverse the direction (20, 20, 10, 10) it does
		// not execute!
		// another issue is if line's height is larger than width(, then percent applied to height(y axis) will causes bigger
		// gap than x-axis. since x increase by one pixel, it leaves gaps for y direction.
		//for (int X = InX0; X <= InX1; X++)
		//{
		//	float Percent = float(X - InX0) / float(InX1 - InX0);
		//	int Y = InY0 + (InY1 - InY0)*Percent;
		//	InImage.set(X, Y, InColor);
		//}

		// third attempt
		//bool bSteep = false;
		//// if line is steep, swap (x,y) to (y,x)
		//if (abs(InY1 - InY0) > abs(InX1 - InX0))
		//{
		//	bSteep = true;
		//	std::swap(InX0, InY0);
		//	std::swap(InX1, InY1);
		//}
		//// draw from left to right
		//if (InX0 > InX1)
		//{
		//	std::swap(InX0, InX1);
		//	std::swap(InY0, InY1);
		//}

		//// exec the second attempt
		//for (int X = InX0; X <= InX1; X++)
		//{
		//	float Percent = float(X - InX0) / float(InX1 - InX0);
		//	int Y = InY0 + (InY1 - InY0)*Percent;
		//	if (!bSteep)
		//	{
		//		InImage.set(X, Y, InColor);
		//	}
		//	else
		//	{
		//		InImage.set(Y, X, InColor);
		//	}
		//}

		// fourth attempt, introduce error
		// because Y is actually computed as a float, trancate it to int is not accurate.
		// e.g., if Y is 20.9f, it is more close to 21 pixel, but still kept as 20.
		bool bSteep = false;
		// bookkeeping pixels of this line.
		std::vector<Vec2i> LineVec;
		// if line is steep, swap (x,y) to (y,x)
		if (abs(InY1 - InY0) > abs(InX1 - InX0))
		{
			bSteep = true;
			std::swap(InX0, InY0);
			std::swap(InX1, InY1);
		}
		// draw from left to right
		if (InX0 > InX1)
		{
			std::swap(InX0, InX1);
			std::swap(InY0, InY1);
		}

		int DX = InX1 - InX0;
		int DY = InY1 - InY0;
		// DError represents increment of Y-direction for one X-direction pixel.
		// if x increase one pixel, y's increment is less than 0.5 pixel, then y's value should not change.
		// if it is more than 0.5 pixel, then it is more near to y+1 pixel.
		float DError = std::abs(float(DY) / float(DX));
		float Error = .0f;
		int Y = InY0;
		// increase one pixel in x-direction
		for (int X = InX0; X <= InX1; X++)
		{
			if (!bSteep)
			{
				LineVec.push_back(Vec2i(X, Y));
				InImage.set(X, Y, InColor);
			}
			else
			{
				LineVec.push_back(Vec2i(Y, X));
				InImage.set(Y, X, InColor);
			}

			// check if next pixel's should increase or not
			Error += DError;
			if (Error > .5f)
			{
				// if it is close to next pixel, then increase Y and reset Error.
				Y += InY1 > InY0 ? 1 : -1;
				Error -= 1.f;
			}
		}

		return LineVec;
	}

	static std::vector<Vec2i> DrawLine(Vec2i InVert0, Vec2i InVert1, TGAImage &InImage, TGAColor InColor)
	{
		return DrawLine(InVert0.x, InVert0.y, InVert1.x, InVert1.y, InImage, InColor);
	}
};
