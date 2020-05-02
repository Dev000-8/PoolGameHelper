#pragma once
class ICollidablePair
{
public:
	ICollidablePair() {}
	
	 double virtual toc() {}
	virtual bool willCollideIn(double param1) {}
	virtual void resolve() {}

};

