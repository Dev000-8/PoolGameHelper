
#include "stdafx.h"
#include "PowerHelper.h"
#include <list>
#include "Ruleset.h"
#include "Wall.h"
#include "Ball.h"
#include "BallWallPair.h"
#include "BallBallPair.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <math.h>
using namespace cv;
using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Control(HWND, UINT, WPARAM, LPARAM);


Ruleset *mRuleSet = NULL;
Vector3D clickPosition;


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_POWERHELPER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)::GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL; //MAKEINTRESOURCEW(IDC_POWERHELPER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

#include <windowsx.h>
#include <time.h>

void DrawFrame(BOOL bErase);
static HDC mDesktopBufferHDC = 0;
static HWND mHwnd = 0;
//static int mSelectedMode;
static HWND mCtrlPanel = 0;

using namespace std;
list<Ball> mBallList;
static RECT mTableRect;
Ball *currentCueBall;

enum {
	MODE_NONE = 0,
	MODE_TABLE = 1,
	MODE_SIM,
	MODE_REAL
} DRAW_MODE;


double GetLocalPosition(double offsetx, double x, double scale)
{
	return ((x - offsetx) / scale);
}

typedef BOOL(*_InstallHook)(HWND hwnd);
typedef BOOL(*_UninstallHook)();
_InstallHook installhook = 0;
_UninstallHook uninstallhook = 0;

#define GWM_LBUTTONDOWN (WM_USER + 0x2000)
#define GWM_LBUTTONUP (WM_USER + 0x2001)
#define GWM_MOUSEMOVE (WM_USER + 0x2002)
#define GWM_RBUTTONDOWN (WM_USER + 0x2003)
#define GWM_KEYDOWN (WM_USER + 0x2004)
#define GWM_KEYUP (WM_USER + 0x2005)

Mat hwnd2mat(HWND hwnd);
RECT PredictTable(Mat img);

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   int screenMaxX = GetSystemMetrics(SM_CXSCREEN);
   int screenMaxY = GetSystemMetrics(SM_CYSCREEN);

   mRuleSet = new Ruleset();
   mRuleSet->initTable();

   HWND hWnd = ::CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED,
	   szWindowClass,
	   L"Pool Assistant",
	   WS_POPUP | WS_VISIBLE,
	   CW_USEDEFAULT, CW_USEDEFAULT,
	   screenMaxX, screenMaxY,
	   NULL, NULL,
	   hInstance,
	   NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   mHwnd = hWnd;
   ::SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

  
   static HINSTANCE hookdll;
   hookdll = LoadLibrary(L"controller.dll");
   installhook = (_InstallHook)GetProcAddress(hookdll , "installhook");
   uninstallhook = (_UninstallHook)GetProcAddress(hookdll, "uninstallhook");
   installhook(mHwnd);

   mCtrlPanel =  CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CONTROL), hWnd, Control);
   ShowWindow(mCtrlPanel, SW_SHOW);

   //mSelectedMode = MODE_NONE;
   mBallList.empty();

   return TRUE;
}

list<Vector3D> cueBallCollisions;
Vector3D firstCollisions;
Vector3D firstTargetBall;
bool ballHit = false;
bool wallHit = false;

Vector3D closestPosition;
Vector3D targetPosition;

double closestDistance;
Ball ghostBall;
Vector3D normalResult;
Vector3D tangentResult;
Vector3D CuePosition;
Vector3D tipPosition;

double maxLength;
double aimVectorLength;
double normalScale;
double tanScale;
int guideLevel = 0;
bool isSimulation = false;
bool isShowCue = false;
int powerLevel = 0;

double getPowerMultiplier(int level)
{
	if (level == 0)
		return 1;
	else
		return (1 + level * 0.35f );
}

double _powerMultiplier = 1; // 1 + this.powerLevel * 0.35
double screenToWorldLength(double offset, double value, double scale)
{
	double normalize = (value - offset) / scale;
	return normalize;
}

double MaxPower()
{
	return (double)(int((0.55 + 9 * 0.02) * 100.0f)) / 100;
}

void PredictCueBall(double posX, double posY);

void Calculate_CuePosition()
{
	CuePosition.x = 0;
	CuePosition.y = 0;
}

void OnHit(double et , double x , double y , Vector3D aim)
{
	
	if (currentCueBall == NULL) return;

	Vector3D cuePosition;
	cuePosition.x = currentCueBall->position.x;
	cuePosition.y = currentCueBall->position.y;
	Vector3D currentPosition;
	currentPosition.x = x;
	currentPosition.y = y;

	Vector3D shootVector;
	shootVector = currentPosition - clickPosition;
	double power = shootVector.dot_product(aim) / aim.dot_product(aim);
	/*
		CuePosition :: Currently does not support spin.
	*/
	Calculate_CuePosition();
	/*************************************************/
	if (power < 0 && !isSimulation)
	{
		double tmp = 0 - MaxPower();
		power = max(power, tmp);
		power = power * getPowerMultiplier(powerLevel);
		Vector3D hitVector  = aim;
		Vector3D torqueVec = hitVector;
		hitVector = hitVector * (power * -2.0f);
		torqueVec = torqueVec * (0 - currentCueBall->radius);
		Matrix3D matrix = Matrix3D();
		matrix.appendRotation(CuePosition.x * (0 - M_PI), Vector3D(0, 0, 1) , Vector3D(0,0,0));
		torqueVec = matrix.transformVector(torqueVec);
		torqueVec = torqueVec * 4.0f;
		torqueVec.z = currentCueBall->radius * CuePosition.y / 2;

		if (currentCueBall->active == false)
		{
			currentCueBall->active = true;
			currentCueBall->reset();
			currentCueBall->bodyState = Body::STATE_REST;
			currentCueBall->ballState = Ball::BALL_STATE_ACTIVE;

		}

		currentCueBall->addImpulse(hitVector, torqueVec);
		isSimulation = true;
		
	}
	
}

