#pragma once
#include "ICollidablePair.h"
#include "GameObject.h"
class BallBallPair 
{
public:
	GameObject *goa;
	GameObject *gob;
	double t;
	Vector3D vDiff;
	Vector3D pDiff;

	void copy(BallBallPair pair);
	double toc();
	bool willCollideIn(double dt);
	void  resolve();
	BallBallPair();
	~BallBallPair();
};

