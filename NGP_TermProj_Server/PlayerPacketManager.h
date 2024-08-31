#pragma once
#include <cmath>
#include <iostream>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <ctime>
#include<fstream>

std::ofstream out("gameLog.txt", std::ios_base::binary);
char* fileBuf[256];

#include "protocol.h"

enum class MatchingState {
	LOBBY,
	MATCHING,
	GAMING,
};

struct GameClientInfo {
	SOCKET socket = NULL;
	std::string name;

	PlayerPacket player;
	Protocol protocol = Protocol::LOGIN_REQUEST;
	MatchingState matchState = MatchingState::LOBBY;
};

struct BlindPlayer {
	GameClientInfo* playerData;
	std::chrono::steady_clock::time_point startBlindTime;
};

extern std::vector<BlindPlayer> g_blindedPlayer;

struct PlayerPacketManager {

	static void InitPlayerData(PlayerPacket* p) {
		p->hp = 5;

		p->sword.length = 50;
		p->sword.rotationDegree = 0.0f;

		p->x = 120.0f;
		p->y = 120.0f;

		p->isGodMode = false;
	}

	static void RotateSword(Sword* packet, char key) {
		int speed = 10;
		if (key == 'A') {
			packet->rotationDegree = (packet->rotationDegree - speed > 0) ?
				(packet->rotationDegree - speed) : (packet->rotationDegree - speed + 360);
		}
		else {
			packet->rotationDegree = (packet->rotationDegree + speed < 360) ?
				(packet->rotationDegree + speed) : (packet->rotationDegree + speed - 360);
		}
	}

	static void MovePlayer(PlayerPacket* packet) {
		int speed = 15;
		double rad = DegToRad((double)packet->sword.rotationDegree);
		packet->x = (float)(packet->x + ((speed)*cos(rad)));
		packet->y = (float)(packet->y + ((speed)*sin(rad)));
	}
};


void CheckCollision_SwordAndPlayer(PlayerPacket* playerPacket, PlayerPacket* enemyPacket) {
	const int SWORD_HEIGHT_HALF = 10; 

	int pointsLength = 3 + (playerPacket->sword.length / SWORD_HEIGHT_HALF);
	int swordLength = playerPacket->sword.length;
	double swordRad = DegToRad(playerPacket->sword.rotationDegree);

	Packet playerPoint = { playerPacket->x, playerPacket->y };
	Packet enemyPoint = { enemyPacket->x, enemyPacket->y };

	// 검 끝부분 원부터 삽입
	std::vector<Packet> swordCollisionMidPoints;
	if (pointsLength > 0) {
		swordCollisionMidPoints.reserve(pointsLength);
		for (int i = 0; i < pointsLength; ++i) {
			int x = 100 + 2 * SWORD_HEIGHT_HALF - SWORD_HEIGHT_HALF - 2 * SWORD_HEIGHT_HALF * i;
			int y = 0;

			double rotatedX = (x * cos(swordRad) - y * sin(swordRad) + playerPoint.x);
			double rotatedY = (y * cos(swordRad) + x * sin(swordRad) + playerPoint.y);

			Packet data = { (float)rotatedX, (float)rotatedY };
			swordCollisionMidPoints.emplace_back(data);
		}
	}

	// left top right bottom
	RECT enemyCollisionBox = {
		(LONG)enemyPoint.x - PLAYER_IMAGE_WIDTH / 2 - SWORD_HEIGHT_HALF,
		(LONG)enemyPoint.y - PLAYER_IMAGE_HEIGHT / 2 - SWORD_HEIGHT_HALF,
		(LONG)enemyPoint.x + PLAYER_IMAGE_WIDTH / 2 + SWORD_HEIGHT_HALF,
		(LONG)enemyPoint.y + PLAYER_IMAGE_HEIGHT / 2 + SWORD_HEIGHT_HALF
	};

	for (Packet& d : swordCollisionMidPoints) {
		POINT p = { (LONG)d.x, (LONG)d.y };

		if (PtInRect(&enemyCollisionBox, p)) {
			if (!enemyPacket->isGodMode) {
				--enemyPacket->hp;
				enemyPacket->isGodMode = true;
				enemyPacket->time = std::chrono::steady_clock::now();
				out << "[Collision Called]player id - " << enemyPacket->id << ", player hp - " << enemyPacket->hp << std::endl;
			}
			else {
				auto curr_time = std::chrono::steady_clock::now();
				if (std::chrono::duration_cast<std::chrono::seconds>(curr_time - enemyPacket->time).count() > 2)
					enemyPacket->isGodMode = false;
			}
			return;
		}
	}
}


