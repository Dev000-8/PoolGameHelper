#include "stdafx.h"
#include "BallWallPair.h"


BallWallPair::BallWallPair()
{
	
	wall = NULL;
	ball = NULL;
}



BallWallPair::~BallWallPair()
{
}

void BallWallPair::copy(BallWallPair pair)
{
	ball = pair.ball;
	wall = pair.wall;
	t = pair.t;
}

bool  BallWallPair::willCollideIn(double dt)
{
	double cdlen = 0;
	Vector3D centerToTail;
	double adotb = 0;
	double div = 0;
	double xpos = 0;
	double  ypos = 0;
	Vector3D negVelocity;
	Vector3D rayStartPoint;
	Vector3D rayPointToCircle;
	double qa = 0;
	double k = 0;
	double qc = 0;
	double qd = 0;
	double param = 0;
	Vector3D normal  = wall->normal;
	Vector3D normScaledRadius  = wall->normal;
	normScaledRadius = normScaledRadius * (-ball->radius);
	Vector3D A  = ball->position + (normScaledRadius);
	Vector3D B  = A + (ball->velocity);
	Vector3D AB  = B - (A);
	Vector3D C  = wall->pointA;
	Vector3D D  = wall->pointB;
	Vector3D AC = C - (A);
	Vector3D CD = D - (C);
	double test = normal.dot_product(AB);
	t = normal.dot_product(AC) / test;
	if (test == 0)
	{
		t = 0;
	}
	if (t <= dt)
	{
		AB = AB * (t);
		AB = AB + (A);
		cdlen = CD.square();
		centerToTail = AB  - (C);
		adotb = centerToTail.dot_product(CD);
		div = adotb / cdlen;
		div = div < 0 ? (0) : div > 1 ? (1) : (div);
		if (div > 0 && div < 1 && t >= 0)
		{
			return true;
		}
		xpos = C.x + CD.x * div;
		ypos = C.y + CD.y * div;
		negVelocity = ball->velocity;
		negVelocity = negVelocity * (-1);
		rayStartPoint = Vector3D(xpos,ypos);
		rayPointToCircle = ball->position - (rayStartPoint);
		qa = negVelocity.dot_product(negVelocity);
		k = rayPointToCircle.dot_product(negVelocity);
		qc = rayPointToCircle.dot_product(rayPointToCircle) - ball->radiusSquared;
		qd = k * k - qa * qc;
		if (qd < 0)
		{
			return false;
		}
		param = (k - sqrt(qd)) / qa;
		if (param <= 1)
		{
			negVelocity = negVelocity * (param);
			rayStartPoint = rayStartPoint + (negVelocity);
			A = rayStartPoint;
			AC = C - (A);
			t = normal.dot_product(AC) / test;
			if (test == 0)
			{
			t = 0;
			}
			if (t <= dt && t >= 0)
			{
			return true;
			}
		}
	}
	return false;
}

void BallWallPair::resolve()
{
	Vector3D dir;
	double diff = 0;
	Vector3D n = wall->normal;
	Vector3D ballToWall = ball->position - wall->pointA;
	Vector3D wallVector = wall->fullVector;
	double ballToWallProj = wall->fullVector.dot_product(ballToWall) / wallVector.square();
	if (ballToWallProj > 1.0)
	{
		dir = ball->position - (wall->pointB);
		dir.normalization();
		n = dir;
	}
	else if (ballToWallProj < 0.0)
	{
		dir = ball->position - (wall->pointA);
		dir.normalization();
		n = dir;
	}
	else
	{
		dir = n;
	}
	Vector3D r = n;
	r = r * (ball->radius);
	double ima = ball->invMass;
	double e = 0.7;
	Vector3D Vab = ball->velocity;
	Vector3D pointVel = ball->angularVelocity.cross_product(r);
	Vector3D normal = n;
	normal = normal * (pointVel.dot_product(normal));
	Vector3D tangentDirection = pointVel - (normal);
	tangentDirection.normalization();
	Vector3D frictionDirection = tangentDirection;
	frictionDirection = frictionDirection * (-1);
	double tangentSpeed = tangentDirection.dot_product(pointVel);
	ima = ima * n.dot_product(n);
	double j = -(1 + e) * Vab.dot_product(n) / ima;
	dir = dir * (j * ima);
	ball->velocity = ball->velocity + (dir);
	frictionDirection = frictionDirection * (j * ima * 0.16);
	ball->velocity = ball->velocity + (frictionDirection);
	ball->velocity.z = 0;
	if (ball->bodyState == Body::STATE_ROLLING)
	{
		ball->angularVelocity.x = 0;
		ball->angularVelocity.y = 0;
		ball->angularVelocity.z = ball->angularVelocity.z * 0.8;
	}
	else
	{
		ball->angularVelocity = ball->angularVelocity * (0.8);
	}

	ball->bodyState = Body::STATE_SLIDING;

}