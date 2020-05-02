#include "stdafx.h"
#include "Vector3D.h"


Vector3D::Vector3D(const Vector3D &vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
}

//addition

Vector3D Vector3D ::operator+(const Vector3D &vec)
{
	return Vector3D(x + vec.x, y + vec.y, z + vec.z);
}

Vector3D &Vector3D ::operator+=(const Vector3D &vec)
{
	x += vec.x;
	y += vec.y;
	z += vec.z;
	return *this;
}
//substraction//
Vector3D Vector3D ::operator-(const Vector3D &vec)
{
	return Vector3D(x - vec.x, y - vec.y, z - vec.z);
}

Vector3D &Vector3D::operator-=(const Vector3D &vec)
{
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	return *this;
}

//scalar multiplication

Vector3D Vector3D ::operator*(f value)
{
	return Vector3D(x*value, y*value, z*value);
}



Vector3D &Vector3D::operator*=(f value)
{
	x *= value;
	y *= value;
	z *= value;
	return *this;
}

//scalar division
Vector3D Vector3D ::operator/(f value)
{
	assert(value != 0);
	return Vector3D(x / value, y / value, z / value);
}

Vector3D &Vector3D ::operator/=(f value)
{
	assert(value != 0);
	x /= value;
	y /= value;
	z /= value;
	return *this;
}


Vector3D &Vector3D::operator=(const Vector3D &vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

//Dot product
f Vector3D::dot_product(const Vector3D &vec)
{
	return x * vec.x + vec.y*y + vec.z*z;
}

//cross product
Vector3D Vector3D::cross_product(const Vector3D &vec)
{
	f ni = y * vec.z - z * vec.y;
	f nj = z * vec.x - x * vec.z;
	f nk = x * vec.y - y * vec.x;
	return Vector3D(ni, nj, nk);
}

f Vector3D::magnitude()
{
	return sqrt(square());
}

f Vector3D::square()
{
	return x * x + y * y + z * z;
}

Vector3D Vector3D::normalization()
{
	if (magnitude() != 0)
	{
		*this /= magnitude();
	}
	return *this;
}

f Vector3D::distance(const Vector3D &vec)
{
	Vector3D dist = *this - vec;
	return dist.magnitude();
}

f Vector3D::show_X()
{
	return x;
}

f Vector3D::show_Y()
{
	return y;
}

f Vector3D::show_Z()
{
	return z;
}

