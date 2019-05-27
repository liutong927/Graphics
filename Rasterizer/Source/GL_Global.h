#pragma once
#include "..\Utils\model.h"
#include "..\Utils\geometry.h"

Model* ModelData;
Matrix VPMatrix;
Matrix Projection;
Matrix ModelView;
//Vec3f LightDir = Vec3f(0, 0, -1);
Vec3f LightDir = Vec3f(1, -1, 1);
