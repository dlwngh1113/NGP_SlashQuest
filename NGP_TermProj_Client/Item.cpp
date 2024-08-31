#include "Item.h"

Item::Item(ItemPacket ip):Object{ Packet(ip.x, ip.y) }
{
	switch (ip.type)
	{
	case ITEM_SWORD_LENGTH:
		_image.Load(TEXT("SlashQuestResource/Images/Item_Atk.png"));
		break;
	case ITEM_ENEMY_BLIND:
		_image.Load(TEXT("SlashQuestResource/Images/Item_Gas.png"));
		break;
	case ITEM_HEAL:
		_image.Load(TEXT("SlashQuestResource/Images/Item_Heal.png"));
		break;
	}
}

bool Item::operator==(const Item& other) const
{
	//std::cout << "����x: " << _pointPacket.x << " | other x: " << other._pointPacket.x << std::endl;
	//std::cout << "����y: " << _pointPacket.y << " | other y: " << other._pointPacket.y << std::endl;
	//std::cout << "����type: " << _itemType << " | other type: " << other._itemType << std::endl;
	if (_pointPacket.x != other._pointPacket.x)
		return false;
	if (_pointPacket.y != other._pointPacket.y)
		return false;
	if (_itemType != other._itemType)
		return false;
	return true;
}

Item::~Item()
{
	//std::cout << "������x: " << _pointPacket.x << " | ������y: " << _pointPacket.y << " | ������type: " << _itemType << std::endl;
	//std::cout << "�������� null: " << (_image.IsNull()) << std::endl;
	if (!_image.IsNull()) {
		_image.Destroy();
	}

	//std::cout << "~������: " << _itemType << "\n";
	//if (!_image.IsNull()) {
	//	printf("�������� null�� �ƴϴ�\n");
	//	_image.ReleaseDC();
	//	_image.Destroy();
	//}
}

void Item::Update()
{
}

void Item::Render(HDC MemDC)
{
	_image.Draw(MemDC, (int)(Object::_pointPacket.x - 32), (int)(Object::_pointPacket.y - 32));
}

void Item::Render(HDC MemDC, float xOffset, float yOffset)
{
	_image.Draw(MemDC, static_cast<int>(Object::_pointPacket.x - 32 - xOffset) , static_cast<int>(Object::_pointPacket.y - 32 - yOffset));
}

void Item::Move(float velX, float velY)
{
	Object::Move(velX, velY);
}