bool aabbCollide(CRectangle a, CRectangle b )
{
   if (a.right < b.left)
   {
	  return false;
   }
   if (a.left > b.right)
   {
	  return false;
   }
   if (a.top > b.bottom)
   {
	  return false;
   }
   if (a.bottom < b.top)
   {
	  return false;
   }
   return true;
}

void ccd(double et)
{
	int MAX_ITER = 100;
	int iter = 0;
	double dt = 0;
	double t = 0;
	double minT = 0;
	double timeSlice = et;
	BallWallPair *minBallWall = NULL;
	BallWallPair wallPair;
	BallBallPair ballPair;
	BallBallPair *minBallBall = NULL;
	bool isAnyMoving = false;
	while (t < timeSlice && ++iter <= MAX_ITER)
	{
		dt = timeSlice - t;
		minT =  pow(2, 1204);
		if (minBallWall) {
			delete minBallBall;
		}
		minBallWall = NULL;
		if (minBallBall) {
			delete minBallBall;
		}
		minBallBall = NULL;

		for (auto i = rbegin(mBallList); i != rend(mBallList); i++)
		{
			Ball &goa(*i);
			goa.updateAABB(dt);
			if (!(goa.velocity.x == 0 && goa.velocity.y == 0))
			{
				isAnyMoving = true;
				for (Wall wall : mRuleSet->walls)
				{
					if (goa.velocity.dot_product(wall.normal) < 0 && aabbCollide(goa.boundingBox, wall.aabb))
					{
						wallPair.ball = &goa;
						wallPair.wall = &wall;
						if (wallPair.willCollideIn(dt))
						{
							if (wallPair.toc() < minT)
							{
								minT = wallPair.toc();
								if (minBallWall) delete minBallWall;
								minBallWall = new BallWallPair();
								minBallWall->copy(wallPair);
								minBallWall->resolve();
							}
						}
					}
				}
			}
		}

		for (auto i = rbegin(mBallList); i != rend(mBallList); i++)
		{
			Ball &goa(*i);
			if (goa.active != false)
			{
				for (auto j = rbegin(mBallList); j!= rend(mBallList); j++)
				{
					Ball &gob(*j);
					if (gob.active != false)
					{
						if (aabbCollide(goa.boundingBox, gob.boundingBox))
						{
							ballPair.goa = &goa;
							ballPair.gob = &gob;
							if (ballPair.willCollideIn(dt))
							{
								if (ballPair.toc() < minT)
								{
									minT = ballPair.toc();
									if (minBallBall) delete minBallBall;
									minBallBall = new BallBallPair();
									minBallBall->copy(ballPair);
								}
							}
						}
					}
				}
			}
		}
		if (minT < pow(2, 1204))
		{
			dt = minT;
		}
		for (auto i = rbegin(mBallList); i != rend(mBallList); i++)
		{
			Ball &ball(*i);
			ball.integrateVelocity(dt);
		}

		if (minBallWall && minBallBall != NULL)
		{
			minBallWall->resolve();
			delete minBallWall;
			minBallWall = NULL;
		}

		if (minBallBall)
		{
			minBallBall->resolve();
			delete minBallBall;
			minBallBall = NULL;
		}
		t = t + dt;
	}

	if (!isAnyMoving)
		isSimulation = false;
}

void worldUpdate(double et)
{
	for (auto i = rbegin(mBallList); i != rend(mBallList); i++)
	{
		Ball &ball(*i);
		ball.update(et);
		ball.integrateForces(et);
	}

	ccd(et);
}

