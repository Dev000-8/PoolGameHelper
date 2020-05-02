#include "stdafx.h"
#include "GameObject.h"


GameObject::GameObject()
{
	boundingBox = CRectangle();
	aabbColision = false;
	active = true;
}


GameObject::~GameObject()
{

}


bool GameObject::update()
{
	return active;
}


void GameObject::updateAABB(double et)
{
	double xt  = position.x + velocity.x * et;
	double yt  = position.y + velocity.y * et;
	double minx = min(position.x, xt);
	double maxx = max(position.x, xt);
	double miny = min(position.y, yt);
	double maxy = max(position.y, yt);
	boundingBox.left = minx - radius;
	boundingBox.right = maxx + radius;
	boundingBox.top = miny - radius;
	boundingBox.bottom = maxy + radius;
}
