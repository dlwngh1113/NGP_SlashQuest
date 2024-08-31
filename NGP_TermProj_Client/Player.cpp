#include "Player.h"

Player::Player(PlayerPacket p) {
	_playerPacket = p;

	_currImage.Load(TEXT("SlashQuestResource/Images/Player_Left.png"));
	_currStat = PlayerStatus::PLAYER_ACTIVE_RIGHT;
	_playerUI = new UiImage * [MAX_LIFE];
	for (int i = 0; i < MAX_LIFE; ++i)
		_playerUI[i] = new UiImage(p.x - 10 * MAX_LIFE + i * 20, p.y, 20, 20, "SlashQuestResource/Images/Heart_Full.png");
	
	_isBlinding = false;
	_blindImage = new UiImage(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 14 * 80, 10 * 80, "SlashQuestResource/Images/Ink.png");

	_swordPolygons.InitPlayerSwordData((int)_playerPacket.x, (int)_playerPacket.y, (int)_playerPacket.sword.length);
}

Player::~Player()
{
	for (int i = 0; i < MAX_LIFE; ++i)
		if (_playerUI[i])
			delete _playerUI[i];
	delete[] _playerUI;

	if (_blindImage)
		delete _blindImage;

	_currImage.Destroy();
}

void Player::ChangeState(PlayerStatus stat)
{

}

void Player::Update()
{
	_swordPolygons.UpdatePlayerSwordData((int)_playerPacket.x, (int)_playerPacket.y, _playerPacket.sword.length, _playerPacket.sword.rotationDegree);
	_currStat = ( 90.0f <= _playerPacket.sword.rotationDegree && _playerPacket.sword.rotationDegree <= 270.0f) ?
		(PlayerStatus::PLAYER_ACTIVE_LEFT) : (PlayerStatus::PLAYER_ACTIVE_RIGHT);

	if (_currStat == PlayerStatus::PLAYER_ACTIVE_LEFT) {
		//_currImage.Destroy();
		//_currImage.Load(TEXT("SlashQuestResource/Images/Player_Left.png"));
	}
	else if (_currStat == PlayerStatus::PLAYER_ACTIVE_RIGHT) {
		//_currImage.Destroy();
		//_currImage.Load(TEXT("SlashQuestResource/Images/Player_Right.png"));
	}
}

void Player::Move(float velX, float velY)
{
	Object::Move(velX, velY);
}

void Player::Render(HDC hDC) 
{
	int w = _currImage.GetWidth();
	int h = _currImage.GetHeight();
	
	// Collision Rect
	//Rectangle(hDC, (int)_x - w/2, (int)_y - h/2, (int)_x + w/2, (int)_y + h/2);
	
	// std::cout << "w, h: " << w << " " << h << std::endl;
	// std::cout << "w, h: " << _playerPacket.x << " " << _playerPacket.y << std::endl;
	
	_currImage.SetTransparentColor(RGB(0, 255, 0));
	_currImage.Draw(hDC, (int)(_playerPacket.x - w/2), (int)(_playerPacket.y - h/2), w, h);
	
	_swordPolygons.DrawPlayerSword(hDC);

	//for (int i = 0; i < 3+ (_playerPacket.sword.length / 10); ++i) {
	//	double playerSwordRad = DegToRad(_playerPacket.sword.rotationDegree);
	//	int x = 100 + 2 * _playerPacket.sword.length - 10 - 2 * 10 * i;
	//	int y = 0; 

	//	double rotatedX = (x * cos(playerSwordRad) - y * sin(playerSwordRad) + _playerPacket.x);
	//	double rotatedY = (y * cos(playerSwordRad) + x * sin(playerSwordRad) + _playerPacket.y);

	//	Packet data = { (float)rotatedX, (float)rotatedY };
	//	Ellipse(hDC, rotatedX - 10, rotatedY - 10, rotatedX + 10, rotatedY + 10);
	//}

	// pivot
	//Rectangle(hDC, (int)_x - 1, (int)_y - 1, (int)_x + 1, (int)_y + 1);
}

void Player::Render(HDC MemDC, float xOffset, float yOffset)
{
	int w = _currImage.GetWidth();
	int h = _currImage.GetHeight();

	_currImage.SetTransparentColor(RGB(0, 255, 0));
	_currImage.Draw(MemDC, static_cast<int>(_playerPacket.x - w / 2 - xOffset), 
		static_cast<int>(_playerPacket.y - h / 2 - yOffset), w, h);

	_swordPolygons.DrawPlayerSword(MemDC, xOffset, yOffset);

	for (int i = 0; i < _playerPacket.hp; ++i)
		_playerUI[i]->Render(MemDC, _playerPacket.x - xOffset + (i - 1) * w / 2, 
			_playerPacket.y - yOffset - h / 2);

	TextOut(MemDC, _playerPacket.x - xOffset, _playerPacket.y - yOffset + h / 2,
		_playerPacket.id, strlen(_playerPacket.id));

	if (_isBlinding)
		_blindImage->Render(MemDC, SCREEN_WIDTH / 2 - 14 * 40, SCREEN_HEIGHT / 2 - 10 * 40);
}