#pragma once

#include "../Utils/tgaimage.h"
#include <algorithm>

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

	// draw contour of triangle
	static void DrawTriangle2D(Vec2i InVert0, Vec2i InVert1, Vec2i InVert2, TGAImage &InImage, TGAColor InColor)
	{
		DrawLine(InVert0.x, InVert0.y, InVert1.x, InVert1.y, InImage, InColor);
		DrawLine(InVert1.x, InVert1.y, InVert2.x, InVert2.y, InImage, InColor);
		DrawLine(InVert2.x, InVert2.y, InVert0.x, InVert0.y, InImage, InColor);
	}

	// fill triangle by Line-Sweep algorithm.
	// 1. Sort vertices of the triangle by their y - coordinates;
	// 2. Rasterize simultaneously the left and the right sides of the triangle;
	// 3. Draw a horizontal line segment between the left and the right boundary points.
	static void DrawAndFillTriangle2D(Vec2i InVert0, Vec2i InVert1, Vec2i InVert2, TGAImage &InImage, TGAColor InColor)
	{
		// first attempt:
		/*
		// sort the vertices of triangle.
		// define 0 is top vertex(with max y value), 1 is left-bottom vertex, 2 is right-bottom vertex.
		if (InVert0.y < InVert1.y)
		{
			std::swap(InVert0, InVert1);
		}
		if (InVert0.y < InVert2.y)
		{
			std::swap(InVert0, InVert2);
		}
		if (InVert1.x > InVert2.x)
		{
			std::swap(InVert1, InVert2);
		}

		// draw triangle line segments. sort with y descent.
		std::vector<Vec2i> TopLeftLine = DrawLine(InVert0, InVert1, InImage, InColor);
		std::vector<Vec2i> TopRightLine = DrawLine(InVert0, InVert2, InImage, InColor);
		std::vector<Vec2i> LeftRightLine = DrawLine(InVert1, InVert2, InImage, InColor);
		std::sort(TopLeftLine.begin(), TopLeftLine.end(), [](Vec2i a, Vec2i b) {return a.y > b.y; });
		std::sort(TopRightLine.begin(), TopRightLine.end(), [](Vec2i a, Vec2i b) {return a.y > b.y; });
		std::sort(LeftRightLine.begin(), LeftRightLine.end(), [](Vec2i a, Vec2i b) {return a.y > b.y; });

		// draw horizontal line inside.
		if (InVert1.y >= InVert2.y)
		{
			for (int Y = TopLeftLine[0].y; Y > InVert1.y; Y--)
			{
				Vec2i Start, End;
				for (auto Vertex : TopLeftLine)
				{
					if (Vertex.y == Y)
					{
						Start = Vertex;
						break;
					}
				}
				for (auto Vertex : TopRightLine)
				{
					if (Vertex.y == Y)
					{
						End = Vertex;
						break;
					}
				}
				DrawLine(Start, End, InImage, InColor);
			}
			for (int Y = LeftRightLine[0].y; Y > InVert2.y; Y--)
			{
				Vec2i Start, End;
				for (auto Vertex : LeftRightLine)
				{
					if (Vertex.y == Y)
					{
						Start = Vertex;
						break;
					}
				}
				for (auto Vertex : TopRightLine)
				{
					if (Vertex.y == Y)
					{
						End = Vertex;
						break;
					}
				}
				DrawLine(Start, End, InImage, InColor);
			}
		}
		else
		{
			for (int Y = TopLeftLine[0].y; Y > InVert2.y; Y--)
			{
				Vec2i Start, End;
				for (auto Vertex : TopLeftLine)
				{
					if (Vertex.y == Y)
					{
						Start = Vertex;
						break;
					}
				}
				for (auto Vertex : TopRightLine)
				{
					if (Vertex.y == Y)
					{
						End = Vertex;
						break;
					}
				}
				DrawLine(Start, End, InImage, InColor);
			}
			for (int Y = LeftRightLine[0].y; Y > InVert1.y; Y--)
			{
				Vec2i Start, End;
				for (auto Vertex : LeftRightLine)
				{
					if (Vertex.y == Y)
					{
						Start = Vertex;
						break;
					}
				}
				for (auto Vertex : TopLeftLine)
				{
					if (Vertex.y == Y)
					{
						End = Vertex;
						break;
					}
				}
				DrawLine(Start, End, InImage, InColor);
			}
		}
		*/

		// the first attempt can work, but it complicates things.
		// it depends 3 contour line segments drawing to compute vertex on line, then draw horizontal lines.
		// another issue is that: we can apply line slope to calculate next pixel x, since we are increasing y by one pixel.
		// but here fill the triangle DOES NOT need to consider the gaps occurs in x-axis. because we are increase y by one pixel,
		// even it has big gaps for contour's x, we are draw horizontal lines to fill the region anyway, the gap will not appear.
		// so here is second attempt.
		
		// sort 3 vertices by y descent.
		// define 0 is top vertex.
		if (InVert0.y < InVert1.y)
		{
			std::swap(InVert0, InVert1);
		}
		if (InVert0.y < InVert2.y)
		{
			std::swap(InVert0, InVert2);
		}
		if (InVert1.y < InVert2.y)
		{
			std::swap(InVert1, InVert2);
		}

		int firstSegments = InVert0.y - InVert1.y;
		int index = 1;
		float Slope01 = float(InVert1.x - InVert0.x) / float(InVert1.y - InVert0.y);
		float Slope02 = float(InVert2.x - InVert0.x) / float(InVert2.y - InVert0.y);
		float Slope12 = float(InVert2.x - InVert1.x) / float(InVert2.y - InVert1.y);

		// Vert0-Vert1 segment
		for (int Y = InVert0.y - 1; Y >= InVert1.y; Y--)
		{
			// be careful with float to int rounding.
			int StartX = InVert0.x - Slope01*index;
			int EndX = InVert0.x - Slope02*index;
			DrawLine(StartX, Y, EndX, Y, InImage, InColor);
			index++;
		}
		// Vert1-Vert2 segment
		index = 1;
		for (int Y = InVert1.y - 1; Y > InVert2.y; Y--)
		{
			// be careful with float to int rounding.
			int StartX = InVert1.x - Slope12*index;
			int EndX = InVert0.x - Slope02*(firstSegments + index);
			DrawLine(StartX, Y, EndX, Y, InImage, InColor);
			index++;
		}
	}
};
