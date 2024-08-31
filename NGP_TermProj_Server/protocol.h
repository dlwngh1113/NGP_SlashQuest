#pragma once
#define _USE_MATH_DEFINES
#include <Windows.h>
#include<chrono>
#include <math.h>

constexpr auto MAX_ID_LEN = 10;
constexpr auto PORT = 9000;
constexpr auto BUF_SIZE = 512;

constexpr auto SCREEN_WIDTH = 1280;
constexpr auto SCREEN_HEIGHT = 720;

constexpr auto PLAYER_IMAGE_WIDTH = 75;
constexpr auto PLAYER_IMAGE_HEIGHT = 84;

constexpr auto GAME_FPS = 30;

enum class Protocol {
	//LOGIN
	LOGIN_REQUEST = 100,
	LOGIN_FAIL,
	LOGIN_SUCCESS,

	//LOBBY
	MATCH_REQUEST = 200,
	MATCH_FAIL,
	MATCH_SUCCESS,

	//INGAME
	ENTER_INGAME_REQUEST = 300,
	GAME_START,

	KEYDOWN_A_REQUEST = 350,
	KEYDOWN_D_REQUEST,
	KEYDOWN_SUCCESS,
	MOVE_REQUEST,
	MOVE_SUCCESS,

	PROCESS_POINTS_REQUSET,

	ITEM_CREATED,
	ITEM_ACTIVATED,
	ITEM_BLIND_ACTIVATED,
	ITEM_BLIND_FINISHED,

	//SYSTEM
	ENEMY_DISCONNECTED = 500,
	SYSTEM_WIN,
	SYSTEM_LOSE,
};

struct Sword {
	short length;
	float rotationDegree;
};

struct Packet {
	float x, y;
	Packet() = default;
	Packet(float x, float y) :x{ x }, y{ y } {	}
};

struct PlayerPacket : public Packet{
	Sword sword;
	short hp;
	bool isGodMode;
	char id[MAX_ID_LEN] = { NULL };
	std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();
};

constexpr WORD ITEM_SWORD_LENGTH = 0;
constexpr WORD ITEM_ENEMY_BLIND = 1;
constexpr WORD ITEM_HEAL		= 2;

struct ItemPacket : public Packet{
	WORD type;
};

inline double DegToRad(double deg) {
	return deg * M_PI / 180.0;
}

inline double RadToDeg(double rad) {
	return rad * (180.0 / M_PI);
}