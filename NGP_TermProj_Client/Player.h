#pragma once
#include"UiImage.h"
#include"Item.h"

#define PLAYER_SPEED 5
#define POLY 5
#define MAX_LIFE 5
#define SWORD_POLYGON 13
#define MAX_INVINCIBLE_TIME 200

class PlayerResource {
public:
	CImage g_imgPlayerLeft;
	CImage g_imgPlayerRight;
	CImage g_imgPlayerLeftHit;
	CImage g_imgPlayerRightHit;

	CImage g_imgHeartFull;
	CImage g_imgHeartBlank;
	CImage g_imgError;
	CImage g_imgPlayerDeadLeft;
	CImage g_imgPlayerDeadRight;
};

enum PlayerDirection {
	PLAYER_DIRECTION_FIXED,
	PLAYER_DIRECTION_RIGHT,
	PLAYER_DIRECTION_LEFT,
	PLAYER_DIRECTION_UP,
	PLAYER_DIRECTION_DOWN,
};

enum class PlayerStatus {
	PLAYER_ACTIVE_LEFT,
	PLAYER_ACTIVE_RIGHT,
	PLAYER_FALL_START,
	PLAYER_FALLING
};

struct PlayerSwordPolygon{
	POINT swordPolygon[SWORD_POLYGON];
	void InitPlayerSwordData(int px, int py, int length) {
		const int SWORD_HEIGHT = 20;
		const int SWORD_HEIGHT_HALF = 10; // 충돌체크 원의 반지름과 동일
		swordPolygon[0].x = + 40;
		swordPolygon[0].y = - SWORD_HEIGHT_HALF;

		swordPolygon[1].x = + 40;
		swordPolygon[1].y = + SWORD_HEIGHT_HALF;

		swordPolygon[2].x = swordPolygon[1].x + 10;
		swordPolygon[2].y = swordPolygon[1].y;

		swordPolygon[3].x = swordPolygon[2].x;
		swordPolygon[3].y = swordPolygon[2].y + 10;

		swordPolygon[4].x = swordPolygon[3].x + 10;
		swordPolygon[4].y = swordPolygon[3].y;

		swordPolygon[5].x = swordPolygon[4].x;
		swordPolygon[5].y = swordPolygon[2].y;
		// 	swordPolygon[6].x = + (268 / 2) - 10 + length;
		swordPolygon[6].x = + (200 / 2) - SWORD_HEIGHT_HALF + 2 * length;
		swordPolygon[6].y = + SWORD_HEIGHT_HALF;

		swordPolygon[7].x = + (200 / 2) + 2 * length;
		swordPolygon[7].y = 0;

		swordPolygon[8].x = + (200 / 2) - SWORD_HEIGHT_HALF + 2 * length;
		swordPolygon[8].y = - SWORD_HEIGHT_HALF;

		swordPolygon[9].x = swordPolygon[0].x + 20;
		swordPolygon[9].y = swordPolygon[0].y;

		swordPolygon[10].x = swordPolygon[9].x;
		swordPolygon[10].y = swordPolygon[9].y - 10;

		swordPolygon[11].x = swordPolygon[10].x - 10;
		swordPolygon[11].y = swordPolygon[10].y;

		swordPolygon[12].x = swordPolygon[0].x + 10;
		swordPolygon[12].y = swordPolygon[0].y;
	}
	void UpdatePlayerSwordData(int px, int py, short len, float deg) {
		double rad = DegToRad(deg);
		PlayerSwordPolygon originData;
		originData.InitPlayerSwordData(px, py, len);
		
		for (int i = 0; i < SWORD_POLYGON; ++i) {
			swordPolygon[i].x = (LONG)((originData.swordPolygon[i].x) * cos(rad)) -
				(LONG)((originData.swordPolygon[i].y) * sin(rad)) + px;
			swordPolygon[i].y = (LONG)((originData.swordPolygon[i].y) * cos(rad)) +
				(LONG)((originData.swordPolygon[i].x) * sin(rad)) + py;
		}
	}

	void DrawPlayerSword(HDC hDC) {
		HBRUSH hBrush, oldBrush;
		HPEN hPen, oldPen;
		hBrush = CreateSolidBrush(RGB(135, 135, 135));
		oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		hPen = CreatePen(PS_SOLID, 5, RGB(135, 135, 135));
		oldPen = (HPEN)SelectObject(hDC, hPen);

		Polygon(hDC, swordPolygon, SWORD_POLYGON);
		
		SelectObject(hDC, oldPen);
		DeleteObject(hPen);
		DeleteObject(hBrush);
	}
	void DrawPlayerSword(HDC hDC, float xOffset, float yOffset) {
		HBRUSH hBrush, oldBrush;
		HPEN hPen, oldPen;
		hBrush = CreateSolidBrush(RGB(135, 135, 135));
		oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		hPen = CreatePen(PS_SOLID, 5, RGB(135, 135, 135));
		oldPen = (HPEN)SelectObject(hDC, hPen);

		POINT tmp[SWORD_POLYGON];
		for (int i = 0; i < SWORD_POLYGON; ++i) {
			tmp[i].x = swordPolygon[i].x - xOffset;
			tmp[i].y = swordPolygon[i].y - yOffset;
		}
		Polygon(hDC, tmp, SWORD_POLYGON);

		SelectObject(hDC, oldPen);
		DeleteObject(hPen);
		DeleteObject(hBrush);
	}
};

class Player : public Object
{
public:
	Player() = default;
	Player(PlayerPacket p);

	void ChangeState(PlayerStatus stat);

#pragma region Getter and Setter
	const Sword& GetSwordData() { return _playerPacket.sword; }
	// const PlayerPacket& GetPlayerPacket() { return _playerPacket; }
	void SetPlayerPacket(PlayerPacket p) { _playerPacket = p; }

	void SetCheckBlinding(bool flag) { _isBlinding = flag; }
#pragma endregion Getter and Setter

	virtual ~Player();
	virtual void Update();
	virtual void Render(HDC hDC);
	virtual void Render(HDC MemDC, float xOffset, float yOffset);
	virtual void Move(float velX, float velY);
private:
	CImage _currImage;
	PlayerStatus _currStat;
	UiImage** _playerUI;

	UiImage* _blindImage;
	bool _isBlinding;

	PlayerPacket _playerPacket;
	PlayerSwordPolygon _swordPolygons;
};