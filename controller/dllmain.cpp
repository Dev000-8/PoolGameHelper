// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#pragma data_seg(".KOKTECH")
HWND h_Wnd = NULL;
HHOOK keyhook = 0;
HHOOK mousehook = 0;
HINSTANCE hInst;
#pragma data_seg()
#pragma comment(linker, "/section:.KOKTECH,rws")

#define GWM_LBUTTONDOWN (WM_USER + 0x2000)
#define GWM_LBUTTONUP (WM_USER + 0x2001)
#define GWM_MOUSEMOVE (WM_USER + 0x2002)
#define GWM_RBUTTONDOWN (WM_USER + 0x2003)
#define GWM_KEYDOWN (WM_USER + 0x2004)
#define GWM_KEYUP (WM_USER + 0x2005)

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

		hInst = hModule;
		return TRUE;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C"  __declspec(dllexport) BOOL uninstallhook(HWND hWnd)
{
	if (hWnd != hWnd || hWnd == NULL)
		return FALSE;
	BOOL unhooked;
	if(keyhook)
		unhooked = UnhookWindowsHookEx(keyhook);
	if(unhooked)
	unhooked = UnhookWindowsHookEx(mousehook);
	return unhooked;
}

LRESULT __declspec(dllexport) CALLBACK KeyboardProc(UINT nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
	{ /* pass it on */
		CallNextHookEx(keyhook, nCode, wParam, lParam);
		return 0;
	} /* pass it on */
	LPMSG msg = (LPMSG)lParam;

	if (lParam & 0x80000000) // check bit 31 for up/down
	{	
		PostMessage(h_Wnd, GWM_KEYUP, wParam, lParam);
	}
	else {
		PostMessage(h_Wnd, GWM_KEYDOWN, wParam, lParam);
	}
	
	return CallNextHookEx(keyhook, nCode, wParam, lParam);
}

LRESULT __declspec(dllexport) CALLBACK MouseProc(UINT nCode, WPARAM wParam, LPARAM lParam)
{
	MSLLHOOKSTRUCT * pMouseStruct = (MSLLHOOKSTRUCT *)lParam;
	if (nCode == 0)
	{
		if (pMouseStruct != NULL)
		{
			LONG_PTR _lParam = 0;
			_lParam = (pMouseStruct->pt.x & 0xFFFF) | ((pMouseStruct->pt.y << 16) & 0xFFFF0000);

			switch (wParam)
			{
			case WM_LBUTTONUP:
			{
				PostMessage(h_Wnd, GWM_LBUTTONUP, wParam, _lParam);
			}
			break;
			case WM_MOUSEMOVE:
			{
				PostMessage(h_Wnd, GWM_MOUSEMOVE, wParam, _lParam);
			}
			break;
			case WM_LBUTTONDOWN:
			{
				PostMessage(h_Wnd, GWM_LBUTTONDOWN, wParam, _lParam);
			}
			break;
			case WM_RBUTTONDOWN:
			{
				PostMessage(h_Wnd, GWM_RBUTTONDOWN, wParam, _lParam);
			}
			break;
			default:
				break;
			}
		}
	} 

	return CallNextHookEx(mousehook, nCode, wParam, lParam); 
}

extern "C" __declspec(dllexport) BOOL installhook(HWND hWnd)
{
	if (h_Wnd != NULL)
		return FALSE; 

	mousehook = SetWindowsHookEx(WH_MOUSE_LL,
		(HOOKPROC)MouseProc,
		hInst,
		0);

	keyhook = SetWindowsHookEx(WH_KEYBOARD,
		(HOOKPROC)KeyboardProc,
		hInst,
		0);  
	
	if (mousehook != NULL)
	{ /* success */
		h_Wnd = hWnd;
		return TRUE;
	} /* success */
	return FALSE; // failed to set hook
}