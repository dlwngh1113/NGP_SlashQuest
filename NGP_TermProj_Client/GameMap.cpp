#include "GameMap.h"

GameMap::GameMap()
{
	LoadLobby();
}

GameMap::GameMap(float x, float y) :Object(Packet(x, y))
{
	LoadLobby();
}

void GameMap::LoadLobby()
{
	_backGroundImg = &bgImagePool.lobby_background;
	_foreGroundImg = &bgImagePool.game_logo;
}

void GameMap::LoadIngame()
{
	_backGroundImg = &bgImagePool.floor;
	_foreGroundImg = nullptr;
}

void GameMap::LoadResult(bool isWin)
{
	_backGroundImg = &bgImagePool.gameover_background;
	if (isWin)
		_foreGroundImg = &bgImagePool.win_forelogo;
	else
		_foreGroundImg = &bgImagePool.lose_forelogo;
}

GameMap::~GameMap()
{

}

void GameMap::Update()
{

}

void GameMap::Render(HDC MemDC)
{
	if (_backGroundImg)
		_backGroundImg->StretchBlt(MemDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (_foreGroundImg)
		_foreGroundImg->Draw(MemDC, SCREEN_WIDTH / 2 - _foreGroundImg->GetWidth() / 2, SCREEN_HEIGHT / 2 - _foreGroundImg->GetHeight() / 2,
			_foreGroundImg->GetWidth(), _foreGroundImg->GetHeight());
}

void GameMap::Render(HDC MemDC, float xOffset, float yOffset)
{
	if (_backGroundImg)
		_backGroundImg->StretchBlt(MemDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
			static_cast<int>(xOffset), static_cast<int>(yOffset),
			static_cast<int>(SCREEN_WIDTH),static_cast<int>(SCREEN_HEIGHT));
	if (_foreGroundImg)
		_foreGroundImg->Draw(MemDC, static_cast<int>(SCREEN_WIDTH / 2 - _foreGroundImg->GetWidth() / 2 + xOffset), 
			static_cast<int>(SCREEN_HEIGHT / 2 - _foreGroundImg->GetHeight() / 2 + yOffset));
}