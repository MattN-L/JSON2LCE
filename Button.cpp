#include "Button.h"

Button::Button(const std::string& text, TTF_Font* font, float x, float y, float width, float height, std::function<void()> onClick) : 
	mRect{ x, y, width, height }, 
	mOnClick{ onClick }, 
	mFont{ font }, 
	mText{text},
	mBorder{ new SDL_FRect{ x, y, mRect.w + 5, mRect.h + 5 } } // 5 px border
{
}

void Button::render(SDL_Renderer* renderer) {
	SDL_Color color = { 0xDC, 0xC8, 0xF5, 0xFF };			// idle color
	if (mPressed) color = { 0xBF, 0xA3, 0xF5, 0xFF };		// pressed color
	else if (mHovered) color = { 0xEB, 0xD6, 0xFF, 0xFF };	// hover color

	// border
	SDL_SetRenderDrawColor(renderer, 0xB9, 0xB1, 0xD3, 0xFF);

	SDL_RenderFillRect(renderer, mBorder);

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, &mRect);

	SDL_Surface* textSurface =
		TTF_RenderText_Blended(mFont, mText.c_str(), 0, {0xFF, 0xFF, 0xFF, 0xFF});

	if (textSurface)
	{
		SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

		// Center text
		SDL_FRect textRect;
		textRect.w = (float)textSurface->w;
		textRect.h = (float)textSurface->h;
		textRect.x = mRect.x + (mRect.w - textRect.w) / 2.0f;
		textRect.y = mRect.y + (mRect.h - textRect.h) / 2.0f;

		SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);

		SDL_DestroyTexture(textTexture);
	}

	SDL_DestroySurface(textSurface);
}

void Button::handleEvent(const SDL_Event* event) {
	if (event->type == SDL_EVENT_MOUSE_MOTION || event->type == SDL_EVENT_MOUSE_BUTTON_DOWN || event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
		float x = event->motion.x;
		float y = event->motion.y;

		// check if mouse is in bounds of the button
		mHovered = (x >= mRect.x && x <= mRect.x + mRect.w && y >= mRect.y && y <= mRect.y + mRect.h);
		mPressed = false;

		if (mHovered && event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
			mPressed = true;
			mOnClick();
		}
		if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
			mPressed = false;
		}
	}
}