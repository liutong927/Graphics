#include "geometry.h"
#include <assert.h>
#include <iostream>

//**********************************************************************
//                         Matrix
//**********************************************************************

Matrix::Matrix(int InRow /*= 4*/, int InCol /*= 4*/): 
	Elements(std::vector<std::vector<float>>(InRow, std::vector<float>(InCol, 0.f))), 
	Rows(InRow), 
	Cols(InCol)
{
}

void Matrix::SetRow(size_t InRowIdx, Vec3f InVec)
{
	assert(InRowIdx < Rows);
	for (size_t i = Cols; i--; Elements[InRowIdx][i] = InVec.raw[i]);
}

void Matrix::SetCol(size_t InColIdx, Vec3f InVec)
{
	assert(InColIdx < Cols);
	for (size_t i = Rows; i--; Elements[i][InColIdx] = InVec.raw[i]);
}

Matrix Matrix::Identity(int InDimensions)
{
	Matrix IMatrix(InDimensions, InDimensions);
	for (int RowIndex = 0; RowIndex < InDimensions; RowIndex++)
	{
		for (int ColIndex = 0; ColIndex < InDimensions; ColIndex++)
		{
			if (RowIndex == ColIndex)
			{
				IMatrix[RowIndex][ColIndex] = 1.f;
			}
			else
			{
				IMatrix[RowIndex][ColIndex] = 0.f;
			}
		}
	}
	return IMatrix;
}

std::vector<float>& Matrix::operator[](const int i)
{
	assert(i >= 0 && i < Rows);
	return Elements[i];
}

Matrix Matrix::operator*(const Matrix& InM)
{
	assert(Cols == InM.Rows);
	Matrix Result(Rows, InM.Cols);
	for (int RowIndex = 0; RowIndex < Result.Rows; RowIndex++)
	{
		for (int ColIndex = 0; ColIndex < Result.Cols; ColIndex++)
		{
			Result[RowIndex][ColIndex] = .0f;
			for (int k = 0; k < Cols; k++)
			{
				// fix row of this matrix multiply col of that matrix
				Result[RowIndex][ColIndex] += Elements[RowIndex][k] * InM.Elements[k][ColIndex];
			}
		}
	}
	return Result;
}

Matrix Matrix::Transpose()
{
	Matrix TransposeMatix(Cols, Rows);
	for (int RowIndex = 0; RowIndex < TransposeMatix.Rows; RowIndex++)
	{
		for (int ColIndex = 0; ColIndex < TransposeMatix.Cols; ColIndex++)
		{
			TransposeMatix[RowIndex][ColIndex] = Elements[ColIndex][RowIndex];
		}
	}
	return TransposeMatix;
}

Matrix Matrix::Inverse()
{
	assert(Rows == Cols);
	// it's a bit complicated, just copy now.
	Matrix Result(Rows, Cols*2);

	for (int RowIndex = 0; RowIndex < Rows; RowIndex++)
	{
		for (int ColIndex = 0; ColIndex < Cols; ColIndex++)
		{
			Result[RowIndex][ColIndex] = Elements[RowIndex][ColIndex];
		}
	}

	for (int RowIndex = 0; RowIndex < Rows; RowIndex++)
	{
		Result[RowIndex][RowIndex + Cols] = 1;
	}

	// first pass
	for (int i = 0; i < Rows - 1; i++) 
	{
		// normalize the first row
		for (int j = Result.Cols - 1; j >= 0; j--)
		{
			Result[i][j] /= Result[i][i];
		}

		for (int k = i + 1; k < Rows; k++) 
		{
			float coeff = Result[k][i];
			for (int j = 0; j < Result.Cols; j++) 
			{
				Result[k][j] -= Result[i][j] * coeff;
			}
		}
	}

	// normalize the last row
	for (int j = Result.Cols - 1; j >= Rows - 1; j--)
	{
		Result[Rows - 1][j] /= Result[Rows - 1][Rows - 1];
	}

	// second pass
	for (int i = Rows - 1; i > 0; i--)
	{
		for (int k = i - 1; k >= 0; k--) 
		{
			float coeff = Result[k][i];
			for (int j = 0; j < Result.Cols; j++)
			{
				Result[k][j] -= Result[i][j] * coeff;
			}
		}
	}
	// cut the identity matrix back
	Matrix truncate(Rows, Cols);
	for (int i = 0; i < Rows; i++)
	{
		for (int j = 0; j < Cols; j++)
		{
			truncate[i][j] = Result[i][j + Cols];
		}
	}

	return truncate;
}

std::ostream& operator<<(std::ostream& s, Matrix& m)
{
	for (int RowIndex = 0; RowIndex < m.Rows; RowIndex++)
	{
		for (int ColIndex = 0; ColIndex < m.Cols; ColIndex++)
		{
			s << m[RowIndex][ColIndex];
			if (ColIndex < m.Cols - 1)
			{
				s << "\t";
			}
		}
		s << "\n";
	}
	return s;
}
