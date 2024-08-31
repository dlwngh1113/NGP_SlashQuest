#pragma once
#include"Player.h"

class Scene;

int recvn(SOCKET s, char* buf, int len, int flags);

enum class PlayerType {
	Player,
	Enemy
};

class NetworkModule
{
private:
	SOCKET _socket;
	HANDLE _threadHandle;
	CRITICAL_SECTION _cs;

	PlayerPacket _playerPacket;
	PlayerPacket _enemyPacket;

	Scene* _scene;

	// char _id[MAX_ID_LEN] = { NULL };
public:
	NetworkModule(Scene* scene);
	virtual ~NetworkModule();
	void ConnectToServer();

	const PlayerPacket& GetPlayerPacket() const { return (_playerPacket); }
	const PlayerPacket& GetEnemyPacket() const { return (_enemyPacket); }

	void SendData(Protocol p, Packet* packet);
	void RecvData(Protocol p, Packet* packet);

	void RecvSwordData(PlayerType type);
	void RecvPlayerData(PlayerType type);

	static DWORD WINAPI NetworkUpdateThread(LPVOID arg);
	DWORD WINAPI NetworkUpdate(SOCKET s);

	void DataUpdate();
};