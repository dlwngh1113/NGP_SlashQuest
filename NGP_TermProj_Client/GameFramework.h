#pragma once
#include"Scene.h"

class GameFramework
{
	HWND hWnd;
	Scene* _scene;
public:
	GameFramework(HWND hWnd);
	virtual ~GameFramework();
	void Update();
	void Render(HDC hDC) const;
	LRESULT KeyInputProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT MouseInputProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};