#ifndef _MATRIX3D_
#define _MATRIX3D_
#include "Vector3D.h"

# define M_PI           3.14159265358979323846

class Matrix3D
{
public:
	double *_matrix;
	Matrix3D();
	~Matrix3D();
	void identity();
	void appendRotation(double degrees, Vector3D axis, Vector3D pivotPoint);
	void append(Matrix3D lhs);
	Matrix3D getRotationMatrix(double theta, double u, double v, double w, double a, double b, double c);
	Vector3D transformVector(Vector3D v);
};
#endif

