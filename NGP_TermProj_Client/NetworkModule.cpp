#include "NetworkModule.h"
#include"Scene.h"

NetworkModule::NetworkModule(Scene* scene):_scene{scene}
{
	ZeroMemory(&_playerPacket, sizeof(PlayerPacket));
	ZeroMemory(&_enemyPacket, sizeof(PlayerPacket));

	InitializeCriticalSection(&_cs);

	ConnectToServer();

	_threadHandle = CreateThread(NULL, 0, NetworkUpdateThread, (LPVOID)this, 0, NULL);
	if (_threadHandle == NULL)
		closesocket(_socket);
}

NetworkModule::~NetworkModule()
{
	DeleteCriticalSection(&_cs);
	closesocket(_socket);
	WSACleanup();
}

void NetworkModule::ConnectToServer()
{
	int retval = 0;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return;
	}

	_socket = socket(AF_INET, SOCK_STREAM, NULL);

	//int option = true;
	//setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&option, sizeof(option));
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);

	while (true) {
		char ipAddr[20] = { NULL };
		printf("서버의 IP주소를 입력하세요:");
		scanf_s("%s", ipAddr, 20);

		addr.sin_addr.s_addr = inet_addr(ipAddr);
		std::cout << ipAddr << std::endl;

		retval = connect(_socket, (sockaddr*)&addr, sizeof(addr));
		if (retval != SOCKET_ERROR)
			break;
		printf("연결에 실패했습니다\n");
	}
	printf("서버에 연결되었습니다!\n사용할 ID를 입력해주세요(10자 이내):");
	while (true) {
		char id[MAX_ID_LEN] = { NULL };
		scanf_s("%s", id, MAX_ID_LEN - 1);
		Protocol p = Protocol::LOGIN_REQUEST;
		send(_socket, (char*)&p, sizeof(Protocol), NULL);
		send(_socket, id, MAX_ID_LEN, NULL);

		recv(_socket, (char*)&p, sizeof(Protocol), NULL);
		if (p == Protocol::LOGIN_SUCCESS) {
			// RecvPlayerData(PlayerType::Player);
			printf("로그인 성공!\n");
			
			break;
		}
		else {
			printf("이미 존재하는 ID입니다.\n다시 입력해주세요:");
		}
	}

	//while (true) {
	//	Protocol p = Protocol::MATCH_REQUEST;
	//	send(_socket, (char*)&p, sizeof(Protocol), 0);
	//	Sleep(1000 / 33);
	//	recv(_socket, (char*)&p, sizeof(Protocol), 0);
	//	if (p == Protocol::MATCH_SUCCESS) {
	//		std::cout << "매칭 성공.\n";
	//		RecvPlayerData(PlayerType::Player);
	//		RecvPlayerData(PlayerType::Enemy);
	//		break;
	//	}
	//	else if (p == Protocol::MATCH_FAIL) {
	//		std::cout << "매칭 실패. 다시 매칭중\n";
	//	}
	//	else {
	//		std::cout << "문제발생, protocol: " << (int)p << "\n";
	//	}
	//}
}

void NetworkModule::SendData(Protocol p, Packet* packet)
{
	send(_socket, (char*)&p, sizeof(Protocol), NULL);
	if (packet) {

	}
}

void NetworkModule::RecvData(Protocol p, Packet* packet)
{

}

void NetworkModule::RecvSwordData(PlayerType type)
{
	short packetSize;
	char buf[BUF_SIZE + 1];

	packetSize = recv(_socket, buf, BUF_SIZE, NULL);
	buf[packetSize] = '\0';

	((type == PlayerType::Player) ? (_playerPacket.sword) : (_enemyPacket.sword)) = (Sword&)buf;
	// printf("recv sword data\n");
	// std::cout << _playerPacket.sword.rotationDegree << std::endl;
}

