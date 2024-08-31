#include "Scene.h"

extern bool g_isActiveWindow;

Scene::Scene(HWND hWnd):_sceneSequence{SceneSequence::LOBBY}, _sceneState{SceneState::FADEIN},
				_myCharacter{-1}, _enemyCharacter{ -1 }, _fadeRatio{0}, _hWnd{hWnd}
{
	_networkModule = new NetworkModule(this);
	_gameMap = new GameMap();
	_camera = new Camera();

	InitScene();
	_alphaImage.CreateEx(SCREEN_WIDTH, SCREEN_HEIGHT, 32, BI_RGB);
}

Scene::~Scene()
{
	for (Object* op : _objects)
		delete op;
	_objects.clear();

	if (_networkModule)
		delete _networkModule;

	if (_gameMap) {
		std::cout << "========\n";
		std::cout << "gameMap is active: " << bool(_gameMap) << std::endl;
		std::cout << "gameMap index: " << (_gameMap) << std::endl;
		std::cout << "gameMap is null: " << (_gameMap == NULL) << std::endl;
		std::cout << "gameMap is nullptr: " << (_gameMap == nullptr) << std::endl;
		std::cout << "========\n";
		delete _gameMap;
	}

	if (_camera)
		delete _camera;
	if (false == _alphaImage.IsNull())
		_alphaImage.Destroy();
}

UINT Scene::AddObject(Packet p)
{
	_objects.emplace_back(new Object(p));
	return (UINT)(_objects.size() - 1);
}

UINT Scene::AddObject(Object* object)
{
	_objects.emplace_back(object);
	return (UINT)(_objects.size() - 1);
}

UINT Scene::FindItemObject(Item object)
{
	std::vector<Object*>::iterator it = std::find_if(_objects.begin(), _objects.end(),
		[=](Object* element) {
			Item* item;
			if (!dynamic_cast<Item*>(element)) {
				return false;
			}
			else {
				item = dynamic_cast<Item*>(element);
				return (*item == object);
			}
		});
	return (std::distance(_objects.begin(), it));
}

SceneSequence Scene::GetSequence() const
{
	return _sceneSequence;
}

void Scene::SetSequence(SceneSequence sceneSequence)
{
	_sceneSequence = sceneSequence;
}

void Scene::DeleteObject(UINT idx)
{
	if (idx != _objects.size() && _objects[idx]) {
		Object* ob = _objects[idx];
		_objects.erase(_objects.begin() + idx);
		delete ob;
	}
}

void Scene::UpdatePlayerPacket() 
{
	//Player* player = (_myCharacter >= 0) ?
	//	(dynamic_cast<Player*>(_objects[_myCharacter])) : (nullptr);
	//Player* enemy = (_enemyCharacter >= 0) ?
	//	(dynamic_cast<Player*>(_objects[_enemyCharacter])) : (nullptr);
	
	if (!g_isActiveWindow)
		return;

	// Press A and Press D
	if (((GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState('a') & 0x8000)) &&
		((GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState('d') & 0x8000))) {
		Protocol p = Protocol::MOVE_REQUEST;
		_networkModule->SendData(p, NULL);
	}
	// Press A
	else if ((GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState('a') & 0x8000)) {
		Protocol p = Protocol::KEYDOWN_A_REQUEST;
		_networkModule->SendData(p, NULL);
		printf("send key A packet\n");
	}
	// Press D
	else if ((GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState('d') & 0x8000)) {
		Protocol p = Protocol::KEYDOWN_D_REQUEST;
		_networkModule->SendData(p, NULL);
		printf("send key D packet\n");
	}
	//PlayerPacket pPacket = _networkModule->GetPlayerPacket();
	//PlayerPacket enemyPacket = _networkModule->GetEnemyPacket();
	//player->SetPlayerPacket(pPacket);
	//enemy->SetPlayerPacket(enemyPacket);
	
}

void Scene::UpdatePlayerBlindState(bool flag)
{
	Player* player = (_myCharacter >= 0) ?
		(dynamic_cast<Player*>(_objects[_myCharacter])) : (nullptr);

	if (player)
		player->SetCheckBlinding(flag);
}

