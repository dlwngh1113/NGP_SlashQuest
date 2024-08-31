#pragma once
#include"GameMap.h"

class Camera
{
	bool _isScroll;
	float _xOffset;
	float _yOffset;
public:
	Camera() = default;
	Camera(float x, float y);
	virtual ~Camera();
	void Update(float x, float y);
	void Render(HDC MemDC, const std::vector<Object*>& objects);
	void ScrollOn();
	void ScrollOff();
};