void findCueCollisions(int depth, Vector3D start, Vector3D direction)
{
	int i;
	Vector3D B;
	Vector3D C;
	Vector3D AB;
	Vector3D AC;
	Ball focusBall;
	double ACdotAB;
	Vector3D BC;
	double D;
	Wall focusWall;
	Vector3D wallNormal;
	Vector3D wallNormalScaled;
	double test;
	double scale;
	if (depth == 0)
	{
		return;
	}
	depth--;

	//var balls : Vector.<Ball> = Globals.model.ruleset.balls;
	Ball *cueBall = currentCueBall;
	int numObjects = mBallList.size();
	if (numObjects <= 0 || cueBall == NULL)
		return;

	double radiusCheck = cueBall->radius * cueBall->radius * 4;
	Vector3D closestPosition;
	double closestDistance = pow(2, 1204);
	Vector3D A = start;
	i = 0;
	for (auto i = rbegin(mBallList); i != rend(mBallList); i++)
	{
		Ball &focusBall(*i);

		if (focusBall.active != false)
		{
			B = focusBall.position;
			AB = B - A;
			AC = direction;
			ACdotAB = AB.dot_product(AC);
			if (ACdotAB > 0)
			{
				C = Vector3D(AC.x * ACdotAB + A.x, AC.y * ACdotAB + A.y);
				BC = C - B;
				if (BC.square() < radiusCheck * 0.99)
				{
					D = radiusCheck - BC.square();
					D = sqrt(D);
					C = Vector3D(C.x - AC.x * D, C.y - AC.y * D);
					AC = C - (A);
					if (AC.magnitude() < closestDistance)
					{
						closestDistance = AC.magnitude();
						closestPosition.x = C.x;
						closestPosition.y = C.y;
						targetPosition = focusBall.position;
						ballHit = true;
					}
				}
			}
		}
		
	}
	
	for (auto i = rbegin(mRuleSet->quickWallCheck); i != rend(mRuleSet->quickWallCheck); i++)
	{
		Wall &focusWall(*i);
		
		wallNormal = focusWall.normal;
		wallNormalScaled = focusWall.normal;
		wallNormalScaled = wallNormalScaled * (currentCueBall->radius);
		C = focusWall.pointA + (wallNormalScaled);
		AC = C - (A);
		test = direction.dot_product(wallNormal);
		if (test < 0)
		{
			scale = AC.dot_product(wallNormal) / test;
			B = Vector3D(A.x + direction.x * scale, A.y + direction.y * scale);
			AB = B - (A);
			if (AB.magnitude() < closestDistance)
			{
				closestDistance = AB.magnitude();
				closestPosition.x = B.x;
				closestPosition.y = B.y;
				wallHit = true;
				ballHit = false;
				normalResult.x = wallNormal.x;
				normalResult.y = wallNormal.y;
			}
		}

	}
	if (ballHit)
	{
		if (firstCollisions.x == 0 && firstCollisions.y == 0)
		{
			firstCollisions = closestPosition;
			firstTargetBall.x = targetPosition.x;
			firstTargetBall.y = targetPosition.y;
		}
		if (depth == 0) {
			cueBallCollisions.push_back(closestPosition);
		}
		else {
			cueBallCollisions.push_back(closestPosition);
			cueBallCollisions.push_back(targetPosition);
		}

		normalResult.x = targetPosition.x - closestPosition.x;
		normalResult.y = targetPosition.y - closestPosition.y;
		normalResult.normalization();
		findCueCollisions(depth, targetPosition, normalResult);
	}
	else 
	{
		normalResult = normalResult * (-1.7 * normalResult.dot_product(direction));
		tangentResult = direction + (normalResult);
		tangentResult.normalization();
		cueBallCollisions.push_back(closestPosition);
		float wi = mRuleSet->tableWidth;
		float hi = mRuleSet->tableHeight;
		
		/*char dbgStr[250];
		sprintf_s(dbgStr, 250, "(%.2f,%.2f) %.2f,%.2f\n", mRuleSet->tableWidth, mRuleSet->tableHeight, closestPosition.x, closestPosition.y);
		OutputDebugStringA(dbgStr);*/

		bool isInPocket = false;
		// top middle pocket
		if (closestPosition.x >= 1.23 && closestPosition.x <= 1.31 && closestPosition.y <= 0.03)
			isInPocket = true;
		// bottom middle pocket
		else if (closestPosition.x >= 1.23 && closestPosition.x <= 1.31 && closestPosition.y >= 1.24)
			isInPocket = true;
		// top left
		else if (closestPosition.x <= 0.03 && ( closestPosition.y <= 0.07 ) || closestPosition.x <= 0.07 && (closestPosition.y <= 0.03))
			isInPocket = true;
		// bottom left
		else if (closestPosition.x <= 0.03 && (closestPosition.y >= (1.24 - 0.04)) || 
			closestPosition.x <= 0.08 && (closestPosition.y >= 1.24))
			isInPocket = true;
		// bottom right
		else if (closestPosition.x >= 2.51 && (closestPosition.y >= (1.24 - 0.04)) ||
			closestPosition.x >= (2.51 - 0.03) && (closestPosition.y >= 1.24))
			isInPocket = true;
		// top right
		else if (closestPosition.x >= 2.51 && (closestPosition.y <= 0.07) ||
			closestPosition.x >= (2.51 - 0.03) && (closestPosition.y <= 0.03))
			isInPocket = true;

		if(!isInPocket)
			findCueCollisions(depth, closestPosition, tangentResult);
	}
}

void PredictCueBall(double posX, double posY)
{
	double scale = (mTableRect.right - mTableRect.left) / mRuleSet->tableWidth;
	ballHit = false;
	wallHit = false;
	guideLevel = 4;
	cueBallCollisions.clear();
	firstCollisions.x = firstCollisions.y = 0;
	if (currentCueBall != NULL)
	{

		Vector3D aimVector;
		aimVector.x = posX- currentCueBall->position.x;
		aimVector.y = posY - currentCueBall->position.y;
		aimVector.z = 0;
		aimVector.normalization();
		findCueCollisions(4, currentCueBall->position, aimVector);
		if (firstCollisions.x != 0 && firstCollisions.y != 0)
		{
			closestPosition = firstCollisions;
			//closestPosition = cueBallCollisions.back();  //[cueBallCollisions.size() - 1];
			closestDistance = (closestPosition - (currentCueBall->position)).magnitude();
			ghostBall.position.x = closestPosition.x;
			ghostBall.position.y = closestPosition.y;
			normalResult.x = firstTargetBall.x - ghostBall.position.x;
			normalResult.y = firstTargetBall.y - ghostBall.position.y;
			maxLength = 6 * currentCueBall->radius;
			aimVectorLength = -1 / 4 * mRuleSet->tableWidth * closestDistance + 1;
			normalResult.normalization();
			tangentResult.x = normalResult.y;
			tangentResult.y = 0 - normalResult.x;
			normalScale = normalResult.dot_product(aimVector);
			tanScale = tangentResult.dot_product(aimVector);
			if (guideLevel == 0)
			{
				aimVectorLength = -1 / 2 * mRuleSet->tableWidth * closestDistance + 1;
			}
			else if (guideLevel == 1)
			{
				aimVectorLength = -1 / 4 * mRuleSet->tableWidth * closestDistance + 1;
			}
			else if (guideLevel == 2)
			{
				aimVectorLength = -1 / 8 * mRuleSet->tableWidth * closestDistance + 1;
			}
			else if (guideLevel == 3)
			{
				maxLength = 8 * currentCueBall->radius;
				aimVectorLength = -1 / (8 * mRuleSet->tableWidth * closestDistance) + 1;
			}
			else if (guideLevel == 4)
			{
				maxLength = 12 * currentCueBall->radius;
				aimVectorLength = 1;
			}
			if (aimVectorLength > 1)
			{
				aimVectorLength = 1;
			}
			else if (aimVectorLength < 0)
			{
				aimVectorLength = 0;
			}
			aimVectorLength = aimVectorLength * maxLength;
			normalResult = normalResult * (normalScale * aimVectorLength);
			tangentResult = tangentResult * (tanScale * aimVectorLength);
			normalResult = normalResult + (ghostBall.position);
			tangentResult = tangentResult + (ghostBall.position);
		}
	}
}

BOOL isCaptured()
{
	return (mDesktopBufferHDC != 0 ? true : false);
}

void releaseDesktop()
{
	if (mDesktopBufferHDC)
	{
		DeleteObject(mDesktopBufferHDC);
		mDesktopBufferHDC = NULL;
	}
}



