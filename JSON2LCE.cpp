// JSON2LCE.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "JSON2LCE.h"
#include <nlohmann/json.hpp>
#include <commdlg.h>
#include "Converter.h"

#define MAX_LOADSTRING 100
#define WIDTH 512
#define HEIGHT 512

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HBRUSH MayBrush = CreateSolidBrush(RGB(0xA0, 0x4D, 0xFF));
HBRUSH StarBrush = CreateSolidBrush(RGB(0xc7, 0x9e, 0xf0));
HPEN StarPen = CreatePen(PS_SOLID, 4, RGB(0xc7, 0x9e, 0xf0));

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR    lpCmdLine,
					 _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_JSON2LCE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JSON2LCE));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JSON2LCE));
	wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = MayBrush;
	wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_JSON2LCE);
	wcex.lpszClassName  = szWindowClass;
	wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_JSON2LCE));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
	  CW_USEDEFAULT, 0, WIDTH, HEIGHT, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
	  return FALSE;
   }

#ifndef NDEBUG
   AllocConsole();

   FILE* fpOut;
   FILE* fpErr;

   freopen_s(&fpOut, "CONOUT$", "w", stdout);
   freopen_s(&fpErr, "CONOUT$", "w", stderr);
#endif

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

bool OpenJSONFileDialog(HWND hWnd, std::wstring& path)
{
	OPENFILENAME ofn;       // common dialog box structure
	WCHAR szFile[MAX_PATH] = {}; // buffer for file name

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
	ofn.lpstrFilter = L"JSON Files\0*.json\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		path = ofn.lpstrFile;

		return true;
	}
	else return false;
}

bool SaveFileDialog(HWND hWnd, std::wstring& path, const std::wstring& filter, const std::wstring& ext)
{
	OPENFILENAME ofn;       // common dialog box structure
	WCHAR szFile[MAX_PATH] = {}; // buffer for file name

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
	std::vector<wchar_t> filterBuffer(filter.begin(), filter.end());
	filterBuffer.push_back(L'\0');
	ofn.lpstrFilter = filterBuffer.data(); // this is such a stupid solution lol
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ext.c_str();

	if (GetSaveFileName(&ofn) == TRUE)
	{
		path = ofn.lpstrFile;

		return true;
	}
	else return false;
}

const float STAR_SIZE = 50;
const float STAR_SPIN_SPEED = 0.0125;
const float STAR_SPEED = 1.5f;
const float X_SPACE = STAR_SIZE * 2.5f;
const float Y_SPACE = STAR_SIZE * 2.5f;

struct Star {
	float baseX, x, y;
	double angle;
};

std::vector<Star> stars;

POINT starPoints[10]; // 10 points to draw a star
const double PI = 3.141592653589793;
void CreateStarPoints(float centerX, float centerY, int radius, double rotationAngle) {

	for (int i = 0; i < 10; ++i) {
		double angle = i * PI / 5 + -PI / 2 + rotationAngle;
		int r = (i % 2 == 0) ? radius : radius / 2;
		starPoints[i].x = static_cast<LONG>(centerX + cos(angle) * r);
		starPoints[i].y = static_cast<LONG>(centerY + sin(angle) * r);
	}
}

int cols = (int)(WIDTH / X_SPACE);
int rows = (int)(HEIGHT / Y_SPACE);

void InitStars() {
	stars.clear();

	float totalStarsWidth = X_SPACE * (cols - 1);
	float startXOffset = (WIDTH - totalStarsWidth) / 2;

	const float secondRowOffset = STAR_SIZE;
	const int loopRows = rows + 2;   // +2 to allow off-screen above
	const int loopCols = cols + 2;   // +2 to allow off-screen to right

	for (int row = -1; row < loopRows - 1; ++row) {
		for (int col = -1; col < loopCols - 1; ++col) {
			float xOffset = ((row + 1) % 2 == 0) ? secondRowOffset : 0;
			float baseX = startXOffset + col * X_SPACE + xOffset;
			float y = Y_SPACE * (row + 1) - STAR_SIZE;
			stars.push_back({ baseX, baseX, y, 0.0 });
		}
	}
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		{
			// ensure decimals with commas cannot happen
			setlocale(LC_NUMERIC, "C");

			SetTimer(hWnd, 1, 8, NULL);

			InitStars();

			HWND hwndButton = CreateWindow(
				L"BUTTON",  // Predefined class; Unicode assumed 
				L".JSON -> .CSM",      // Button text
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles 
				(WIDTH - 200) / 2,         // x position 
				40,         // y position 
				200,        // Button width
				100,        // Button height
				hWnd,     // Parent window
				(HMENU)5, // I chose 5 because I want it to be 5, fuck you
				hInst,
				NULL);      // Pointer not needed.
		}
		break;
	case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);

			std::wstring jsonPath;
			std::wstring outPath;

			// Parse the menu selections:
			switch (wmId)
			{
			case 5: // because
				if (!OpenJSONFileDialog(hWnd, jsonPath) ||
					!SaveFileDialog(hWnd, outPath, L"CSM Files\0*.csm\0All Files\0*.*\0", L"csm"))
				{
					MessageBox(hWnd, L"Operation aborted", L"Aborted", MB_ICONERROR);
					break;
				}
				std::wcout << jsonPath << " " << outPath << std::endl;
				JSON2CSM(jsonPath, outPath);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
	case WM_TIMER:
		{
			for (int i = 0; i < stars.size(); ++i) {
				stars[i].x -= STAR_SPEED;
				stars[i].y += STAR_SPEED;

				// Wrap horizontally
				if (stars[i].x < -STAR_SIZE)
					stars[i].x += (cols + 2) * X_SPACE;

				// Wrap vertically
				if (stars[i].y > HEIGHT + STAR_SIZE)
					stars[i].y -= (rows + 2) * Y_SPACE;

				stars[i].angle += STAR_SPIN_SPEED;
				if (stars[i].angle > 2 * PI)
					stars[i].angle -= 2 * PI;
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
	break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			HDC memDC = CreateCompatibleDC(hdc);
			RECT rc;
			GetClientRect(hWnd, &rc);
			HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
			HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

			FillRect(memDC, &rc, MayBrush);
			HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, StarBrush);

			SelectObject(memDC, StarPen);

			for (int i = 0; i < stars.size(); ++i) {
				CreateStarPoints(stars[i].x, stars[i].y, STAR_SIZE, stars[i].angle);
				Polygon(memDC, starPoints, 10);
			}

			BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, memDC, 0, 0, SRCCOPY);

			SelectObject(memDC, oldBitmap);
			DeleteObject(memBitmap);
			DeleteDC(memDC);

			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
