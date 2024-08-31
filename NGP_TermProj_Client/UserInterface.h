#pragma once
#include"framework.h"
class UserInterface
{
protected:
	float _x, _y;
public:
	UserInterface(float x, float y) :_x{ x }, _y{ y }{}
	virtual ~UserInterface() {};
};