#ifndef _BALL_
#define _BALL_
#include "GameObject.h"
class Ball : public GameObject
{
public:
	int ballNumber;
	int ballType;
	int ballState;
	bool hitBackWall;
	int railsHit;
	int railsHitBeforeFirstHit;
	int ballReturnIndex;
	double MU_SLIDE = 0.2f;
	double MU_ROLL = 0.0056f;
	Vector3D contactPoint;

	static const int BALL_NONE = -1;
	static const int BALL_CUE = 0;
	static const int BALL_OTHER = 1;

	static const int BALL_STATE_ACTIVE = 0;
	static const int BALL_STATE_INACTIVE = 1;

	bool update(double et);
	void shoot(Vector3D cueToBallWorldSpace, Vector3D r);
	void initPocketBall(int index);

	Ball();
	~Ball();
};

#endif

