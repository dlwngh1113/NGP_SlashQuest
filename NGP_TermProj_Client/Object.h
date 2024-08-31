#pragma once
#include"framework.h"

class Object {
public:
	Object();
	Object(Packet packet);
	virtual ~Object();
	virtual void Update();
	virtual void Render(HDC hDC);
	virtual void Render(HDC hDC, float xOffset, float yOffset);
	virtual void Move(float velX, float velY);
protected:
	Packet _pointPacket;
};