void CaptureDesktop()
{
	DrawFrame(true);

	if (mCtrlPanel != 0)
	{
		ShowWindow(mCtrlPanel, SW_HIDE);
		Sleep(500);
	}

	HDC pDesktopDC = GetDCEx(NULL, NULL, 0);
	int screenMaxX = GetSystemMetrics(SM_CXSCREEN);
	int screenMaxY = GetSystemMetrics(SM_CYSCREEN);
	if (mDesktopBufferHDC != NULL)
	{
		DeleteObject(mDesktopBufferHDC);
	}
	mDesktopBufferHDC = CreateCompatibleDC(pDesktopDC);

	HBITMAP hBMP;
	hBMP = CreateCompatibleBitmap(pDesktopDC, screenMaxX, screenMaxY);
	SelectObject(mDesktopBufferHDC , hBMP);

	BitBlt(mDesktopBufferHDC , 0, 0, screenMaxX, screenMaxY, pDesktopDC, 0, 0, SRCCOPY);
		
	LPCTSTR szmark = { L"Capture Mode" };
	COLORREF color = RGB(239, 15, 225);
	SetTextColor(mDesktopBufferHDC , color);
	SetBkMode(mDesktopBufferHDC , TRANSPARENT);
	TextOut(mDesktopBufferHDC , 10, 10, szmark, _tcslen(szmark));

	ReleaseDC(NULL, pDesktopDC);
	DeleteObject(hBMP);
	if (mCtrlPanel != 0)
	{
		ShowWindow(mCtrlPanel, SW_SHOW);
	}

}

