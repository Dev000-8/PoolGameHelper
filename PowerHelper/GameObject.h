#ifndef _GAME_OBJ_
#define _GAME_OBJ_
#include "Body.h"
#include "Rectangle.h"
class GameObject : public Body
{
public:
	bool aabbColision;
	CRectangle boundingBox;
	bool active;
	GameObject();
	~GameObject();
	bool update();
	void updateAABB(double et);
};

#endif
