#include "stdafx.h"
#include "Matrix3D.h"
#include<iostream>

Matrix3D::Matrix3D()
{
	_matrix = new double[16];
	identity();
}


Matrix3D::~Matrix3D()
{
}


void Matrix3D::identity()
{
	double *m = _matrix;
	m[0] = m[5] = m[10] = m[15] = 1;
	m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0;

}


void Matrix3D::append(Matrix3D lhs)
{
	double  *ma = lhs._matrix, *mb = _matrix, *m = _matrix;
	double ma11 = ma[0], ma12 = ma[4], ma13 = ma[8], ma14 = ma[12],
		ma21 = ma[1], ma22 = ma[5], ma23 = ma[9], ma24 = ma[13],
		ma31 = ma[2], ma32 = ma[6], ma33 = ma[10], ma34 = ma[14],
		ma41 = ma[3], ma42 = ma[7], ma43 = ma[11], ma44 = ma[15];

	double mb11 = mb[0], mb12 = mb[4], mb13 = mb[8], mb14 = mb[12],
		mb21 = mb[1], mb22 = mb[5], mb23 = mb[9], mb24 = mb[13],
		mb31 = mb[2], mb32 = mb[6], mb33 = mb[10], mb34 = mb[14],
		mb41 = mb[3], mb42 = mb[7], mb43 = mb[11], mb44 = mb[15];

	m[0] = ma11 * mb11 + ma12 * mb21 + ma13 * mb31 + ma14 * mb41;
	m[1] = ma21 * mb11 + ma22 * mb21 + ma23 * mb31 + ma24 * mb41;
	m[2] = ma31 * mb11 + ma32 * mb21 + ma33 * mb31 + ma34 * mb41;
	m[3] = ma41 * mb11 + ma42 * mb21 + ma43 * mb31 + ma44 * mb41;

	m[4] = ma11 * mb12 + ma12 * mb22 + ma13 * mb32 + ma14 * mb42;
	m[5] = ma21 * mb12 + ma22 * mb22 + ma23 * mb32 + ma24 * mb42;
	m[6] = ma31 * mb12 + ma32 * mb22 + ma33 * mb32 + ma34 * mb42;
	m[7] = ma41 * mb12 + ma42 * mb22 + ma43 * mb32 + ma44 * mb42;

	m[8] = ma11 * mb13 + ma12 * mb23 + ma13 * mb33 + ma14 * mb43;
	m[9] = ma21 * mb13 + ma22 * mb23 + ma23 * mb33 + ma24 * mb43;
	m[10] = ma31 * mb13 + ma32 * mb23 + ma33 * mb33 + ma34 * mb43;
	m[11] = ma41 * mb13 + ma42 * mb23 + ma43 * mb33 + ma44 * mb43;

	m[12] = ma11 * mb14 + ma12 * mb24 + ma13 * mb34 + ma14 * mb44;
	m[13] = ma21 * mb14 + ma22 * mb24 + ma23 * mb34 + ma24 * mb44;
	m[14] = ma31 * mb14 + ma32 * mb24 + ma33 * mb34 + ma34 * mb44;
	m[15] = ma41 * mb14 + ma42 * mb24 + ma43 * mb34 + ma44 * mb44;

}
void Matrix3D::appendRotation(double degrees , Vector3D axis , Vector3D pivotPoint )
{
	degrees = +degrees;
	append(getRotationMatrix(degrees , axis.x, axis.y, axis.z,
		pivotPoint.x , pivotPoint.y ,pivotPoint.z ));

}

Matrix3D Matrix3D::getRotationMatrix(double theta, double u, double v, double w, double a, double b, double c)
{
	double u2 = u * u, v2 = v * v, w2 = w * w;
	double L2 = u2 + v2 + w2, L = sqrt(L2);
	u /= L; v /= L; w /= L;
	u2 /= L2; v2 /= L2; w2 /= L2;
	double cos_ = cos(theta), sin_ = sin(theta);

	Matrix3D m;
	m._matrix[0] =  u2 + (v2 + w2) * cos_;
	m._matrix[1] = u * v * (1 - cos_) + w * sin_;
	m._matrix[2] = u * w * (1 - cos_) - v * sin_;
	m._matrix[3] = 0;
	m._matrix[4] = u * v * (1 - cos_) - w * sin_;
	m._matrix[5] = v2 + (u2 + w2) * cos_;
	m._matrix[6] = v * w * (1 - cos_) + u * sin_;
	m._matrix[7] = 0;
	m._matrix[8] = u * w * (1 - cos_) + v * sin_;
	m._matrix[9] = v * w * (1 - cos_) - u * sin_;
	m._matrix[10] = w2 + (u2 + v2) * cos_;
	m._matrix[11] = 0;
	m._matrix[12] = (a * (v2 + w2) - u * (b * v + c * w)) * (1 - cos_) + (b * w - c * v) * sin_;
	m._matrix[13] = (b * (u2 + w2) - v * (a * u + c * w)) * (1 - cos_) + (c * u - a * w) * sin_;
	m._matrix[14] = (c * (u2 + v2) - w * (a * u + b * v)) * (1 - cos_) + (a * v - b * u) * sin_;
	m._matrix[15] = 1 ;

	return m;
}

Vector3D Matrix3D::transformVector(Vector3D v)
{
	double *m = _matrix;
	double x = v.x, y = v.y, z = v.z;
	Vector3D vNew(m[0] * x + m[4] * y + m[8] * z + m[12],
		m[1] * x + m[5] * y + m[9] * z + m[13],
		m[2] * x + m[6] * y + m[10] * z + m[14]);

	return vNew;
}
