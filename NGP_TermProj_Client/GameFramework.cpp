#include "GameFramework.h"

GameFramework::GameFramework(HWND hWnd):hWnd{hWnd}
{
	_scene = new Scene(hWnd);
}

GameFramework::~GameFramework()
{
	if (_scene)
		delete _scene;
}

void GameFramework::Update()
{
	_scene->Update();
}

void GameFramework::Render(HDC hDC) const
{
	HDC MemDC = CreateCompatibleDC(hDC);
	HBITMAP hBit = CreateCompatibleBitmap(hDC, SCREEN_WIDTH, SCREEN_HEIGHT);
	HBITMAP oldBit = (HBITMAP)SelectObject(MemDC, hBit);

	PatBlt(MemDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITENESS);

	//Add Rendering Code
	_scene->Render(MemDC);
	//Render Code End

	BitBlt(hDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MemDC, 0, 0, SRCCOPY);
	
	SelectObject(MemDC, oldBit);
	DeleteObject(hBit);
	DeleteDC(MemDC);
}

LRESULT GameFramework::KeyInputProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return _scene->KeyInputProcess(hWnd, message, wParam, lParam);
}

LRESULT GameFramework::MouseInputProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return _scene->MouseInputProcess(hWnd, message, wParam, lParam);
}