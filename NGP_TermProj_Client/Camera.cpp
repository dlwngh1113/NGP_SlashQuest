#include "Camera.h"

Camera::Camera(float x, float y):_xOffset{x}, _yOffset{y}, _isScroll{false}
{
}

Camera::~Camera()
{
}

void Camera::Update(float x, float y)
{
	_xOffset = x;
	_yOffset = y;
}

void Camera::Render(HDC MemDC, const std::vector<Object*>& objects)
{
	if (_isScroll)
	{
		for (auto& obj : objects) {
			if(obj)
				obj->Render(MemDC, _xOffset, _yOffset);
		}
	}
	else {
		for (auto& obj : objects) {
			if (obj)
				obj->Render(MemDC);
		}
	}
}

void Camera::ScrollOff()
{
	_isScroll = false;
}

void Camera::ScrollOn()
{
	_isScroll = true;
}