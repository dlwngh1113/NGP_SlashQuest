#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32")
#include<stdio.h>
#include<iostream>
#include<string>
#include<WS2tcpip.h>
#include<WinSock2.h>
#include<unordered_map>
#include<utility>
#include<chrono>

#include"protocol.h"
#include"PlayerPacketManager.h"

CRITICAL_SECTION g_cs;

void err_quit(const char* msg);
void err_display(const char* msg);
DWORD WINAPI ServerMainThread(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);
void check_id(const char* id, GameClientInfo* gp);

auto g_itemTimer = std::chrono::steady_clock::now();
std::vector<ItemPacket> g_itemsDataList;
std::vector<BlindPlayer> g_blindedPlayer;

void SpawnItem(std::vector<ItemPacket>* itemList, GameClientInfo* player, GameClientInfo* enemy);
std::tuple<GameClientInfo*, GameClientInfo*, bool> isPlayerMatching(GameClientInfo* gp);
bool isMatchOver(GameClientInfo* player1, GameClientInfo* player2);

std::unordered_map<std::string, GameClientInfo*> g_players;
std::vector <std::tuple<GameClientInfo*, GameClientInfo*, bool>> g_matchingPlayers;

int main(int argc, char* argv[])
{
	InitializeCriticalSection(&g_cs);

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	ADDRINFO* result = nullptr;
	ADDRINFO serverAddrInfo;
	ZeroMemory(&serverAddrInfo, sizeof(serverAddrInfo));
	serverAddrInfo.ai_family = AF_INET;
	serverAddrInfo.ai_socktype = SOCK_STREAM;
	serverAddrInfo.ai_protocol = IPPROTO_TCP;
	serverAddrInfo.ai_flags = AI_PASSIVE;

	int rv = getaddrinfo(NULL, "9000", &serverAddrInfo, &result);

	// SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (INVALID_SOCKET == listenSocket) 
		err_quit("socket()");

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int retval = bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR) {
		err_quit("bind()");
	}

	retval = listen(listenSocket, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		err_quit("listen()");
	}

	//SOCKADDR_IN clientAddr;
	//ZeroMemory(&clientAddr, sizeof(clientAddr));
	//int addrlen = sizeof(clientAddr);
	//HANDLE hThread;

	HANDLE mainThread = CreateThread(NULL, 0, ServerMainThread, 0, 0, NULL);
	if (mainThread == NULL)
		return 0;
	else
		CloseHandle(mainThread);

	while (true) {
		SOCKADDR_IN clientAddr;
		ZeroMemory(&clientAddr, sizeof(clientAddr));
		int addrlen = sizeof(clientAddr);
		HANDLE hThread;
		SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrlen);
		if (INVALID_SOCKET == clientSocket) {
			err_display("accept()");
			break;
		}

		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)clientSocket, 0, NULL);
		if (NULL == hThread)
			closesocket(clientSocket);
		else
			CloseHandle(hThread);
	}
	closesocket(listenSocket);

	for (auto& p : g_players)
		delete p.second;
	g_players.clear();
	out.close();
	DeleteCriticalSection(&g_cs);
	WSACleanup();
	return 0;
}

void check_id(const char* id, GameClientInfo* gp)
{ 
	if (g_players.count(id)) {
		gp->protocol = Protocol::LOGIN_FAIL;
		out << "로그인 중복 확인" << std::endl;
		send(gp->socket, (char*)&gp->protocol, sizeof(Protocol), NULL);
	}
	else {
		gp->protocol = Protocol::LOGIN_SUCCESS;
		gp->name = id;
		gp->matchState = MatchingState::LOBBY;
		strcpy(gp->player.id, id);
		g_players[id] = gp;
		PlayerPacketManager::InitPlayerData(&gp->player);
		send(gp->socket, (char*)&gp->protocol, sizeof(Protocol), NULL);
		//send(gp->socket, (char*)&gp->player, sizeof(PlayerPacket), NULL);
		out << "send login data\n" << std::endl;
	}
}