#pragma comment( lib, "winmm.lib")
#include <Windows.h>
time_t _lastUpdateTime;
void DrawFrame(BOOL bErase)
{
	int screenMaxX = GetSystemMetrics(SM_CXSCREEN);
	int screenMaxY = GetSystemMetrics(SM_CYSCREEN);

	time_t currentTime = timeGetTime();
	float et = currentTime - _lastUpdateTime;
	_lastUpdateTime = currentTime;
	if (et > 2000)
	{
		et = 2000;
	}
	et = et / 1000;

	worldUpdate(et);
	HDC hdc = GetDC(mHwnd);
	if (hdc == NULL) return;
	HDC hMem = CreateCompatibleDC(hdc);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, screenMaxX , screenMaxY);
	HBITMAP hOld = (HBITMAP)SelectObject(hMem, hBmp);

	// Background process.
	/*if (mSelectedMode == MODE_REAL || mSelectedMode == MODE_NONE || bErase)*/
	/*{*/
		HBRUSH br = CreateSolidBrush(RGB(0, 0, 0));
		HGDIOBJ oldbr = SelectObject(hMem, br);
		Ellipse(hMem, 0, 0, screenMaxX, screenMaxY);
		SelectObject(hMem, oldbr);
		DeleteObject(br);
	/*}
	else
	{
		/*BitBlt(hMem, 0, 0, screenMaxX, screenMaxY, mDesktopBufferHDC, 0, 0, SRCCOPY);
	}*/

	double scale = (mTableRect.right - mTableRect.left) / mRuleSet->tableWidth;

	if (!bErase) {
		// Draw table
		HPEN hPen = CreatePen(PS_SOLID, 2, RGB(136, 0, 21));
		HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // CreateSolidBrush(RGB(0, 0, 0));
		HPEN hWhitePen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
		HBRUSH hActiveBrush = CreateSolidBrush(RGB(0, 56, 182));
		HGDIOBJ oldPen = SelectObject(hMem, hPen);
		HGDIOBJ oldBrush = SelectObject(hMem, hBrush);
		if (mRuleSet != NULL)
		{
			/*for (Wall w : mRuleSet->rails)
			{
				MoveToEx(hMem, mTableRect.left + w.pointA.x * scale, mTableRect.top + w.pointA.y * scale, NULL);
				LineTo(hMem, mTableRect.left + w.pointB.x * scale, mTableRect.top + w.pointB.y * scale);
				//Rectangle(hMem, mTableRect.left + w.aabb.left * scale , mTableRect.top + w.aabb.top * scale , mTableRect.left + w.aabb.right * scale , mTableRect.top + w.aabb.bottom * scale);
			}*/
			for (Wall w : mRuleSet->walls)
			{
				MoveToEx(hMem, mTableRect.left + w.pointA.x * scale, mTableRect.top + w.pointA.y * scale, NULL);
				LineTo(hMem, mTableRect.left + w.pointB.x * scale, mTableRect.top + w.pointB.y * scale);
				//Rectangle(hMem, mTableRect.left + w.pointA.x * scale, mTableRect.top + w.pointA.y * scale, mTableRect.left + w.pointB.x * scale, mTableRect.top + w.pointB.y * scale);
			}
			/*for (Wall w : mRuleSet->quickWallCheck)
			{
				MoveToEx(hMem, mTableRect.left + w.pointA.x * scale, mTableRect.top + w.pointA.y * scale, NULL);
				LineTo(hMem , mTableRect.left + w.pointB.x * scale, mTableRect.top + w.pointB.y * scale);
				
				//Rectangle(hMem, mTableRect.left + w.pointA.x * scale, mTableRect.top + w.pointA.y * scale, mTableRect.left + w.pointB.x * scale, mTableRect.top + w.pointB.y * scale);
			}*/
		}

		if (currentCueBall && GetAsyncKeyState(VK_CONTROL))
		{
			//if (mSelectedMode != MODE_REAL) 
			{

				bool skipFirstLine = true;
				if (wallHit) {
					MoveToEx(hMem,
						mTableRect.left + currentCueBall->position.x * scale,
						mTableRect.top + currentCueBall->position.y * scale, NULL);

					for (Vector3D point : cueBallCollisions)
					{

						LineTo(hMem,
							mTableRect.left + point.x * scale,
							mTableRect.top + point.y * scale);
						Ellipse(hMem,
							mTableRect.left + point.x * scale - currentCueBall->radius * scale,
							mTableRect.top + point.y * scale - currentCueBall->radius * scale,
							mTableRect.left + point.x * scale + currentCueBall->radius * scale,
							mTableRect.top + point.y * scale + currentCueBall->radius * scale);
						LineTo(hMem,
							mTableRect.left + point.x * scale,
							mTableRect.top + point.y * scale);

						if (skipFirstLine)
						{
							skipFirstLine = false;
						}
					}
				}
			}
			if(firstCollisions.x != 0 && firstCollisions.y != 0)
			{

				Ellipse(hMem,
					mTableRect.left + ghostBall.position.x * scale - currentCueBall->radius * scale,
					mTableRect.top + ghostBall.position.y * scale - currentCueBall->radius * scale,
					mTableRect.left + ghostBall.position.x * scale + currentCueBall->radius * scale,
					mTableRect.top + ghostBall.position.y * scale + currentCueBall->radius * scale);
				/*MoveToEx(hMem,
					mTableRect.left + currentCueBall->position.x * scale,
					mTableRect.top + currentCueBall->position.y * scale, NULL);
				LineTo(hMem,
					mTableRect.left + ghostBall.position.x * scale,
					mTableRect.top + ghostBall.position.y * scale);*/

				/*MoveToEx(hMem,
					mTableRect.left + ghostBall.position.x * scale,
					mTableRect.top + ghostBall.position.y * scale, NULL);
				LineTo(hMem,
					mTableRect.left + normalResult.x * scale,
					mTableRect.top + normalResult.y * scale);*/

				MoveToEx(hMem,
					mTableRect.left + ghostBall.position.x * scale,
					mTableRect.top + ghostBall.position.y * scale, NULL);
				LineTo(hMem,
					mTableRect.left + tangentResult.x * scale,
					mTableRect.top + tangentResult.y * scale);

			}

			if (isShowCue) {

				Ellipse(hMem, mTableRect.left + tipPosition.x * scale - 4, mTableRect.top + tipPosition.y * scale - 4
					, mTableRect.left + tipPosition.x * scale + 4, mTableRect.top + tipPosition.y  * scale + 4);

			}
		}
		/*Rectangle(hMem, mTableRect.left,	mTableRect.top,	 mTableRect.right, mTableRect.bottom);
		Rectangle(hMem, mTableRect.left,	mTableRect.top,	 mTableRect.right, -mTableSize + mTableRect.top);
		Rectangle(hMem, mTableRect.right,	mTableRect.top,	 mTableSize + mTableRect.right, mTableRect.bottom);
		Rectangle(hMem, mTableRect.left,	mTableRect.top,	 mTableRect.left - mTableSize, mTableRect.bottom);
		Rectangle(hMem, mTableRect.left,	mTableRect.bottom, mTableRect.right, +mTableSize + mTableRect.bottom);*/
		// draw ball
		
		//if (mSelectedMode != MODE_REAL)
		{
			for (Ball b : mBallList)
			{
				HPEN hBallPen = CreatePen(PS_SOLID, 2, RGB(255, 69, 0));
				//HBRUSH hBallBrush = CreateSolidBrush(*RGB(48, 255, 0));
				HBRUSH hBallBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
				if (b.ballType == Ball::BALL_CUE) {

					/*if (b.velocity.x == 0 && b.velocity.y == 0) {
						SelectObject(hMem, hBallPen);
						SelectObject(hMem, hBrush);
						Rectangle(hMem, mTableRect.left + b.position.x * scale - b.radius * scale, mTableRect.top + b.position.y * scale - b.radius * scale
							, mTableRect.left + b.position.x * scale + b.radius * scale, mTableRect.top + b.position.y  * scale + b.radius * scale);
					}
					else {*/
					SelectObject(hMem, hBallPen);
					SelectObject(hMem, hActiveBrush);
					Ellipse(hMem, mTableRect.left + b.position.x * scale - b.radius * scale, mTableRect.top + b.position.y * scale - b.radius * scale
						, mTableRect.left + b.position.x * scale + b.radius * scale, mTableRect.top + b.position.y  * scale + b.radius * scale);
					Ellipse(hMem, mTableRect.left + b.position.x * scale - 2, mTableRect.top + b.position.y * scale - 2
						, mTableRect.left + b.position.x * scale + 2, mTableRect.top + b.position.y  * scale + 2);
					/*}*/
				}
				else {
					SelectObject(hMem, hBallPen);
					SelectObject(hMem, hBallBrush);
					Ellipse(hMem, mTableRect.left + b.position.x * scale - b.radius * scale, mTableRect.top + b.position.y * scale - b.radius * scale
						, mTableRect.left + b.position.x * scale + b.radius * scale, mTableRect.top + b.position.y  * scale + b.radius * scale);

					Ellipse(hMem, mTableRect.left + b.position.x * scale - 2, mTableRect.top + b.position.y * scale - 2
						, mTableRect.left + b.position.x * scale + 2, mTableRect.top + b.position.y  * scale + 2);
				}



				DeleteObject(hBallPen);
				//DeleteObject(hBallBrush);
			}
		}

		DeleteObject(hActiveBrush);
		DeleteObject(hWhitePen);
		// render memdc
		BitBlt(hdc, 0, 0, screenMaxX, screenMaxY, hMem, 0, 0, SRCCOPY);
		SelectObject(hMem , oldPen);
		SelectObject(hMem , oldBrush);
		DeleteObject(hPen);
		//DeleteObject(hBrush);
	}

	
	// Desktop Render
	BitBlt(hdc, 0, 0, screenMaxX, screenMaxY, hMem, 0, 0, SRCCOPY);

	// Resource Free
	SelectObject(hMem, hOld);
	ReleaseDC(mHwnd, hdc);
	DeleteObject(hMem);
	DeleteObject(hBmp);

}


