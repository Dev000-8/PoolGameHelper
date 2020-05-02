#ifndef _BODY_
#define _BODY_
#include "Vector3D.h"
#include "Matrix3D.h"

class Body
{
public:
	static const int STATE_REST = 0;
	static const int STATE_SLIDING = 1;
	static const int STATE_ROLLING = 2;
	Vector3D position;
	Vector3D org_position;
	Vector3D velocity;
	Vector3D angularVelocity;
	Vector3D upVector;
	Vector3D rightVector;
	double mass;
	double inertia;
	double invMass;
	double invInertia;
	double radius;
	double radiusSquared;
	int bodyState;
	Vector3D force;
	Vector3D acceleration;
	Vector3D torque;

	Matrix3D rotMatrix;

	const double radiansToDegrees = 57.29577951308232f;
	const double threshold = 0.01f;

	Body();
	~Body();
	void reset();
	void setMass(double m);
	void setInertia(double i);
	bool update(double et);
	void addForceComponents(double x , double y , double z);
	void addForce(Vector3D af , Vector3D point);
	void addTorque(Vector3D t);
	void addImpulse(Vector3D i , Vector3D point);
	void integrateForces2(double et);
	void integrateForces(double et);
	void integrateVelocity(double et);
	double normalizeAngle(double angle);
};

#endif

