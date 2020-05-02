#pragma once
#include "ICollidablePair.h"
#include "GameObject.h"
#include "Wall.h"
class BallWallPair 
{
public:
	GameObject *ball;
	Wall *wall;
	double t;
	BallWallPair();

	double toc()
	{
		return t;
	}


	bool  willCollideIn(double dt);
	void resolve();
	void copy(BallWallPair pair);
	~BallWallPair();
};

