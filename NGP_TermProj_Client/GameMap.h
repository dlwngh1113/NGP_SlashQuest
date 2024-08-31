#pragma once
#include"Object.h"

struct BGImagePool {
	// lobby img
	CImage lobby_background;
	CImage game_logo;
	//

	// ingame img
	CImage floor;
	//

	//result img
	CImage gameover_background;
	CImage win_forelogo;
	CImage lose_forelogo;
	//

	BGImagePool() {
		lobby_background.Load(TEXT("SlashQuestResource/Images/BG_decal_01.png"));
		game_logo.Load(TEXT("SlashQuestResource/Images/Logo.png"));

		floor.Load(TEXT("SlashQuestResource/Images/Floor_2048sz.png"));

		gameover_background.Load(TEXT("SlashQuestResource/Images/gameover_background.png"));
		win_forelogo.Load(TEXT("SlashQuestResource/Images/win_forelogo.png"));
		lose_forelogo.Load(TEXT("SlashQuestResource/Images/lose_forelogo.png"));
	}
	~BGImagePool() {
		lobby_background.Destroy();
		game_logo.Destroy();

		floor.Destroy();

		gameover_background.Destroy();
		win_forelogo.Destroy();
		lose_forelogo.Destroy();
	}
};

class GameMap : public Object
{
	BGImagePool bgImagePool;
	CImage* _backGroundImg;
	CImage* _foreGroundImg;
public:
	GameMap();
	GameMap(float x, float y);
	void LoadLobby();
	void LoadIngame();
	void LoadResult(bool isWin);
	virtual ~GameMap();
	virtual void Update();
	virtual void Render(HDC hDC);
	virtual void Render(HDC MemDC, float xOffset, float yOffset);
};