BOOL bClicked = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

	case GWM_KEYDOWN:
	{
		if (currentCueBall != NULL)
		{
			if (wParam == VK_LEFT)
			{
				currentCueBall->position.x -= 0.001;
				currentCueBall->org_position = currentCueBall->position;
			}
			if (wParam == VK_RIGHT)
			{
				currentCueBall->position.x += 0.001;
				currentCueBall->org_position = currentCueBall->position;
			}
			if (wParam == VK_UP)
			{
				currentCueBall->position.y -= 0.001;
				currentCueBall->org_position = currentCueBall->position;
			}
			if (wParam == VK_DOWN)
			{
				currentCueBall->position.y += 0.001;
				currentCueBall->org_position = currentCueBall->position;
			}


		}
	}
	break;
	case GWM_KEYUP:
	{
		/*if (wParam == 'R')
		{
			for (auto i = rbegin(mBallList); i != rend(mBallList); i++) {

				Ball &ball(*i);
				ball.position = ball.org_position;
			}
		}
		else*/ if (wParam == 'R')
		{
			DrawFrame(true);
			ShowWindow(mCtrlPanel, SW_HIDE);
			//Sleep(500);
			HWND hwndDesktop = GetDesktopWindow();
			Mat src = hwnd2mat(hwndDesktop);
			ShowWindow(mCtrlPanel, SW_SHOW);
			mTableRect = PredictTable(src);
		}
	}
	break;
	case GWM_LBUTTONDOWN:
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		bClicked = true;
		double scale = (mTableRect.right - mTableRect.left) / mRuleSet->tableWidth;

		if (GetAsyncKeyState(VK_SHIFT)) // table start
		{
			mTableRect.left = xPos;
			mTableRect.top = yPos;
			mTableRect.right = xPos;
			mTableRect.bottom = yPos;
		}
		else if (GetAsyncKeyState(VK_CONTROL))  // ball draw or select
		{ 
			if (GetAsyncKeyState(VK_SPACE))
			{
				clickPosition.x = GetLocalPosition(mTableRect.left, xPos, scale);
				clickPosition.y = GetLocalPosition(mTableRect.top, yPos, scale);
			}
			else 
			{
				//if (GetAsyncKeyState())
				{

					BOOL ballSelected = false;
					currentCueBall = NULL;
					for (auto i = rbegin(mBallList); i != rend(mBallList); i++) {

						Ball &ball(*i);
						// check ball is selected
						if ((xPos - mTableRect.left) > ball.position.x * scale - ball.radius * scale &&
							(xPos - mTableRect.left) < ball.position.x * scale + ball.radius * scale &&
							(yPos - mTableRect.top) > ball.position.y * scale - ball.radius * scale &&
							(yPos - mTableRect.top) < ball.position.y * scale + ball.radius * scale)
						{
							ballSelected = true;
							ball.ballType = Ball::BALL_CUE;
							currentCueBall = &ball;
						}
						else
						{
							ball.ballType = Ball::BALL_OTHER;
						}
					}
					if (!ballSelected) { // ball is not selected , new ball create.
						// Ball Creation
						Ball b;
						b.position.x = (xPos - mTableRect.left) / scale;
						b.position.y = (yPos - mTableRect.top) / scale;

						b.org_position = b.position;

						b.ballType = Ball::BALL_CUE;
						srand((unsigned)time(NULL));
						mBallList.push_back(b);
						currentCueBall = &mBallList.back();
					}
				}
			}
		}
	}
	break;
	case GWM_LBUTTONUP:
	{
		bClicked = false;
	
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		double scale = (mTableRect.right - mTableRect.left) / mRuleSet->tableWidth;
		if (GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_SPACE)) 
		// if only space bar clicked , simulation start.
		{
			Vector3D aimVector;
			if (currentCueBall != NULL)
			{

				aimVector.x = clickPosition.x - currentCueBall->position.x;
				aimVector.y = clickPosition.y - currentCueBall->position.y;
				aimVector.z = 0;
				aimVector.normalization();

				OnHit(0.001f, GetLocalPosition(mTableRect.left, xPos, scale), GetLocalPosition(mTableRect.top, yPos, scale), aimVector);
			}
			//while(isSimulation)
			//	worldUpdate(0.017);
		}
		clickPosition.x = clickPosition.y = 0;
	}break;
	case GWM_MOUSEMOVE:
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		double scale = (mTableRect.right - mTableRect.left) / mRuleSet->tableWidth;

		if (GetAsyncKeyState(VK_SHIFT) && mTableRect.left > 0) // table move
		{
			if (GetAsyncKeyState(VK_SPACE))
			{
				int deltax = 0, deltay = 0;
				deltax = xPos - mTableRect.right;
				deltay = yPos - mTableRect.bottom;
				mTableRect.right += deltax;
				mTableRect.bottom += deltay;
				mTableRect.left += deltax;
				mTableRect.top += deltay;
			}
			else
			{
				mTableRect.right = xPos;
				mTableRect.bottom = yPos;
			}   
		}
		else if (GetAsyncKeyState(VK_CONTROL)) // ball move
		{
			if (bClicked&& !GetAsyncKeyState(VK_SPACE))
			{
				if (currentCueBall) // cueball move, that is,  cueball  is the selected ball.
				{
					currentCueBall->position.x = GetLocalPosition(mTableRect.left, xPos, scale);
					currentCueBall->position.y = GetLocalPosition(mTableRect.top, yPos, scale);
					currentCueBall->org_position = currentCueBall->position;
				}
			}
			else {

				// assist line draw
				if (clickPosition.x == 0 && clickPosition.y == 0 && !isSimulation)
					PredictCueBall(GetLocalPosition(mTableRect.left, xPos, scale), GetLocalPosition(mTableRect.top, yPos, scale));
				else
					PredictCueBall(clickPosition.x, clickPosition.y);

				if (currentCueBall)
				{
					Vector3D aimVector;
					aimVector.x = clickPosition.x - currentCueBall->position.x;
					aimVector.y = clickPosition.y - currentCueBall->position.y;
					aimVector.z = 0;
					aimVector.normalization();

					Vector3D currentPosition;
					currentPosition.x = GetLocalPosition(mTableRect.left, xPos, scale);;
					currentPosition.y = GetLocalPosition(mTableRect.top, yPos, scale);;

					Vector3D shootVector;
					shootVector = currentPosition - clickPosition;
					double temppower = shootVector.dot_product(aimVector) / aimVector.dot_product(aimVector);
					if (temppower < 0 - MaxPower())
					{
						temppower = 0 - MaxPower();
					}
					double tipMag = min(0.0, temppower) - currentCueBall->radius;

					tipPosition.x = currentCueBall->position.x + aimVector.x * tipMag;
					tipPosition.y = currentCueBall->position.y + aimVector.y * tipMag;
					isShowCue = true;
				}
			}
		}
	}
	break;
	case GWM_RBUTTONDOWN:
	{
		if (mBallList.size() > 0)
		{
			if (currentCueBall == &mBallList.back())
			{
				currentCueBall = NULL;
			}
			if (mBallList.size() > 0) mBallList.pop_back();

		}
	}
	break;

	case WM_LBUTTONDOWN:
	{
		
	}
	break;
	
	case WM_MOUSEMOVE:
	{		
	}
	break;

	case WM_LBUTTONUP:
	{
	}
	break;

	case WM_RBUTTONDOWN:
		
		break;
    case WM_COMMAND:

        return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_KEYDOWN:
	{

	}

	case WM_KEYUP:
	{
		
	}
    case WM_PAINT:
	{
	
	}
        break;
    case WM_DESTROY:
		if (uninstallhook)
		{
			uninstallhook();
		}
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

