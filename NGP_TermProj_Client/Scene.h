#pragma once
#include"UiImage.h"
#include"Camera.h"
#include"NetworkModule.h"

enum class SceneSequence {
	LOBBY,
	INGAME,
	ENDING,
};

enum class SceneState {
	RUNNING,
	FADEIN,
	FADEOUT,
};

class Scene
{
	/// <summary>
	/// Scene Member variables
	/// </summary>
	std::vector<Object*> _objects;
	/// <summary>
	/// my character index
	/// </summary>
	int _myCharacter;
	/// <summary>
	/// enemy character index
	/// </summary>
	int _enemyCharacter;
	/// <summary>
	/// current scene state
	/// </summary>
 	SceneSequence _sceneSequence;
	/// <summary>
	/// current scene state
	/// </summary>
	SceneState _sceneState;
	/// <summary>
	/// alphablending black image
	/// </summary>
	CImage _alphaImage;
	/// <summary>
	/// fade ratio
	/// </summary>
	unsigned char _fadeRatio;
	/// <summary>
	/// network module
	/// </summary>
	NetworkModule* _networkModule;
	/// <summary>
	/// game map class
	/// </summary>
	GameMap* _gameMap;
	/// <summary>
	/// rendering camera
	/// </summary>
	Camera* _camera;
	/// <summary>
	/// UI creating hwnd
	/// </summary>
	HWND _hWnd;
public:
	Scene(HWND hWnd);
	virtual ~Scene();
	UINT AddObject(Packet p);
	UINT AddObject(Object* object);
	void DeleteObject(UINT idx);

	void UpdatePlayerPacket();
	void UpdatePlayerBlindState(bool flag);
	UINT FindItemObject(Item object);

	SceneSequence GetSequence()const;
	void SetSequence(SceneSequence sceneSequence);

	void Update();
	void Render(HDC hDC) const;

	void ClearScene();
	void InitScene();
	void LoadLobby();
	void LoadIngame();
	void LoadResult(bool isWin);
	void ChangeScene(SceneSequence data);
	LRESULT KeyInputProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT MouseInputProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};