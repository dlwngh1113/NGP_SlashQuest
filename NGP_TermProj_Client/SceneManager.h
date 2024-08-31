#pragma once
#include"Scene.h"

class SceneManager
{
	static CImage _fadeImage;
public:
	static float LoadScene(Scene* scene);
	static void FadeOut(Scene* scene);
	static void FadeIn(Scene* scene);
};