Mat hwnd2mat(HWND hwnd)
{
	HDC hwindowDC, hwindowCompatibleDC;

	int height, width, srcheight, srcwidth;
	HBITMAP hbwindow;
	Mat src;
	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;    // get the height and width of the screen
	GetClientRect(hwnd, &windowsize);

	srcheight = windowsize.bottom;
	srcwidth = windowsize.right;
	height = windowsize.bottom / 1;  //change this to whatever size you want to resize to
	width = windowsize.right / 1;

	src.create(height, width, CV_8UC4);

	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);

	return src;
}

RECT PredictTable(Mat img)
{
	Mat gray, blurred, canny_img, img_hsv, table_edges_mask, balls_mask, balls_mask_new;
	cvtColor(img, gray, COLOR_BGR2GRAY);
	bilateralFilter(gray, blurred, 5, 15, 15);
	Canny(blurred, canny_img, 30, 150);
	cvtColor(img, img_hsv, COLOR_BGR2HSV);
	inRange(img_hsv, Scalar(67, 0, 116), Scalar(86, 255, 255), table_edges_mask);
	inRange(img_hsv, Scalar(67, 0, 0), Scalar(86, 255, 255), balls_mask);
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	balls_mask.copyTo(balls_mask_new);
	dilate(balls_mask_new, balls_mask_new, kernel, Point(-1, -1), 2);
	erode(balls_mask_new, balls_mask_new, kernel, Point(-1, -1), 2);
	int min_table_area = 0.5 * img.size().width * img.size().height;
	vector<vector<Point> > contours;
	findContours(balls_mask_new, contours, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
	Rect rc(0, 0, 0, 0);
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (contourArea(contours[i]) >= balls_mask_new.size().height)
		{
			if (boundingRect(contours[i]).width > rc.width &&
				boundingRect(contours[i]).height > rc.height)
				rc = boundingRect(contours[i]);
		}
	}
	if (rc == Rect(0, 0, 0, 0))
		return RECT();

	// get ball positions
	Mat ballMask;
	Mat _img, cropImg = img(rc);
	cropImg.copyTo(_img);

	Mat _gray, _blurred, _auto_canny;
	cvtColor(_img, _gray, COLOR_BGR2GRAY);
	bilateralFilter(_gray, _blurred, 5, 15, 15);
	// auto canny
	int med = 0;
	float sigma = 0.33f;
	for (int i = 0; i < _blurred.rows; ++i)
	{
		for (int j = 0; j < _blurred.cols; j++)
			med += _blurred.at<uchar>(i, j);
	}
	med = med / (_blurred.rows * _blurred.cols);
	int lower = int(max(0.0, (1.0 - sigma) * med));
	int upper = int(min(255.0, (1.0 + sigma) * med));
	Canny(_blurred, _auto_canny, lower, upper);
	//
	int w = _img.size().width;
	int h = _img.size().height;
	int x1_top, x1_bottom, x1_left, x2_left;
	int y1_top, y1_left, y2_top, y1_right;
	int x2_top, x2_bottom, x1_right, x2_right;
	int y1_bottom, y2_bottom, y2_left, y2_right;
	x1_top = x1_bottom = x1_left = x2_left = 0;
	y1_top = y1_left = y2_top = y1_right = 0;
	x2_top = x2_bottom = x1_right = x2_right = w;
	y1_bottom = y2_bottom = y2_left = y2_right = h;

	vector<Vec2f> lines;
	HoughLines(_auto_canny, lines, 1, 3.1415926543 / 180, 100);
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 2000 * (-b));
		pt1.y = cvRound(y0 + 2000 * (a));
		pt2.x = cvRound(x0 - 2000 * (-b));
		pt2.y = cvRound(y0 - 2000 * (a));

		float _a = (float)((int)(abs(a) * 100 + .5) / 100);
		float _b = (float)((int)(abs(b) * 100 + .5) / 100);

		//line(_img, pt1, pt2, Scalar(0, 0, 255), 1);

		if (abs(_a) == 1.0 && abs(_b) == 0.0)
		{
			// vertical line
			// find out if this vertical line is closer to left or right edge
			if (max(pt1.x, pt2.x) < (w / 2))
			{
				// closer to left edge
				if (max(pt1.x, pt2.x) > max(x1_left, x2_left))
				{
					x1_left = max(pt1.x, pt2.x);
					x2_left = max(pt1.x, pt2.x);
					y1_left = pt1.y;
					y2_left = pt2.y;
				}
			}
			else
			{
				// closer to right edge
				if (min(pt1.x, pt2.x) < min(x1_right, x2_right))
				{
					x1_right = min(pt1.x, pt2.x);
					x2_right = min(pt1.x, pt2.x);
					y1_right = pt1.y;
					y2_right = pt2.y;
				}

			}
		}
		// horizontal line
		// find out if this horizontal line is closer to top or bottom edge
		else if (abs(_a) == 0.0 && abs(_b) == 1.0)
		{
			if (max(pt1.y, pt2.y) < (h / 2))
			{
				// closer to top edge
				if (max(pt1.y, pt2.y) > max(y1_top, y2_top))
				{
					y1_top = max(pt1.y, pt2.y);
					y2_top = max(pt1.y, pt2.y);
					x1_top = pt1.x;
					x2_top = pt2.x;
				}
			}
			else {
				// closer to bottom edge
				if (min(pt1.y, pt2.y) < min(y1_bottom, y2_bottom))
				{
					y1_bottom = min(pt1.y, pt2.y);
					y2_bottom = min(pt1.y, pt2.y);
					x1_bottom = pt1.x;
					x2_bottom = pt2.x;
				}
			}
		}
	}

	x1_top += rc.y;
	y1_top += rc.y;
	x2_top += rc.y;
	y2_top += rc.y;

	x1_bottom += rc.y;
	y1_bottom += rc.y;
	x2_bottom += rc.y;
	y2_bottom += rc.y;

	x1_left += rc.x;
	y1_left += rc.x;
	x2_left += rc.x;
	y2_left += rc.x;

	x1_right += rc.x;
	y1_right += rc.x;
	x2_right += rc.x;
	y2_right += rc.x;

	x1_top = x1_left;
	x2_top = x1_right;
	x1_bottom = x1_left;
	x2_bottom = x1_right;
	y1_left = y1_top;
	y2_left = y1_bottom;
	y1_right = y1_top;
	y2_right = y1_bottom;

	line(img, Point(x1_top, y1_top), Point(x2_top, y2_top), Scalar(255, 0, 0), 1);
	line(img, Point(x1_bottom, y1_bottom), Point(x2_bottom, y2_bottom), Scalar(255, 0, 0), 1);
	line(img, Point(x1_left, y1_left), Point(x2_left, y2_left), Scalar(255, 0, 0), 1);
	line(img, Point(x1_right, y1_right), Point(x2_right, y2_right), Scalar(255, 0, 0), 1);

	//imshow("result", img);
	RECT result;
	result.left = x1_top ;
	result.top = y1_top;
	result.right = x2_right + 1;
	result.bottom = y2_right;

	double scale = (result.right - result.left) / mRuleSet->tableWidth;
	mBallList.clear();

	ballMask = balls_mask_new(rc);
	findContours(ballMask, contours, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (contourArea(contours[i]) <= 1000 && contourArea(contours[i]) > 100)
		{
			Point2f p;
			float r;
			minEnclosingCircle(contours[i], p, r);
			Rect _rc = boundingRect(contours[i]);
			Mat1b mask(_img.rows, _img.cols, uchar(0));
			rectangle(mask, _rc, Scalar(255), -1);
			//drawContours(mask, contours, i, Scalar(255), 1);
			Scalar s = mean(_img, mask);
			rectangle(_img, _rc, s, 4);
			//circle(_img, p, r, s, 3);

			// Create New ball
			Ball b; 
			unsigned char _r, _g, _b;
			_r = _g = _b = 140;
			b.position.x = (rc.x + 0.5 + p.x - result.left) / scale;
			b.position.y = (rc.y + 0.5 + p.y - result.top) / scale;

			b.org_position = b.position;
			if (s[0] >= _r && s[1] >= _g && s[2] >= _b)
			{
				_r = s[0];
				_g = s[1];
				_b = s[2];
				b.ballType = Ball::BALL_CUE;
				mBallList.push_back(b);
				currentCueBall = &mBallList.back();
			} 
			else {
				b.ballType = Ball::BALL_OTHER;
				mBallList.push_back(b);
			}


		}
	}
	//imshow("test" , _img);
	return result;
}