void CheckCollision_PlayerAndItem(GameClientInfo* redPlayerInfo, GameClientInfo* bluePlayerInfo, std::vector<ItemPacket>* itemList) {
	constexpr int ITEM_IMAGE_RADIUS_HALF = 16;
	constexpr int MAX_HEALTH = 5;
	constexpr short SWORD_MAX_LENGTH = 50;
	constexpr short SWORD_MIN_LENGTH = 10;
	constexpr short SWORD_LENGTH_VALUE = 5; 

	Packet redPlayerPoint = { redPlayerInfo->player.x, redPlayerInfo->player.y };
	Packet bluePlayerPoint = { bluePlayerInfo->player.x, bluePlayerInfo->player.y };

	RECT redPlayerCollisionBox = {
		(LONG)redPlayerPoint.x - PLAYER_IMAGE_WIDTH / 2 - ITEM_IMAGE_RADIUS_HALF,
		(LONG)redPlayerPoint.y - PLAYER_IMAGE_HEIGHT / 2 - ITEM_IMAGE_RADIUS_HALF,
		(LONG)redPlayerPoint.x + PLAYER_IMAGE_WIDTH / 2 + ITEM_IMAGE_RADIUS_HALF,
		(LONG)redPlayerPoint.y + PLAYER_IMAGE_HEIGHT / 2 + ITEM_IMAGE_RADIUS_HALF
	};

	RECT bluePlayerCollisionBox = {
		(LONG)bluePlayerPoint.x - PLAYER_IMAGE_WIDTH / 2 - ITEM_IMAGE_RADIUS_HALF,
		(LONG)bluePlayerPoint.y - PLAYER_IMAGE_HEIGHT / 2 - ITEM_IMAGE_RADIUS_HALF,
		(LONG)bluePlayerPoint.x + PLAYER_IMAGE_WIDTH / 2 + ITEM_IMAGE_RADIUS_HALF,
		(LONG)bluePlayerPoint.y + PLAYER_IMAGE_HEIGHT / 2 + ITEM_IMAGE_RADIUS_HALF
	};

		std::vector<ItemPacket>::iterator it = find_if(itemList->begin(), itemList->end(),
		[redPlayerCollisionBox](ItemPacket element) {
			POINT p = { (LONG)element.x, (LONG)element.y };
			return (PtInRect(&redPlayerCollisionBox, p));
		});

		if (it != itemList->end()) {
			out << redPlayerInfo->player.id << "가" << it->type << "아이템(x: " << 
				it->x << ", y:" << it->y << ")과 충돌\n";

			switch (it->type)
			{
			case ITEM_SWORD_LENGTH:
				redPlayerInfo->player.sword.length = std::clamp((short)(redPlayerInfo->player.sword.length + SWORD_LENGTH_VALUE),
					SWORD_MIN_LENGTH, SWORD_MAX_LENGTH);
				break;
			case ITEM_ENEMY_BLIND: {
				bluePlayerInfo->protocol = Protocol::ITEM_BLIND_ACTIVATED;
				std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
				std::vector<BlindPlayer>::iterator blindIt = std::find_if(g_blindedPlayer.begin(), g_blindedPlayer.end(),
					[bluePlayerInfo](BlindPlayer x) {
						return strcmp(bluePlayerInfo->player.id, x.playerData->player.id) == 0;
					});
				if (blindIt != g_blindedPlayer.end()) {
					blindIt->startBlindTime = start;
				}
				else {
					g_blindedPlayer.emplace_back(BlindPlayer{
						bluePlayerInfo,
						start });
				}
				bool flag = true;
				send(bluePlayerInfo->socket, (char*)&bluePlayerInfo->protocol, sizeof(Protocol), NULL);
				send(bluePlayerInfo->socket, (char*)&flag, sizeof(bool), NULL);
			}
				break;
			case ITEM_HEAL:
				redPlayerInfo->player.hp = (redPlayerInfo->player.hp + 1 <= 5) ?
					(redPlayerInfo->player.hp + 1) : (5);
				break;
			}

			redPlayerInfo->protocol = Protocol::ITEM_ACTIVATED;
			send(redPlayerInfo->socket, (char*)&redPlayerInfo->protocol, sizeof(Protocol), NULL);
			ItemPacket i = *it;
			send(redPlayerInfo->socket, (char*)&i, sizeof(ItemPacket), NULL);

			bluePlayerInfo->protocol = Protocol::ITEM_ACTIVATED;
			send(bluePlayerInfo->socket, (char*)&bluePlayerInfo->protocol, sizeof(Protocol), NULL);
			send(bluePlayerInfo->socket, (char*)&i, sizeof(ItemPacket), NULL);
			itemList->erase(it);
		}
}

