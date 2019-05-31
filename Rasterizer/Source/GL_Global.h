#pragma once
#include "..\Utils\model.h"
#include "..\Utils\geometry.h"

Model* ModelData;
Matrix VPMatrix;
Matrix Projection;
Matrix ModelView;
Matrix Uniform_M; //Projection*ModelView
Matrix Uniform_MIT; // inverse transposed Uniform_M

//Vec3f LightDir = Vec3f(0, 0, -1);
Vec3f LightDir = Vec3f(1, 0, 0);

Vec3f Camera(0, 0, 3);
Vec3f Eye(1, 1, 4);
Vec3f Center(0, 0, 0);

int Width = 800;
int Height = 800;
