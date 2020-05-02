#include "stdafx.h"
#include "Body.h"


Body::Body()
{
	bodyState = STATE_REST;
	mass = 1;
	inertia = 1;
	invMass = 0;
	invInertia = 0;
	upVector = Vector3D(0, -1, 0, 0);
	rightVector =  Vector3D(1, 0, 0, 0);
	rotMatrix =  Matrix3D();
}


Body::~Body()
{
}


void Body::reset()
{
	velocity.x = velocity.y = velocity.z = 0;
	angularVelocity.x = angularVelocity.y = angularVelocity.z = 0;
	force.x = force.y = force.z = 0;
	torque.x = torque.y = torque.z = 0;
	bodyState = STATE_REST;
}


void Body::setMass(double m)
{
	mass = m;
	if (mass > 0)
	{
		invMass = 1.0f / mass;
	}
	else
	{
		invMass = 0;
	}
}



void Body::setInertia(double i)
{
	inertia = i;
	if (inertia > 0)
	{
		invInertia = 1.0f / inertia;
	}
	else
	{
		inertia = 0;
	}
}


bool Body::update(double et)
{
	return true;
}


void Body::addForceComponents(double x , double y , double z)
{
	force.x = force.x + x;
	force.x = force.x + x;
	force.y = force.y + y;
	force.z = force.z + z;
}


void Body::addForce(Vector3D af, Vector3D point)
{
	force = force + af;
	torque = torque + point.cross_product(af);
}


void Body::addTorque(Vector3D t)
{
	
	torque = torque + t;
}


void Body::addImpulse(Vector3D i, Vector3D point)
{
	velocity.x = velocity.x + i.x * invMass;
	velocity.y = velocity.y + i.y * invMass;
	velocity.z = velocity.z + i.z * invMass;
	Vector3D at = point.cross_product(i);
	at = at * invInertia;
	angularVelocity = angularVelocity + at;
	bodyState = STATE_SLIDING;
}


void Body::integrateForces2(double et)
{
	force = force * (invMass * et);
	velocity = velocity + force;
	torque = torque * (invInertia * et);
	angularVelocity = angularVelocity + (torque);
	force.x = 0;
	force.y = 0;
	force.z = 0;
	torque.x = 0;
	torque.y = 0;
	torque.z = 0;
}

void  Body::integrateForces(double et)
{
	Vector3D cp;
	Vector3D av;
	Vector3D contact;
	Vector3D rw;
	double dotp;
	double diff;

	torque = torque * (invInertia * et);
	angularVelocity = angularVelocity + (torque);
	if (bodyState == STATE_ROLLING)
	{
		cp = Vector3D(0, 0, radius);
		av = cp.cross_product(angularVelocity);
		velocity.x = av.x;
		velocity.y = av.y;
		velocity.z = av.z;
	}
	else if (bodyState == STATE_SLIDING)
	{
		force = force * (invMass * et);
		velocity = velocity + (force);
		contact = Vector3D(0, 0, radius);
		rw = contact.cross_product(angularVelocity);
		dotp = rw.dot_product(velocity);
		diff = (rw - (velocity)).magnitude();
		if (dotp && diff < 0.05)
		{
			bodyState = STATE_ROLLING;
			velocity.x = rw.x;
			velocity.y = rw.y;
			velocity.z = rw.z;
		}
	}
	double velSquared = velocity.square();
	double angSquared = angularVelocity.square();
	angSquared = angSquared * angSquared;
	if (velSquared < threshold && angSquared < threshold)
	{
		bodyState = STATE_REST;
		velocity.x = 0;
		velocity.y = 0;
		velocity.z = 0;
		angularVelocity.x = 0;
		angularVelocity.y = 0;
		angularVelocity.z = 0;
	}
	force.x = 0;
	force.y = 0;
	force.z = 0;
	torque.x = 0;
	torque.y = 0;
	torque.z = 0;
}

void Body::integrateVelocity(double et)
{
	int dx = 0;
	int dy = 0;
	Vector3D angularRotation;
	if (bodyState != STATE_REST)
	{
		dx = int(velocity.x * 10000) * int(et * 10000);
		dy = int(velocity.y * 10000) * int(et * 10000);
		position.x = position.x + (double)dx / (double)100000000;
		position.y = position.y + (double)dy / (double)100000000;
		rotMatrix = Matrix3D();
		
		angularRotation = angularVelocity;
		angularRotation = angularRotation * et;
		
		rotMatrix.appendRotation( radiansToDegrees * angularRotation.z, Vector3D::Z_AXIS() , Vector3D(0, 0 ,0));
		rotMatrix.appendRotation(-radiansToDegrees * angularRotation.x, Vector3D::X_AXIS() , Vector3D(0, 0, 0));
		rotMatrix.appendRotation(-radiansToDegrees * angularRotation.y, Vector3D::Y_AXIS() , Vector3D(0, 0, 0));
		upVector = rotMatrix.transformVector(upVector);
		rightVector = rotMatrix.transformVector(rightVector);
	}
}

double Body::normalizeAngle(double angle)
{
   double pi  = M_PI;
   double negpi  = -pi;
   double twopi  = M_PI * 2.0f;
   while (angle < negpi)
   {
	  angle = angle + twopi;
   }
   while (angle > pi)
   {
	  angle = angle - twopi;
   }
   return angle;
}
