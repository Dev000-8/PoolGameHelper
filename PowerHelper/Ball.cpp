#include "stdafx.h"
#include "Ball.h"


Ball::Ball()
{
	radius = 0.05715f * 0.5f;
	radiusSquared = radius * radius;
	setMass(0.16f);
	setInertia(2.0f / 5.0f * mass * radius * radius);
	aabbColision = false;
	contactPoint = Vector3D(0, 0, radius);
	ballNumber = 0;
	hitBackWall = false;
	ballType = BALL_CUE;
	railsHit = 0;
	railsHitBeforeFirstHit = 0;
	ballReturnIndex = -1;
	ballState = BALL_STATE_ACTIVE;
}


bool Ball::update(double et)
{

   double mu  = 0;
   Vector3D contactVec;
   Vector3D vp;
   Vector3D zDrag;
   double distanceCheck = 0;
   double wi = 0;
   double hi = 0;
   double  gravity = 9.8f;
   if (ballState == BALL_STATE_ACTIVE)
   {
	  if (bodyState == STATE_SLIDING)
	  {
		 contactVec = Vector3D(0,0,radius);
		 vp = angularVelocity.cross_product(contactVec);
		 vp = vp + velocity;
		 vp.normalization();
		 vp = vp * (-MU_SLIDE * gravity * mass);
		 addForce(vp,contactVec);
	  }
	  else if (bodyState == STATE_ROLLING)
	  {
		 contactVec = Vector3D(0,0,radius);
		 vp = angularVelocity.cross_product(contactVec);
		 vp.normalization();
		 vp = vp * (-MU_ROLL * gravity * mass);
		 addTorque(contactVec.cross_product(vp));
	  }

	  zDrag = Vector3D(0,0,angularVelocity.z);
	  zDrag.normalization();
	  if (angularVelocity.z > -0.05 && angularVelocity.z < 0.05)
	  {
		 zDrag.z = angularVelocity.z;
	  }
	  zDrag = zDrag * (-MU_ROLL * mass * 0.6);
	  addTorque(zDrag);
   }
   else {

   }
   
   return active;
}


void Ball::shoot(Vector3D cueToBallWorldSpace, Vector3D r)
{
	double springConstant = -50.0f;
	double deltaTime  = 0.005f;
	double combinedValue  = springConstant * deltaTime;
	cueToBallWorldSpace = cueToBallWorldSpace * (combinedValue);
	addImpulse(cueToBallWorldSpace, r);
}

void Ball::initPocketBall(int index)
{
	reset();
	ballReturnIndex = index;
	angularVelocity.x = 0;
	angularVelocity.y = 15;
	angularVelocity.z = 0;
	double startPositionX  = 1.2f;
	position.x = startPositionX;
	position.y = 0;
	double distanceToTravel  = startPositionX - index * 2 * radius;
	/*pocketTimer = distanceToTravel / (angularVelocity.y * radius);
	ballState = BALL_STATE_ROLLINGONRAIL;*/
	bodyState = STATE_ROLLING;
	/*Globals.view.table.resetBall(this);*/
}

Ball::~Ball()
{
}