void NetworkModule::RecvPlayerData(PlayerType type)
{
	short packetSize;
	char buf[BUF_SIZE + 1];
	
	packetSize = recv(_socket, buf, sizeof(PlayerPacket), NULL);
	//std::cout << packetSize << std::endl;
	buf[packetSize] = '\0';

	((type == PlayerType::Player) ? (_playerPacket) : (_enemyPacket)) = (PlayerPacket&)buf;

	// ((type == PlayerType::Player) ? (printf("recv player packet\n")) : (printf("recv enemy packet\n")));
	
	//PlayerPacket a;
	//a = ((type == PlayerType::Player) ? (_playerPacket) : (_enemyPacket));
	//std::cout << "id: " << a.id << " " << std::endl;
	//std::cout << "hp: " << a.hp << std::endl;
	//std::cout << "x: " << a.x << std::endl;
	//std::cout << "y: " << a.y << std::endl;
	//std::cout << "sword rad: " << a.sword.rotationDegree << std::endl;
	//std::cout << "god: " << a.isGodMode << std::endl;
} 

DWORD WINAPI NetworkModule::NetworkUpdateThread(LPVOID arg)
{
	NetworkModule* network = (NetworkModule*)arg;
	return network->NetworkUpdate(network->_socket);
}

DWORD WINAPI NetworkModule::NetworkUpdate(SOCKET s)
{
	printf("call thread\n");
	//SOCKET _socket = (SOCKET)s;
	_socket = (SOCKET)s;
	int retval = 0;
	Protocol p = Protocol::MOVE_SUCCESS;
	while (true)
	{
		retval = recv(_socket, (char*)&p, sizeof(Protocol), NULL);
		if (retval == WSAEWOULDBLOCK)
			continue;
		else if (0 == retval)
			continue;
		EnterCriticalSection(&_cs);
		switch (p)
		{
		case Protocol::MATCH_FAIL:
		{
			std::cout << "매칭 실패. 다시 시도중\n";
			SendData(Protocol::MATCH_REQUEST, NULL);
		}
			break;
		case Protocol::MATCH_SUCCESS:
		{
			std::cout << "매칭 성공.\n";
			RecvPlayerData(PlayerType::Player);
			RecvPlayerData(PlayerType::Enemy);
			_scene->ChangeScene(SceneSequence::INGAME);
		}
			break;
		case Protocol::GAME_START:
			break;
		case Protocol::KEYDOWN_SUCCESS:
			//RecvSwordData(PlayerType::Player);
			//printf("recv key A or D packet\n");
			break;
		case Protocol::MOVE_SUCCESS:
			//RecvPlayerData(PlayerType::Player);
			break;
		case Protocol::ITEM_CREATED:
		{
			ItemPacket item;
			recv(_socket, (char*)&item, sizeof(item), NULL);
			Object* object = new Item(item);
			_scene->AddObject(object);
		}
			break;
		case Protocol::ITEM_ACTIVATED:
		{
			ItemPacket itemPacket;
			recv(_socket, (char*)&itemPacket, sizeof(itemPacket), NULL);
			Item item(itemPacket);

			int idx = _scene->FindItemObject(item);
			_scene->DeleteObject(idx);
		}
			break;
		case Protocol::ITEM_BLIND_ACTIVATED: {
			bool flag;
			recv(_socket, (char*)&flag, sizeof(bool), NULL);
			_scene->UpdatePlayerBlindState(flag); 
		}
			break;
		case Protocol::ITEM_BLIND_FINISHED: {
			bool flag;
			recv(_socket, (char*)&flag, sizeof(bool), NULL);
			_scene->UpdatePlayerBlindState(flag);
		}
			break;
		case Protocol::ENEMY_DISCONNECTED:
			_scene->ChangeScene(SceneSequence::LOBBY);
			break;
		case Protocol::PROCESS_POINTS_REQUSET:
			//std::cout << "try update points\n";
			RecvPlayerData(PlayerType::Player);
			RecvPlayerData(PlayerType::Enemy);
			break;
		case Protocol::SYSTEM_WIN:
			_scene->LoadResult(true);
			break;
		case Protocol::SYSTEM_LOSE:
			_scene->LoadResult(false);
			break;
		default:
			break;
		}
		LeaveCriticalSection(&_cs);
	}
	return 0;
}

void NetworkModule::DataUpdate()
{
	Protocol p = Protocol::PROCESS_POINTS_REQUSET;
	send(_socket, (char*)&p, sizeof(Protocol), NULL);
}

int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}
	return (len - left);
}
