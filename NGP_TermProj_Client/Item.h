#pragma once
#include"Object.h"
class Item : public Object {
	CImage _image;
	WORD _itemType;
public:
	Item() = default;
	Item(ItemPacket ip);

	bool operator==(const Item& other) const;

	virtual ~Item();
	virtual void Update();
	virtual void Render(HDC MemDC);
	virtual void Render(HDC MemDC, float xOffset, float yOffset);
	virtual void Move(float velX, float velY);
};