std::tuple<GameClientInfo*, GameClientInfo*, bool> isPlayerMatching(GameClientInfo* gp)
{
	for (auto& p : g_matchingPlayers)
	{
		if (std::get<0>(p) == gp) {
			std::get<2>(p) = true;
			return p;
		}
		else if (std::get<1>(p) == gp) {
			std::get<2>(p) = true;
			return p;
		}
	}
	GameClientInfo* nptr = nullptr;
	return std::make_tuple(nptr, nptr, false);
}

bool isMatchOver(GameClientInfo* player, GameClientInfo* enemy)
{
	if (0 == player->player.hp) {
		EnterCriticalSection(&g_cs);
		player->protocol = Protocol::SYSTEM_LOSE;
		send(player->socket, (char*)&player->protocol, sizeof(Protocol), NULL);
		player = nullptr;

		enemy->protocol = Protocol::SYSTEM_WIN;
		send(enemy->socket, (char*)&enemy->protocol, sizeof(Protocol), NULL);
		enemy = nullptr;
		LeaveCriticalSection(&g_cs);
		return true;
	}
	else if (0 == enemy->player.hp) {
		EnterCriticalSection(&g_cs);
		if (enemy) {
			enemy->protocol = Protocol::SYSTEM_LOSE;
			send(enemy->socket, (char*)&enemy->protocol, sizeof(Protocol), NULL);
		}

		if (player) {
			player->protocol = Protocol::SYSTEM_WIN;
			send(player->socket, (char*)&player->protocol, sizeof(Protocol), NULL);
		}
		LeaveCriticalSection(&g_cs);
		return true;
	}
	return false;
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	GameClientInfo* client = new GameClientInfo;
	client->socket = (SOCKET)arg;
	client->matchState = MatchingState::LOBBY;
	int retval;

	SOCKADDR_IN addr;
	int len = sizeof(addr);
	getpeername(client->socket, (sockaddr*)&addr, &len);

	GameClientInfo* playerData = nullptr;
	GameClientInfo* enemyData = nullptr;

	while (true) {
		retval = recv(client->socket, (char*)&client->protocol, sizeof(Protocol), NULL);
		if (retval == SOCKET_ERROR)
			break;
		else if (retval == 0)
			break;
		EnterCriticalSection(&g_cs);

		switch (client->protocol)
		{
			case Protocol::LOGIN_REQUEST:
			{
				char buf[MAX_ID_LEN + 1] = { NULL };
				recv(client->socket, buf, MAX_ID_LEN, NULL);
				check_id(buf, client);
			}
				break;
			case Protocol::MATCH_REQUEST:
			{
				//std::cout << "enter match critical section 1\n";
				bool matchSuccessed = false;
				client->matchState = MatchingState::MATCHING;
				// red - left side, blue - right side
				// this line means 1st player connect(second connect, blue player connect)
				std::tuple<GameClientInfo*, GameClientInfo*, bool> p = isPlayerMatching(client);
				if (std::get<1>(p)) {
					client->protocol = Protocol::MATCH_SUCCESS;
					client->matchState = MatchingState::GAMING;

					GameClientInfo* redPlayerPacket = std::get<0>(p);
					GameClientInfo* bluePlayerPacket = std::get<1>(p);

					playerData = bluePlayerPacket;
					enemyData = redPlayerPacket;

					send(client->socket, (char*)&client->protocol, sizeof(Protocol), NULL);
					send(client->socket, (char*)&bluePlayerPacket->player, sizeof(PlayerPacket), NULL); // player
					send(client->socket, (char*)&redPlayerPacket->player, sizeof(PlayerPacket), NULL); // enemy
					matchSuccessed = true;
					out << "blue player(" << bluePlayerPacket->player.id << ") join ingame. enemy: " <<
						redPlayerPacket->player.id << "\n";

					break;
				}

				// this line means 2nd player connect(first connect, red player connect)
				for (auto& bluePlayer : g_players)
				{
					if ((bluePlayer.second->matchState == MatchingState::MATCHING) && (bluePlayer.second->name != client->name))
					{
						matchSuccessed = true;
						client->protocol = Protocol::MATCH_SUCCESS;
						
						g_matchingPlayers.emplace_back(std::make_tuple(client, bluePlayer.second, false));

						bluePlayer.second->player.x = 600.f;
						bluePlayer.second->player.sword.rotationDegree = 180.f;

						playerData = client;
						enemyData = bluePlayer.second;

						send(client->socket, (char*)&client->protocol, sizeof(Protocol), NULL);
						send(client->socket, (char*)&client->player, sizeof(PlayerPacket), NULL); // player
						send(client->socket, (char*)&bluePlayer.second->player, sizeof(PlayerPacket), NULL); // enemy

						out << "red player(" << playerData->player.id << ") join ingame. enemy: " <<
							enemyData->player.id << "\n";
						client->matchState = MatchingState::GAMING;

						break;
					}
				}
				if (matchSuccessed)
					break;

				client->protocol = Protocol::MATCH_FAIL;
				send(client->socket, (char*)&client->protocol, sizeof(Protocol), NULL);
				std::cout << ntohs(addr.sin_port) << "번호, " << client->name << " 이름, 매칭실패" << std::endl;
			}
				break;
			case Protocol::ENTER_INGAME_REQUEST:
				break;
			case Protocol::MOVE_REQUEST:
			{
				PlayerPacketManager::MovePlayer(&playerData->player);
				//std::cout << client->name << " client is Moving\n";
				// client->protocol = Protocol::MOVE_SUCCESS;
				// send(client->socket, (char*)&client->protocol, sizeof(Protocol), NULL);
			}
				break;
			case Protocol::KEYDOWN_A_REQUEST:
			{
				PlayerPacketManager::RotateSword(&playerData->player.sword, 'A');
				//std::cout << client->name << " client Keydown A Key\n";
				// client->protocol = Protocol::KEYDOWN_SUCCESS;
				// send(client->socket, (char*)&client->protocol, sizeof(Protocol), NULL);
			}
				break;
			case Protocol::KEYDOWN_D_REQUEST:
			{
				PlayerPacketManager::RotateSword(&playerData->player.sword, 'D');
				//std::cout << client->name << " client Keydown D Key\n";
				// client->protocol = Protocol::KEYDOWN_SUCCESS;
				// send(client->socket, (char*)&client->protocol, sizeof(Protocol), NULL);
			}
				break;
			case Protocol::PROCESS_POINTS_REQUSET:
			{
				//EnterCriticalSection(&g_cs);
				//send(client->socket, (char*)&client->protocol, sizeof(Protocol), NULL);

				//if (playerData && enemyData) {
				//	// printf("send player stat\n");
				//	CheckCollision_SwordAndPlayer(&playerData->player, &enemyData->player);
				//	CheckCollision_SwordAndSword(&playerData->player, &enemyData->player);
				//	CheckCollision_PlayerAndItem(&playerData->player, &itemsDataList);

				//	SpawnItem(&itemsDataList, playerData, enemyData);

				//	if (isMatchOver(playerData, enemyData)) {
				//		EnterCriticalSection(&g_cs);
				//		g_matchingPlayers.pop_back();
				//		playerData = enemyData = nullptr;
				//		LeaveCriticalSection(&g_cs);
				//		break;
				//	}

				//	send(client->socket, (char*)&playerData->player, sizeof(PlayerPacket), 0);
				//	send(client->socket, (char*)&enemyData->player, sizeof(PlayerPacket), 0);
				}
				break;
			default:
				break;
		}
		LeaveCriticalSection(&g_cs);
	}

	out << "[TCP 서버] 클라이언트 종료 : IP주소 = " << inet_ntoa(addr.sin_addr) <<
		", 포트 번호 = " << ntohs(addr.sin_port) << std::endl;

	EnterCriticalSection(&g_cs);
	if (enemyData) {
		enemyData->matchState = MatchingState::LOBBY;
		enemyData->protocol = Protocol::ENEMY_DISCONNECTED;
		PlayerPacketManager::InitPlayerData(&enemyData->player);
		send(enemyData->socket, (char*)&enemyData->protocol, sizeof(Protocol), NULL);

		if (!g_matchingPlayers.empty())
			g_matchingPlayers.pop_back();
	}
	closesocket(client->socket);
	g_players.erase(client->name);
	delete client;
	LeaveCriticalSection(&g_cs);
	return 0;
}

