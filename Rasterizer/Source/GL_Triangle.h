#pragma once
#pragma once

#include <algorithm>
#include "GL_Line.h"
#include "GL_Shader.h"

class Triangle
{
public:
	// draw contour of triangle
	static void DrawTriangle2D(Vec2i InVert0, Vec2i InVert1, Vec2i InVert2, TGAImage &InImage, TGAColor InColor)
	{
		Line::DrawLine(InVert0.x, InVert0.y, InVert1.x, InVert1.y, InImage, InColor);
		Line::DrawLine(InVert1.x, InVert1.y, InVert2.x, InVert2.y, InImage, InColor);
		Line::DrawLine(InVert2.x, InVert2.y, InVert0.x, InVert0.y, InImage, InColor);
	}

	// fill triangle by Line-Sweep algorithm.
	// 1. Sort vertices of the triangle by their y - coordinates;
	// 2. Rasterize simultaneously the left and the right sides of the triangle;
	// 3. Draw a horizontal line segment between the left and the right boundary points.
	static void DrawAndFillTriangle2D_LineSweep(Vec2i InVert0, Vec2i InVert1, Vec2i InVert2, TGAImage &InImage, TGAColor InColor)
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
			Line::DrawLine(StartX, Y, EndX, Y, InImage, InColor);
			index++;
		}
		// Vert1-Vert2 segment
		index = 1;
		for (int Y = InVert1.y - 1; Y > InVert2.y; Y--)
		{
			// be careful with float to int rounding.
			int StartX = InVert1.x - Slope12*index;
			int EndX = InVert0.x - Slope02*(firstSegments + index);
			Line::DrawLine(StartX, Y, EndX, Y, InImage, InColor);
			index++;
		}
	}

	// find barycentric coordinates of point P regarding triangle ABC.
	static Vec3f ComputeBarycentric(Vec2i* InPoints, Vec2i InP)
	{
		Vec3f U = cross(Vec3f(InPoints[2].x - InPoints[0].x, InPoints[1].x - InPoints[0].x, InPoints[0].x - InP.x),
			Vec3f(InPoints[2].y - InPoints[0].y, InPoints[1].y - InPoints[0].y, InPoints[0].y - InP.y));
		// InPoints and InP has integer value as coordinates.
		// so if abs(u.z)<1 means abs(u[2])=0?
		if (std::abs(U.z) < 1)
		{
			return Vec3f(-1, 1, 1);
		}

		return Vec3f(1.f - (U.x + U.y) / U.z, U.y / U.z, U.x / U.z);
	}

	static Vec3f ComputeBarycentric3D(Vec3f* InPoints, Vec3f InP)
	{
		Vec3f U = cross(Vec3f(InPoints[2].x - InPoints[0].x, InPoints[1].x - InPoints[0].x, InPoints[0].x - InP.x),
			Vec3f(InPoints[2].y - InPoints[0].y, InPoints[1].y - InPoints[0].y, InPoints[0].y - InP.y));
		// InPoints and InP has integer value as coordinates.
		// so if abs(u.z)<1 means abs(u[2])=0?
		if (std::abs(U.z) < 1e-2)
		{
			return Vec3f(-1, 1, 1);
		}

		return Vec3f(1.f - (U.x + U.y) / U.z, U.y / U.z, U.x / U.z);
	}

	// fill triangle by bounding-box algorithm.
	// iterate through all pixels of a bounding box for a given triangle. 
	// For each pixel we compute its barycentric coordinates. If it has at least one negative component, then the pixel is outside of the triangle.
	static void DrawAndFillTriangle2D_BoundingBox(Vec2i* InVert, TGAImage &InImage, TGAColor InColor)
	{
		// find bounding box of triangle by give 3 points.
		// a bounding box is defined by 2 points: bottom left and upper right of box containing triangle.
		// to find these corner points, iterate through 3 vertices of the triangle and choose min/max coordinates.
		Vec2i BBoxMin(0, 0);
		Vec2i BBoxMax(InImage.get_width(), InImage.get_height());

		// find triangle vertices' min X/Y and max X/Y
		// also need to consider triangle may out of image box.
		std::vector<Vec2i> Triangle;
		for (int index = 0; index < 3; index++)
		{
			Triangle.push_back(InVert[index]);
		}

		std::sort(Triangle.begin(), Triangle.end(), [](Vec2i a, Vec2i b) {return a.y > b.y; });
		BBoxMax.y = std::min(InImage.get_height(), Triangle[0].y);
		BBoxMin.y = std::max(0, Triangle[2].y);

		std::sort(Triangle.begin(), Triangle.end(), [](Vec2i a, Vec2i b) {return a.x > b.x; });
		BBoxMax.x = std::min(InImage.get_width(), Triangle[0].x);
		BBoxMin.x = std::max(0, Triangle[2].x);

		// for each pixel in this bounding box, test point if it is inside triangle, if yes draw pixel.
		for (int X = BBoxMin.x; X < BBoxMax.x; X++)
		{
			for (int Y = BBoxMin.y; Y < BBoxMax.y; Y++)
			{
				Vec3f BarycentricVec = ComputeBarycentric(InVert, Vec2i(X, Y));

				if (BarycentricVec.x < 0 || BarycentricVec.y < 0 || BarycentricVec.z < 0)
				{
					continue;
				}

				InImage.set(X, Y, InColor);
			}
		}
	}

	// Fill triangle with z buffer.
	static void DrawAndFillTriangle3DWithZBuffer_BoundingBox(Vec3f* InVert, Vec2f* InUVs, float* InZBuffer, TGAImage &InImage, TGAColor InColor, Model& InModel)
	{
		// find bounding box of triangle by give 3 points.
		// a bounding box is defined by 2 points: bottom left and upper right of box containing triangle.
		// to find these corner points, iterate through 3 vertices of the triangle and choose min/max coordinates.
		Vec2f BBoxMin(0, 0);
		Vec2f BBoxMax(InImage.get_width(), InImage.get_height());

		// find triangle vertices' min X/Y and max X/Y
		// also need to consider triangle may out of image box.
		std::vector<Vec3f> Triangle;
		for (int index = 0; index < 3; index++)
		{
			Triangle.push_back(InVert[index]);
		}

		// find min/max x/y of 3 vertices of this triangle
		std::sort(Triangle.begin(), Triangle.end(), [](Vec3f a, Vec3f b) {return a.y > b.y; });
		BBoxMax.y = std::min((float)InImage.get_height(), Triangle[0].y);
		BBoxMin.y = std::max(0.f, Triangle[2].y);

		std::sort(Triangle.begin(), Triangle.end(), [](Vec3f a, Vec3f b) {return a.x > b.x; });
		BBoxMax.x = std::min((float)InImage.get_width(), Triangle[0].x);
		BBoxMin.x = std::max(0.f, Triangle[2].x);

		// for each pixel in this bounding box, test point if it is inside triangle, if yes draw pixel.
		for (int X = BBoxMin.x; X < BBoxMax.x; X++)
		{
			for (int Y = BBoxMin.y; Y < BBoxMax.y; Y++)
			{
				Vec3f CurrentPoint(X, Y, 0);
				Vec3f BarycentricVec = ComputeBarycentric3D(InVert, CurrentPoint);

				if (BarycentricVec.x < 0 || BarycentricVec.y < 0 || BarycentricVec.z < 0)
				{
					continue;
				}

				// below handle pixel inside this triangle.
				// compute each pixel's uv by Interpolation. How to Compute?
				// we already the point inside triangle represent by 3 vertices(barycentric coordinates).
				// so the uv of this point should also have same scaling in this coordinate system.
				// get diffuse color by uv value. 
				// this works. Brilliant! the interpolation method is better than original.
				Vec2f InterpolatedUV;
				InterpolatedUV.x = InUVs[0].x * BarycentricVec.x +
					InUVs[1].x * BarycentricVec.y +
					InUVs[2].x * BarycentricVec.z;
				InterpolatedUV.y = InUVs[0].y * BarycentricVec.x +
					InUVs[1].y * BarycentricVec.y +
					InUVs[2].y * BarycentricVec.z;

				// take into account z-buffer.
				// z-buffer is 2-dimension(width*height), make it 1D array.
				// compute pixel's z value by barycentric coordinates multiply triangle's vertice's z value.
				// note BarycentricVec is current pixel's barycentric coordinates, here we want current pixel's z value,
				// so use 3 vertex's z value multiply with its barycentric value.
				CurrentPoint.z = InVert[0].z*BarycentricVec.x +
					InVert[1].z*BarycentricVec.y +
					InVert[2].z*BarycentricVec.z;

				int CurrentPointZBufferIndex = InImage.get_width()*Y + X;
				if (InZBuffer[CurrentPointZBufferIndex] < CurrentPoint.z)
				{
					InZBuffer[CurrentPointZBufferIndex] = CurrentPoint.z;
					// pass Model as argument of this function to fetch diffuse data.
					TGAColor Color = InModel.diffuse(InterpolatedUV);
					// pass in color to get original intensity by InColor/255.
					InImage.set(X, Y, TGAColor(Color.bgra[0] * InColor.bgra[0] / 255, Color.bgra[1] * InColor.bgra[1] / 255, Color.bgra[2] * InColor.bgra[2] / 255));
					//InImage.set(X, Y, InColor);
				}
			}
		}
	}

	// refactor DrawAndFillTriangle3DWithZBuffer_BoundingBox to interpolate light intensity of pixel.
	// input: one triangle's vertex/uv/intensity.
	static void DrawAndFillTriangle3D_GouraudShading(Vec3f* InVert, Vec2f* InUVs, float* InIntensity, float* InZBuffer, TGAImage &InImage)
	{
		// find bounding box of triangle by give 3 points.
		// a bounding box is defined by 2 points: bottom left and upper right of box containing triangle.
		// to find these corner points, iterate through 3 vertices of the triangle and choose min/max coordinates.
		Vec2f BBoxMin(0, 0);
		Vec2f BBoxMax(InImage.get_width(), InImage.get_height());

		// find triangle vertices' min X/Y and max X/Y
		// also need to consider triangle may out of image box.
		std::vector<Vec3f> Triangle;
		for (int index = 0; index < 3; index++)
		{
			Triangle.push_back(InVert[index]);
		}

		// find min/max x/y of 3 vertices of this triangle
		std::sort(Triangle.begin(), Triangle.end(), [](Vec3f a, Vec3f b) {return a.y > b.y; });
		BBoxMax.y = std::min((float)InImage.get_height(), Triangle[0].y);
		BBoxMin.y = std::max(0.f, Triangle[2].y);

		std::sort(Triangle.begin(), Triangle.end(), [](Vec3f a, Vec3f b) {return a.x > b.x; });
		BBoxMax.x = std::min((float)InImage.get_width(), Triangle[0].x);
		BBoxMin.x = std::max(0.f, Triangle[2].x);

		// for each pixel in this bounding box, test point if it is inside triangle, if yes draw pixel.
		for (int X = BBoxMin.x; X < BBoxMax.x; X++)
		{
			for (int Y = BBoxMin.y; Y < BBoxMax.y; Y++)
			{
				Vec3f CurrentPoint(X, Y, 0);
				Vec3f BarycentricVec = ComputeBarycentric3D(InVert, CurrentPoint);

				if (BarycentricVec.x < 0 || BarycentricVec.y < 0 || BarycentricVec.z < 0)
				{
					continue;
				}

				// below handle pixel inside this triangle.
				// interpolate pixel uv.
				Vec2f InterpolatedUV;
				InterpolatedUV.x = InUVs[0].x * BarycentricVec.x +
					InUVs[1].x * BarycentricVec.y +
					InUVs[2].x * BarycentricVec.z;
				InterpolatedUV.y = InUVs[0].y * BarycentricVec.x +
					InUVs[1].y * BarycentricVec.y +
					InUVs[2].y * BarycentricVec.z;

				// interpolate pixel intensity. this allows smooth face color.
				float InterpolatedIntensity = InIntensity[0] * BarycentricVec.x +
					InIntensity[1] * BarycentricVec.y +
					InIntensity[2] * BarycentricVec.z;

				// interpolate pixel z buffer
				CurrentPoint.z = InVert[0].z*BarycentricVec.x +
					InVert[1].z*BarycentricVec.y +
					InVert[2].z*BarycentricVec.z;

				int CurrentPointZBufferIndex = InImage.get_width()*Y + X;
				if (InZBuffer[CurrentPointZBufferIndex] < CurrentPoint.z)
				{
					InZBuffer[CurrentPointZBufferIndex] = CurrentPoint.z;
					InImage.set(X, Y, TGAColor(255, 255, 255) * InterpolatedIntensity);
				}
			}
		}
	}

	// refactor DrawAndFillTriangle3D_GouraudShading to do triangle rasterization for arbitary shader. 
	static void DrawAndFillTriangleWithShader(Vec3f* InScreenVert, IShader& InShader, float* InZBuffer, TGAImage &InImage)
	{
		// find bounding box of triangle by give 3 points.
		// a bounding box is defined by 2 points: bottom left and upper right of box containing triangle.
		// to find these corner points, iterate through 3 vertices of the triangle and choose min/max coordinates.
		Vec2f BBoxMin(0, 0);
		Vec2f BBoxMax(InImage.get_width(), InImage.get_height());

		// find triangle vertices' min X/Y and max X/Y
		// also need to consider triangle may out of image box.
		std::vector<Vec3f> Triangle;
		for (int index = 0; index < 3; index++)
		{
			Triangle.push_back(InScreenVert[index]);
		}

		// find min/max x/y of 3 vertices of this triangle
		std::sort(Triangle.begin(), Triangle.end(), [](Vec3f a, Vec3f b) {return a.y > b.y; });
		BBoxMax.y = std::min((float)InImage.get_height(), Triangle[0].y);
		BBoxMin.y = std::max(0.f, Triangle[2].y);

		std::sort(Triangle.begin(), Triangle.end(), [](Vec3f a, Vec3f b) {return a.x > b.x; });
		BBoxMax.x = std::min((float)InImage.get_width(), Triangle[0].x);
		BBoxMin.x = std::max(0.f, Triangle[2].x);

		// for each pixel in this bounding box, test point if it is inside triangle, if yes draw pixel.
		for (int X = BBoxMin.x; X < BBoxMax.x; X++)
		{
			for (int Y = BBoxMin.y; Y < BBoxMax.y; Y++)
			{
				Vec3f CurrentPoint(X, Y, 0);
				Vec3f BarycentricVec = ComputeBarycentric3D(InScreenVert, CurrentPoint);
				int CurrentPointZBufferIndex = InImage.get_width()*Y + X;

				// interpolate pixel z buffer
				CurrentPoint.z = InScreenVert[0].z*BarycentricVec.x +
					InScreenVert[1].z*BarycentricVec.y +
					InScreenVert[2].z*BarycentricVec.z;

				if (BarycentricVec.x < 0 || BarycentricVec.y < 0 || BarycentricVec.z < 0
					|| InZBuffer[CurrentPointZBufferIndex] > CurrentPoint.z)
				{
					continue;
				}

				TGAColor PixelColor;
				bool bDiscard = InShader.Fragment(BarycentricVec, PixelColor);
				if (!bDiscard)
				{
					InZBuffer[CurrentPointZBufferIndex] = CurrentPoint.z;
					InImage.set(X, Y, PixelColor);
				}
			}
		}
	}
};

