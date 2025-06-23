#pragma once
#include <SDL3/SDL.h>
#include <vector>

class Star
{
	float mInitialX;
	float mPosX;
	float mPosY;
	double mAngle;

public:
	Star(float x, float y);

	void render(SDL_Renderer* renderer);

	void update();

	static void InitStars(std::vector<Star*>& stars);

	static void CreateStarPoints(float x, float y, float angle, SDL_FPoint* outPoints);
};