void Scene::Update()
{
	if (_sceneSequence == SceneSequence::INGAME) {
		_networkModule->DataUpdate();

		PlayerPacket playerPacket = _networkModule->GetPlayerPacket();
		PlayerPacket enemyPacket = _networkModule->GetEnemyPacket();

		Player* player = (_myCharacter >= 0) ?
			(dynamic_cast<Player*>(_objects[_myCharacter])) : (nullptr);
		Player* enemy = (_enemyCharacter >= 0) ?
			(dynamic_cast<Player*>(_objects[_enemyCharacter])) : (nullptr);

		if (player)
			player->SetPlayerPacket(playerPacket);
		if (enemy)
			enemy->SetPlayerPacket(enemyPacket);

		UpdatePlayerPacket();
		for (const auto& obj : _objects)
			obj->Update();

		if (player)
			_camera->Update(playerPacket.x - SCREEN_WIDTH / 2, playerPacket.y - SCREEN_HEIGHT / 2);
	}
}

void Scene::Render(HDC MemDC) const
{
	if (MemDC) {
		_camera->Render(MemDC, _objects);
	}

	//alphablending code here
}

void Scene::ClearScene()
{
	for (int i = 0; i < _objects.size(); ++i)
		DeleteObject(i);
	_objects.clear();
	_myCharacter = -1;
	_enemyCharacter = -1;
	_gameMap = nullptr;

	printf("clear scene\n");
}

void Scene::InitScene()
{
	switch (_sceneSequence)
	{
	case SceneSequence::LOBBY:
	{
		Scene::LoadLobby();
	}
		break;
	case SceneSequence::INGAME:
	{
		Scene::LoadIngame();
	}
		break;
	case SceneSequence::ENDING:
	{	
		Scene::LoadResult(true);
	}
		break;
	}
}

void Scene::LoadLobby()
{
	if(!_gameMap)
		_gameMap = new GameMap();
	_gameMap->LoadLobby();
	AddObject(_gameMap);

	_camera->ScrollOff();
}

void Scene::LoadIngame()
{
	if(!_gameMap)
		_gameMap = new GameMap();
	_gameMap->LoadIngame();
	AddObject(_gameMap);
	Object* op = new Player(_networkModule->GetPlayerPacket());
	_myCharacter = AddObject(op);

	op = new Player(_networkModule->GetEnemyPacket());
	_enemyCharacter = AddObject(op);
	_camera->ScrollOn();
	std::cout << "player x, y : " << _networkModule->GetPlayerPacket().x << " " << _networkModule->GetPlayerPacket().y <<
		"enemy x, y : " << _networkModule->GetEnemyPacket().x << " " << _networkModule->GetEnemyPacket().y << std::endl;
	// _sceneSequence = SceneSequence::ENDING;
}

void Scene::LoadResult(bool isWin)
{
	if (!_gameMap)
		_gameMap = new GameMap();
	_gameMap->LoadResult(isWin);
	AddObject(_gameMap);
	_camera->ScrollOff();

	_sceneSequence = SceneSequence::LOBBY;
}

void Scene::ChangeScene(SceneSequence data)
{
	printf("¾Àº¯°æÁØºñ\n");
	_sceneState = SceneState::FADEOUT;
	ClearScene();
	_sceneSequence = data;
	InitScene();
	_sceneState = SceneState::FADEIN;
	printf("changed scene\n");
}

LRESULT Scene::KeyInputProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_LEFT:
		break;
	case VK_RIGHT:
		break;
	case VK_UP:
		break;
	case VK_DOWN:
		break;
	case VK_SPACE:
		if(_sceneSequence == SceneSequence::LOBBY)
			_networkModule->SendData(Protocol::MATCH_REQUEST, NULL);
		break;
	case VK_ESCAPE:
		PostQuitMessage(0);
		break;
	default:
		break;
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT Scene::MouseInputProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
		// _networkModule->SendData(Protocol::MATCH_REQUEST, NULL);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}      