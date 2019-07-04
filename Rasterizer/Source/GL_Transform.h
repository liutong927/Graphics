#pragma once
#include "..\Utils\geometry.h"

static int Depth = 255;

class Transform
{
public:

	// 4*1 matrix to vec3f for point
	static Vec3f Matrix2Vec(Matrix InMatrix)
	{
		return Vec3f(InMatrix[0][0] / InMatrix[3][0], InMatrix[1][0] / InMatrix[3][0], InMatrix[2][0] / InMatrix[3][0]);
	}

	// 4*1 matrix to vec3f for vector
	static Vec3f Matrix2VecForV(Matrix InMatrix)
	{
		return Vec3f(InMatrix[0][0], InMatrix[1][0], InMatrix[2][0]);
	}

	// vec3f to 4*1 matrix.
	static Matrix Vec2Matrix(Vec3f InVec, float InFill = 1.f)
	{
		Matrix Result(4, 1);
		Result[0][0] = InVec.x;
		Result[1][0] = InVec.y;
		Result[2][0] = InVec.z;
		Result[3][0] = InFill;

		return Result;
	}

	static Matrix Vec2Matrix13(Vec3f InVec)
	{
		Matrix Result(1, 3);
		Result[0][0] = InVec.x;
		Result[0][1] = InVec.y;
		Result[0][2] = InVec.z;

		return Result;
	}

	// Vec3f convert to translation matrix
	static Matrix Translation(Vec3f InVec)
	{
		Matrix Translation(Matrix::Identity(4));
		Translation[0][3] = InVec.x;
		Translation[1][3] = InVec.y;
		Translation[2][3] = InVec.z;

		return Translation;
	}

	// Vec3f convert to scale matrix
	static Matrix Scale(Vec3f InVec)
	{
		Matrix Scale(Matrix::Identity(4));
		Scale[0][0] = InVec.x;
		Scale[1][1] = InVec.y;
		Scale[2][2] = InVec.z;

		return Scale;
	}

	// Zoom by apply same scaling.
	static Matrix Zoom(float InFactor)
	{
		Vec3f ZoomVec(InFactor, InFactor, InFactor);
		return Scale(ZoomVec);
	}

	// Vec3f convert to rotation matrix
	// rotate along x-axis.
	static Matrix RotationX(float InCosAngle, float InSinAngle)
	{
		Matrix Rotation(Matrix::Identity(4));

		Rotation[1][1] = Rotation[2][2] = InCosAngle;
		Rotation[1][2] = -InSinAngle;
		Rotation[2][1] = InSinAngle;

		return Rotation;
	}

	// rotate along y-axis.
	static Matrix RotationY(float InCosAngle, float InSinAngle)
	{
		Matrix Rotation(Matrix::Identity(4));

		Rotation[0][0] = Rotation[2][2] = InCosAngle;
		Rotation[0][2] = -InSinAngle;
		Rotation[2][0] = InSinAngle;

		return Rotation;
	}

	// rotate along z-axis.
	static Matrix RotationZ(float InCosAngle, float InSinAngle)
	{
		Matrix Rotation(Matrix::Identity(4));

		Rotation[0][0] = Rotation[1][1] = InCosAngle;
		Rotation[0][1] = -InSinAngle;
		Rotation[1][0] = InSinAngle;

		return Rotation;
	}

	// viewport matrix.
	// vertex -> [-1,1]
	// after apply viewport matrix,
	// [-1,1] is mapping to [X, X+Width] [Y, Y+Height] [0, Depth]
	static Matrix Viewport(int X, int Y, int Width, int Height)
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

	// camera is at InEye, point to InCenter, InUp is camera direction which will be vertical frame
	// how to compute this matrix?
	// firstly we fix the z-axis which is camera to origin direction.
	// then we need to get x-axis and y-axis, but how?
	// we know upper vector in world, then we can compute right (x) axis of camera space by cross product
	// of upper and camera vector.
	// lastly we compute upper vector(y-axis) of camera space by another cross product of x and z.
	static Matrix LookAt(Vec3f InEye, Vec3f InCenter, Vec3f InUp)
	{
		Matrix Result(Matrix::Identity(4));
		// compute transformed frame x,y,z axis.
		Vec3f z = (InEye - InCenter).normalize();
		Vec3f x = cross(InUp, z).normalize();
		Vec3f y = cross(z, x).normalize();

		Matrix CameraFrame = Matrix::Identity(4);
		Matrix Translation = Matrix::Identity(4);

		// lookup matrix is camera frame multiply a translation(world to camera translation)
		for (int i = 0; i < 3; i++)
		{
			CameraFrame[0][i] = x.raw[i];
			CameraFrame[1][i] = y.raw[i];
			CameraFrame[2][i] = z.raw[i];
			Translation[i][3] = -InCenter.raw[i];
		}

		Result = CameraFrame * Translation;
		return Result;
	}

	// construct projection matrix
	static Matrix Projection(float InCoffient)
	{
		Matrix Result(Matrix::Identity(4));
		Result[3][2] = InCoffient;
		return Result;
	}
};