//아이템 처리(인게임 입장할때만 쓰레드로 동작하게 해야함)
void SpawnItem(std::vector<ItemPacket>* itemList, GameClientInfo* redPlayer, GameClientInfo* bluePlayer)
{
	auto endTime = std::chrono::steady_clock::now();

	auto t = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - g_itemTimer).count();
	if (t > 3000)
	{
		ItemPacket item;
		srand(time(NULL));
		item.type = rand() % 3;
		item.x = static_cast<float>(rand() % SCREEN_WIDTH);
		item.y = static_cast<float>(rand() % SCREEN_HEIGHT);

		redPlayer->protocol = Protocol::ITEM_CREATED;
		send(redPlayer->socket, (char*)&redPlayer->protocol, sizeof(Protocol), NULL);
		send(redPlayer->socket, (char*)&item, sizeof(item), NULL);

		bluePlayer->protocol = Protocol::ITEM_CREATED;
		send(bluePlayer->socket, (char*)&bluePlayer->protocol, sizeof(Protocol), NULL);
		send(bluePlayer->socket, (char*)&item, sizeof(item), NULL);

		itemList->emplace_back(item);

		out << "[Item Spawned] t = " << t << ", item.type = " << item.type << ", item.x = " << item.x <<
			", item.y = " << item.y << std::endl;
		g_itemTimer = endTime;
	}
}


