#include "Object.h"

Object::Object()
{
	ZeroMemory(&_pointPacket, sizeof(Packet));
}

Object::Object(Packet packet) :_pointPacket(packet)
{
}

Object::~Object()
{
}

void Object::Update()
{
}

void Object::Render(HDC hDC)
{
	//Rectangle(hDC, _pointPacket.x - 10, _pointPacket.y - 10, _pointPacket.x + 10, _pointPacket.y + 10);
}

void Object::Render(HDC hDC, float xOffset, float yOffset)
{

}

void Object::Move(float velX, float velY)
{
	//_x += velX;
	//_y += velY;
}

