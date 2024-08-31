#pragma once
#include"UserInterface.h"
class UiImage : public UserInterface
{
	CImage _image;
	float _width, _height;
public:
	UiImage(float x, float y, float width, float height, const char* imgPath);
	virtual ~UiImage();
	void Update(float x, float y);
	void Render(HDC MemDC, float x, float y);
};