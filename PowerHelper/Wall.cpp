#include "stdafx.h"
#include "Wall.h"
#include <math.h>


Wall::Wall()
{
}


Wall::~Wall()
{
}

Wall::Wall(double ax, double ay, double bx, double by, bool backwall , bool rail )
{
	

	pointA = Vector3D(ax, ay, 0);
	pointB = Vector3D(bx, by, 0);
	normal = Vector3D(ay - by, bx - ax);
	normal.normalization();
	vectorAB = pointB - pointA;
	fullVector = vectorAB;
	vectorAB.normalization();
	aabb = CRectangle();
	aabb.left = min(ax, bx);
	aabb.right = max(ax, bx);
	aabb.top = min(ay, by);
	aabb.bottom = max(ay, by);
	isBackWall = backwall;
	isRail = rail;
}