void CallBlindItem() {
	auto endTime = std::chrono::steady_clock::now();
	std::vector<BlindPlayer>::iterator blindIt;
	for (blindIt = g_blindedPlayer.begin(); blindIt != g_blindedPlayer.end(); ++blindIt) {
		auto t = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - blindIt->startBlindTime).count();
		if (t > 5000) {
			blindIt->playerData->protocol = Protocol::ITEM_BLIND_FINISHED;
			bool flag = false;
			send(blindIt->playerData->socket, (char*)&blindIt->playerData->protocol, sizeof(Protocol), NULL);
			send(blindIt->playerData->socket, (char*)&flag, sizeof(bool), NULL);

			g_blindedPlayer.erase(blindIt);
			return;
		}
	}
}

void CallParingSwords(PlayerPacket* playerPacket, PlayerPacket* enemyPacket, char playerKey, char enemyKey, char inputKey=NULL) {
	constexpr int callCount = 1;
	if (inputKey) {
		PlayerPacketManager::RotateSword(&playerPacket->sword, (inputKey == 'A') ? ('D') : ('A'));
	}
	else {
		PlayerPacketManager::RotateSword(&playerPacket->sword, playerKey);
	}

	for (int i = 0; i < callCount; ++i) {
		if (inputKey) {
			PlayerPacketManager::RotateSword(&playerPacket->sword, (inputKey == 'A') ? ('D') : ('A') );
			PlayerPacketManager::RotateSword(&enemyPacket->sword, enemyKey);
		}
		else {
			PlayerPacketManager::RotateSword(&playerPacket->sword, playerKey);
			PlayerPacketManager::RotateSword(&enemyPacket->sword, enemyKey);
		}
	}
}