void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL); 
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCSTR)(msg), MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

DWORD WINAPI ServerMainThread(LPVOID arg)
{
	//std::vector<std::tuple<GameClientInfo*, GameClientInfo*, bool>>::iterator it;
	while (true) {
		for (auto it = g_matchingPlayers.begin(); it != g_matchingPlayers.end(); ++it) {
			GameClientInfo* redPlayerData = std::get<0>(*it);
			GameClientInfo* bluePlayerData = std::get<1>(*it);

			if (redPlayerData && bluePlayerData && std::get<2>(*it)) {
				EnterCriticalSection(&g_cs);

				CheckCollision_SwordAndPlayer(&redPlayerData->player, &bluePlayerData->player);
				CheckCollision_SwordAndPlayer(&bluePlayerData->player, &redPlayerData->player);

				CheckCollision_SwordAndSword(&redPlayerData->player, &bluePlayerData->player);

				CheckCollision_PlayerAndItem(redPlayerData, bluePlayerData, &g_itemsDataList);
				CheckCollision_PlayerAndItem(bluePlayerData, redPlayerData, &g_itemsDataList);

				if (isMatchOver(redPlayerData, bluePlayerData)) {
					if (!g_matchingPlayers.empty())
						g_matchingPlayers.pop_back();
					LeaveCriticalSection(&g_cs);
					break;
				}

				SpawnItem(&g_itemsDataList, redPlayerData, bluePlayerData);
				CallBlindItem();

				redPlayerData->protocol = Protocol::PROCESS_POINTS_REQUSET;
				send(redPlayerData->socket, (char*)&redPlayerData->protocol, sizeof(Protocol), NULL);

				send(redPlayerData->socket, (char*)&redPlayerData->player, sizeof(PlayerPacket), 0);
				send(redPlayerData->socket, (char*)&bluePlayerData->player, sizeof(PlayerPacket), 0);

				bluePlayerData->protocol = Protocol::PROCESS_POINTS_REQUSET;
				send(bluePlayerData->socket, (char*)&bluePlayerData->protocol, sizeof(Protocol), NULL);

				send(bluePlayerData->socket, (char*)&bluePlayerData->player, sizeof(PlayerPacket), 0);
				send(bluePlayerData->socket, (char*)&redPlayerData->player, sizeof(PlayerPacket), 0);

				LeaveCriticalSection(&g_cs);
			}

		}
		SleepEx(GAME_FPS, TRUE);
	}
}
