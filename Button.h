#pragma once
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <functional>

class Button {
	SDL_FRect mRect;
	SDL_FRect* mBorder;

	TTF_Font* mFont;
	std::string mText;

	bool mHovered = false;
	bool mPressed = false;

	std::function<void()> mOnClick;

public:
	Button(const std::string& text, TTF_Font* font, float x, float y, float width, float height, std::function<void()> onClick);

	void render(SDL_Renderer* renderer);

	void handleEvent(const SDL_Event* event);
};