// Į�� Į�� �浹�ϸ�
void CheckCollision_SwordAndSword(PlayerPacket* playerPacket, PlayerPacket* enemyPacket, char playerInputKey=NULL) {
	const int SWORD_HEIGHT_HALF = 10; 

	// player
	int playerPointsLength = 3 + (playerPacket->sword.length / SWORD_HEIGHT_HALF);
	int playerSwordLength = playerPacket->sword.length;
	double playerSwordRad = DegToRad(playerPacket->sword.rotationDegree);
	Packet playerPoint = { playerPacket->x, playerPacket->y };

	std::vector<Packet> playerSwordCollisionMidPoints;
	if (playerPointsLength > 0) {
		playerSwordCollisionMidPoints.reserve(playerPointsLength);
		for (int i = 0; i < playerPointsLength; ++i) {
			int x = 100 + 2 * playerSwordLength - SWORD_HEIGHT_HALF - 2 * SWORD_HEIGHT_HALF * i;
			int y = 0;

			double rotatedX = (x * cos(playerSwordRad) - y * sin(playerSwordRad) + playerPoint.x);
			double rotatedY = (y * cos(playerSwordRad) + x * sin(playerSwordRad) + playerPoint.y);

			Packet data = { (float)rotatedX, (float)rotatedY };
			playerSwordCollisionMidPoints.emplace_back(data);
		}
	}

	// enemy
	int enemyPointsLength = 3 + (enemyPacket->sword.length / SWORD_HEIGHT_HALF);
	int enemySwordLength = enemyPacket->sword.length;
	double enemySwordRad = DegToRad(enemyPacket->sword.rotationDegree);
	Packet enemyPoint = { enemyPacket->x, enemyPacket->y };

	std::vector<Packet> enemySwordCollisionMidPoints;
	if (enemyPointsLength > 0) {
		enemySwordCollisionMidPoints.reserve(enemyPointsLength);
		for (int i = 0; i < enemyPointsLength; ++i) {
			int x = 100 + 2 * SWORD_HEIGHT_HALF - SWORD_HEIGHT_HALF - 2 * SWORD_HEIGHT_HALF * i;
			int y = 0;

			double rotatedX = (x * cos(enemySwordRad) - y * sin(enemySwordRad) + enemyPoint.x);
			double rotatedY = (y * cos(enemySwordRad) + x * sin(enemySwordRad) + enemyPoint.y);

			Packet data = { (float)rotatedX, (float)rotatedY };
			enemySwordCollisionMidPoints.emplace_back(data);
		}
	}

	constexpr short SWORD_MAX_LENGTH = 50;
	constexpr short SWORD_MIN_LENGTH = 10;
	constexpr short SWORD_LENGTH_VALUE = 5;

	// CheckCollision_SwordAndSword
	for (Packet& pl : playerSwordCollisionMidPoints) {
		for (Packet& en : enemySwordCollisionMidPoints) {
			//std::cout << en.x << " " << pl.x << " | " << en.y << " " << pl.y << std::endl;
			if ((std::powf((en.x - pl.x), 2) + std::powf((en.y - pl.y), 2))
				<= std::powf(2 * SWORD_HEIGHT_HALF, 2)) {

				Packet playerVector2D = {
					playerSwordCollisionMidPoints.front().x - playerSwordCollisionMidPoints.back().x,
					playerSwordCollisionMidPoints.front().y - playerSwordCollisionMidPoints.back().y
				};
				Packet enemyVector2D = {
					enemySwordCollisionMidPoints.front().x - enemySwordCollisionMidPoints.back().x,
					enemySwordCollisionMidPoints.front().y - enemySwordCollisionMidPoints.back().y 
				};

				float setaDeg = acosf(
					(playerVector2D.x * enemyVector2D.x + playerVector2D.y * enemyVector2D.y) / 
					( std::sqrtf(playerVector2D.x * playerVector2D.x + playerVector2D.y * playerVector2D.y) *
						std::sqrtf(enemyVector2D.x * enemyVector2D.x + enemyVector2D.y * enemyVector2D.y) )
				);
				setaDeg = (float)RadToDeg(setaDeg);

				time_t currTime = time(nullptr);
				tm* currTm = localtime(&currTime);
				std::cout << currTm->tm_min << ":" << currTm->tm_sec << " ";
				if (setaDeg <= 90) {
					if(playerPacket->y <= enemyPacket->y) {
						if (90.0f <= playerPacket->sword.rotationDegree && playerPacket->sword.rotationDegree <= 270.0f) {
							CallParingSwords(playerPacket, enemyPacket, 'D', 'A', playerInputKey);
						}
						else {
							CallParingSwords(playerPacket, enemyPacket, 'A', 'D', playerInputKey);
						}
					}
					else {
						if (90.0f <= enemyPacket->sword.rotationDegree && enemyPacket->sword.rotationDegree <= 270.0f) {
							CallParingSwords(playerPacket, enemyPacket, 'A', 'D', playerInputKey);
						}
						else {
							CallParingSwords(playerPacket, enemyPacket, 'D', 'A', playerInputKey);
						}
					}
				}
				else {
					if (playerSwordCollisionMidPoints.front().x >= enemySwordCollisionMidPoints.front().x) {
						CallParingSwords(playerPacket, enemyPacket, 'A', 'A', playerInputKey);
					}
					else {
						CallParingSwords(playerPacket, enemyPacket, 'D', 'D', playerInputKey);
					}
				}
				enemyPacket->sword.length = std::clamp((short)(enemyPacket->sword.length - SWORD_LENGTH_VALUE),
					SWORD_MIN_LENGTH, SWORD_MAX_LENGTH);
				playerPacket->sword.length = std::clamp((short)(playerPacket->sword.length - SWORD_LENGTH_VALUE),
					SWORD_MIN_LENGTH, SWORD_MAX_LENGTH);
				return;
			}
		}
	}
}