INT_PTR CALLBACK Control(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			DestroyWindow(mHwnd);
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDC_PRESENT) {
			DrawFrame(true);
			ShowWindow(mCtrlPanel, SW_HIDE);
			//Sleep(500);
			HWND hwndDesktop = GetDesktopWindow();
			Mat src = hwnd2mat(hwndDesktop);
			ShowWindow(mCtrlPanel, SW_SHOW);
			mTableRect = PredictTable(src);

			//imshow("output", src);
			/*mTableRect.left = 191;
			mTableRect.right = 1439;
			mTableRect.top = 223;
			mTableRect.bottom = 948;*/
		}/*
		else if (LOWORD(wParam) == IDC_TABLE) {
			mSelectedMode = MODE_TABLE;
			if (!isCaptured()) CaptureDesktop();
			::SetLayeredWindowAttributes(mHwnd, RGB(0, 0, 0), 255 , LWA_ALPHA | LWA_COLORKEY);
		}
		else if (LOWORD(wParam) == IDC_BALL) {
			if(!isCaptured()) CaptureDesktop();
			mSelectedMode = MODE_SIM;
			::SetLayeredWindowAttributes(mHwnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);
		}
		else if (LOWORD(wParam) == IDC_SIM) {
			releaseDesktop();
			mSelectedMode = MODE_REAL;
			::SetLayeredWindowAttributes(mHwnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);
		}*/
		break;
	}
	return (INT_PTR)FALSE;
}

#pragma comment( lib, "winmm.lib")
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	int n = 3;
	list<int> *args = new list<int>[10]();
	args[8].push_back(3);
	args[9].push_back(3);
	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_POWERHELPER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_POWERHELPER));

	MSG msg;


	// Main message loop:
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		DrawFrame(false);

	}

	return (int)msg.wParam;
}

typedef DWORD(WINAPI *PTHREAD_START_ROUTINE)(
	LPVOID lpThreadParameter
	);
