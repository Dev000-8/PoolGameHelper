#include "stdafx.h"
#include "Matrix3D2.h"


Matrix3D2::Matrix3D2()
{
	m00 = 0.0;
	m01 = 0.0;
	m02 = 0.0;

	m10 = 0.0;
	m11 = 0.0;
	m12 = 0.0;

	m20 = 0.0;
	m21 = 0.0;
	m22 = 0.0;

}

Matrix3D2::Matrix3D2(double m00, double m01, double m02,
	double m10, double m11, double m12,
	double m20, double m21, double m22)
{
	m00 = m00;
	m01 = m01;
	m02 = m02;

	m10 = m10;
	m11 = m11;
	m12 = m12;

	m20 = m20;
	m21 = m21;
	m22 = m22;

}

void Matrix3D2::setIdentity()
{
	m00 = 1.0;
	m01 = 0.0;
	m02 = 0.0;

	m10 = 0.0;
	m11 = 1.0;
	m12 = 0.0;

	m20 = 0.0;
	m21 = 0.0;
	m22 = 1.0;
}
#include <math.h>
void Matrix3D2::rotX(double angle)
{
	double sinAngle, cosAngle;

	sinAngle = sin(angle);
	cosAngle = cos(angle);

	m00 = 1.0;
	m01 = 0.0;
	m02 = 0.0;

	m10 = 0.0;
	m11 = cosAngle;
	m12 = -sinAngle;

	m20 = 0.0;
	m21 = sinAngle;
	m22 = cosAngle;
}

void Matrix3D2::rotY(double angle)
{
	double sinAngle, cosAngle;

	sinAngle = sin(angle);
	cosAngle = cos(angle);

	m00 = cosAngle;
	m01 = 0.0;
	m02 = sinAngle;

	m10 = 0.0;
	m11 = 1.0;
	m12 = 0.0;

	m20 = -sinAngle;
	m21 = 0.0;
	m22 = cosAngle;
}

void Matrix3D2::rotZ(double angle)
{
	double sinAngle, cosAngle;

	sinAngle = sin(angle);
	cosAngle = cos(angle);

	m00 = cosAngle;
	m01 = -sinAngle;
	m02 = 0.0;

	m10 = sinAngle;
	m11 = cosAngle;
	m12 = 0.0;

	m20 = 0.0;
	m21 = 0.0;
	m22 = 1.0;
}


Vector3D Matrix3D2::transform(Vector3D t) 
{
	double x, y, z;
	x = m00 * t.x + m01 * t.y + m02 * t.z;
	y = m10 * t.x + m11 * t.y + m12 * t.z;
	z = m20 * t.x + m21 * t.y + m22 * t.z;
	
	return Vector3D(x, y, z);
}

Matrix3D2::~Matrix3D2()
{
}
