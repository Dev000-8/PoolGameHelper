#pragma once

#include "Vector3D.h"
class Matrix3D2
{
public:
	/**
	 * The first matrix element in the first row.
	 */
	 double m00;

	/**
	 * The second matrix element in the first row.
	 */
	 double m01;

	/**
	 * The third matrix element in the first row.
	 */
	 double m02;

	/**
	 * The first matrix element in the second row.
	 */
	 double m10;

	/**
	 * The second matrix element in the second row.
	 */
	 double m11;

	/**
	 * The third matrix element in the second row.
	 */
	 double m12;

	/**
	 * The first matrix element in the third row.
	 */
	 double m20;

	/**
	 * The second matrix element in the third row.
	 */
	 double m21;

	/**
	 * The third matrix element in the third row.
	 */
	 double m22;

	 double EPS = 1.110223024E-16;
	 double ERR_EPS = 1.0E-8;
	 double xin, yin, zin, xout, yout, zout;

	 Matrix3D2(double m00, double m01, double m02,
		 double m10, double m11, double m12,
		 double m20, double m21, double m22);
	 void setIdentity();
	Matrix3D2();
	void rotX(double angle);
	void rotY(double angle);
	void rotZ(double angle);

	Vector3D transform(Vector3D t);

	~Matrix3D2();
};

