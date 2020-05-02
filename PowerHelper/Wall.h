#ifndef _WALL_
#define _WALL_
#include "Vector3D.h"
#include "Rectangle.h"


class Wall
{
public:

	Vector3D pointA;

	Vector3D pointB;

	Vector3D normal;

	Vector3D vectorAB;

	Vector3D fullVector;

	CRectangle aabb;

	bool isBackWall;

	bool isRail;

	Wall(double ax, double ay, double bx, double by, bool backwall = false, bool rail = false);

	Wall();
	~Wall();
};

#endif


