#ifndef _RULESET_
#define _RULESET_
#include<iostream>
#include <list>
#include "Wall.h"
using namespace std;
class Ruleset
{
public:
	double tableWidth;
	double tableHeight;
	double railSize;
	double pocketRadius;
	double pocketRadiusSquared;

	list<Wall> rails;
	list<Wall> walls;
	list<Vector3D> pocketPositions;
	list<Wall> quickWallCheck;

	const int POCKET_SIDE  = 0;

	const int POCKET_CORNER  = 1;

	Ruleset();
	~Ruleset();
	void initTable();
};

#endif


