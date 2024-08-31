#include "SceneManager.h"

//float SceneManager::LoadScene(Scene* scene)
//{
//	switch (scene->GetSequence())
//	{
//	case SceneSequence::LOBBY:
//		for (int i = 0; i < 10; ++i) {
//			float posX, posY;
//			posX = static_cast<float>(rand() % SCREEN_WIDTH);
//			posY = static_cast<float>(rand() % SCREEN_HEIGHT);
//			scene->AddObject(posX, posY);
//		}
//		scene->SetSequence(SceneSequence::INGAME);
//		break;
//	case SceneSequence::INGAME:
//		scene->SetSequence(SceneSequence::ENDING);
//		break;
//	case SceneSequence::ENDING:
//		scene->SetSequence(SceneSequence::LOBBY);
//		break;
//	default:
//		printf("Unknown Scene Sequence!\n");
//		break;
//	}
//	return 0.0f;
//}

void SceneManager::FadeOut(Scene* sce6ne)
{
}

void SceneManager::FadeIn(Scene* scene)
{
}