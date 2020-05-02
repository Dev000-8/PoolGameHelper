#include "stdafx.h"
#include "Ruleset.h"
#include "Wall.h"
# define M_PI           3.14159265358979323846
Ruleset::Ruleset()
{
	tableWidth = 2.54f;
	tableHeight = 1.27f;
	railSize = 0.01905f * 10.0f;
	pocketRadius = 0.05715f * 0.5f * 1.5f * 1.15f;
	pocketRadiusSquared = pocketRadius * pocketRadius;
}


Ruleset::~Ruleset()
{
	
}


void Ruleset::initTable()
{
	double wallX = 0.0808f;
	double wallY = 0.0f;
	double dX = 1.125f + 0.127f;
	double dY = tableHeight;
	double i = 0, j = 0;
	while (i < 2)
	{
		j = 0;
		while (j < 2) {
			Wall wall = Wall(wallX + i * dX + 1.125f * j , wallY + j * dY, wallX + i * dX + 1.125f * (1 - j), wallY + j * dY, false, true);
			rails.push_back(wall);
			walls.push_back(wall);
			j++;
		}
		i++;
	}

	wallX = 0.0f;
	wallY = 0.0808f;
	dX = tableWidth;
	i = 0;
	while (i < 2)
	{
		Wall wall = Wall(wallX + i * dX, wallY + (1 - i) * 1.108f, wallX + i * dX, wallY + 1.108f * i, false, true);
		rails.push_back(wall);
		walls.push_back(wall);
		i++;
	}

	double pocketLength  = pocketRadius * 2.5f;
	double angle  = 2 * M_PI - 142.0f * M_PI / 180;
	Vector3D pocketVector =  Vector3D(cos(angle), sin(angle));
	pocketVector.normalization();

	Wall wallA = Wall(0.0808f + pocketVector.x * pocketLength, pocketVector.y * pocketLength, 0.0808f, 0);
	angle = 142 * M_PI / 180 + M_PI / 2;
	Vector3D pocketVector2  = Vector3D(cos(angle), sin(angle));
	Wall wallB = Wall(0, 0.0808f, pocketVector2.x * pocketLength, 0.0808f + pocketVector2.y * pocketLength);
	Wall wall = Wall(wallB.pointB.x, wallB.pointB.y, wallA.pointA.x, wallA.pointA.y, true);
	walls.push_back(wallA);
	walls.push_back(wallB);
	walls.push_back(wall);
	pocketVector *= (0 - pocketLength);
	pocketVector2*= (0 - pocketLength);

	wallA = Wall(tableWidth, 0.0808 + 1.108, tableWidth + pocketVector2.x, 0.0808 + 1.108 + pocketVector2.y);
	wallB = Wall(tableWidth - 0.0808 + pocketVector.x, tableHeight + pocketVector.y, tableWidth - 0.0808, tableHeight);
	wall = Wall(wallA.pointB.x, wallA.pointB.y, wallB.pointA.x, wallB.pointA.y, true);
	walls.push_back(wallA);
	walls.push_back(wallB);
	walls.push_back(wall);

	angle = 142.0f * M_PI / 180.0f + M_PI;
	pocketVector = Vector3D(cos(angle), sin(angle));
	angle = 5.0f * M_PI / 2.0f - 142.0f * M_PI / 180.0f;
	pocketVector2 = Vector3D(cos(angle), sin(angle));
	pocketVector *= (pocketLength);
	pocketVector2 *= (pocketLength);

	wallA = Wall(tableWidth - 0.0808f, 0, tableWidth - 0.0808f + pocketVector.x, pocketVector.y);
	wallB = Wall(tableWidth + pocketVector2.x, 0.0808f + pocketVector2.y, tableWidth, 0.0808f);
	wall = Wall(wallA.pointB.x, wallA.pointB.y, wallB.pointA.x, wallB.pointA.y, true);
	walls.push_back(wallA);
	walls.push_back(wallB);
	walls.push_back(wall);

	pocketVector *= (-1);
	pocketVector2 *= (-1);
	wallA = Wall(pocketVector2.x, pocketVector2.y + tableHeight - 0.0808f, 0, tableHeight - 0.0808f);
	wallB = Wall(0.0808f, tableHeight, 0.0808f + pocketVector.x, tableHeight + pocketVector.y);
	wall = Wall(wallB.pointB.x, wallB.pointB.y, wallA.pointA.x, wallA.pointA.y, true);
	walls.push_back(wallA);
	walls.push_back(wallB);
	walls.push_back(wall);

	angle = 104.0f * M_PI / 180.0f + M_PI;
	pocketVector = Vector3D(cos(angle), sin(angle));
	angle = 2.0f * M_PI - 104.0f * M_PI / 180.0f;
	pocketVector2 = Vector3D(cos(angle), sin(angle));
	pocketVector *= (pocketLength);
	pocketVector2 *= (pocketLength);

	wallA = Wall(0.0808f + 1.125f, 0, 0.0808f + 1.125f + pocketVector.x, pocketVector.y);
	wallB = Wall(0.0808f + 1.125f + 0.127f + pocketVector2.x, pocketVector2.y, 0.0808f + 1.125f + 0.127f, 0);
	wall = Wall(wallA.pointB.x, wallA.pointB.y, wallB.pointA.x, wallB.pointA.y, true);
	walls.push_back(wallA);
	walls.push_back(wallB);
	walls.push_back(wall);

	pocketVector *= (-1);
	pocketVector2 *= (-1);

	wallA = Wall(0.0808f + 1.125f + pocketVector2.x, tableHeight + pocketVector2.y, 0.0808f + 1.125f, tableHeight);
	wallB = Wall(0.0808f + 1.125f + 0.127f, tableHeight, 0.0808f + 1.125f + 0.127f + pocketVector.x, pocketVector.y + tableHeight);
	wall = Wall(wallB.pointB.x, wallB.pointB.y, wallA.pointA.x, wallA.pointA.y, true);
	walls.push_back(wallA);
	walls.push_back(wallB);
	walls.push_back(wall);

	double pocketX = 0;
	double pocketY = 0;
	double dx = tableWidth;
	double dy = tableHeight;
	i = 0;
	while (i < 2)
	{
		j = 0;
		while (j < 2)
		{
			Vector3D pocket = Vector3D(pocketX + dx * j, pocketY + dy * i, 0/*, POCKET_CORNER*/);
			pocketPositions.push_back (pocket);
			j++;
		}
		i++;
	}
	pocketX = tableWidth * 0.5;
	pocketY = 0 - pocketRadius;
	dy = 2 * pocketRadius + tableHeight;
	i = 0;
	while (i < 2)
	{
		Vector3D pocket2 = Vector3D(pocketX, pocketY + dy * i, 0/*, POCKET_SIDE*/);
		pocketPositions.push_back(pocket2);
		i++;
	}

	wall = Wall(0, 0, tableWidth, 0);
	quickWallCheck.push_back(wall);
	wall = Wall(tableWidth, 0, tableWidth, tableHeight);
	quickWallCheck.push_back(wall);
	wall = Wall(tableWidth, tableHeight, 0, tableHeight);
	quickWallCheck.push_back(wall);
	wall = Wall(0, tableHeight, 0, 0);
	quickWallCheck.push_back(wall);

}
