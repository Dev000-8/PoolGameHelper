#include "stdafx.h"
#include "BallBallPair.h"


BallBallPair::BallBallPair()
{
	goa = NULL;
	gob = NULL;
}

void BallBallPair::copy(BallBallPair pair)
{
	goa = pair.goa;
	gob = pair.gob;
	t = pair.t;
}

double BallBallPair::toc()
{
	return t;
}

bool BallBallPair::willCollideIn(double dt)
{
   vDiff.x = goa->velocity.x - gob->velocity.x;
   vDiff.y = goa->velocity.y - gob->velocity.y;
   double a = vDiff.dot_product(vDiff);
   if (a < 0)
   {
	  return false;
   }
   pDiff.x = goa->position.x - gob->position.x;
   pDiff.y = goa->position.y - gob->position.y;
   double b  = pDiff.dot_product(vDiff);
   if (b >= 0)
   {
	  return false;
   }
   double c  = pDiff.dot_product(pDiff) - 4 * goa->radiusSquared;
   double d  = b * b - a * c;
   if (d < 0)
   {
	  return false;
   }
   t = (-b - sqrt(d)) / a;
   return t <= dt;
}

void  BallBallPair::resolve()
{
	Vector3D n;
	double ima = 0;
	double imb  = 0;
	double e  = 0;
	Vector3D Vab ;
	double ims ;
	GameObject *a = goa;
	GameObject *b = gob;
	Vector3D vectorBetweenAB = a->position - (b->position);
	ima = a->invMass;
	imb = b->invMass;
	ims = ima + imb;
	e = 0.93;
	Vab = a->velocity - (b->velocity);
	n = vectorBetweenAB;
	ims = ims * n.dot_product(n);
	double value = Vab.dot_product(n);
	if (value >= 0)
	{
		return;
	}
	double jj = -(1 + e) * value / ims;
	n = n * (jj * ima);
	a->velocity = a->velocity + (n);
	n = n * (-1);
	b->velocity = b->velocity + (n);
	a->bodyState = Body::STATE_SLIDING;
	b->bodyState = Body::STATE_SLIDING;
	
}

BallBallPair::~BallBallPair()
{
}
