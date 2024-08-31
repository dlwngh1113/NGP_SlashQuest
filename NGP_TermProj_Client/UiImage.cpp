#include "UiImage.h"

UiImage::UiImage(float x, float y, float width, float height, const char* imgPath):UserInterface(x, y),
_width{width}, _height{height}
{
	_image.Load(imgPath);
	if (_image.IsNull()) {
		printf("%s doesnt exist(UiImage)", imgPath);
		delete this;
	}
}

UiImage::~UiImage()
{
	if(!_image.IsNull())
		_image.Destroy();
}

void UiImage::Update(float x, float y)
{
}

void UiImage::Render(HDC MemDC, float x, float y)
{
	_image.Draw(MemDC, x, y, _width, _height);
}