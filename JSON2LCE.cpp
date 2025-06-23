#include "JSON2LCE.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_dialog.h>
#include <SDL3_ttf/SDL_ttf.h>

#define SDL_MAIN_USE_CALLBACKS 1

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

#define WIDTH	512
#define HEIGHT	512
#define FPS		60

std::vector<Star*> stars;

TTF_Font* MayFont{}; // yeah so what if I'm keeping the name lol
Button* jsonToCsmButton{};

// cap framerate. SDL timer counts in nanoseconds...
const Uint64 NANOSECONDS_PER_SECOND = 1000000000;
const Uint64 NANOSECONDS_PER_MILLISECOND = 1000000; // BUT THEN ALSO USES MILLISECONDS TO SLEEP??? WHY?????
const Uint64 TARGET_FRAME_DURATION_NS = NANOSECONDS_PER_SECOND / FPS;

SDL_Color textColor{ 0xFF, 0xFF, 0xFF, 0xFF };
SDL_Color backgroundColor{ 0xA0, 0x4D, 0xFF, 0xFF };

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
	SDL_SetAppMetadata("JSON2LCE", "1.1", "com.MattNL.json2lce");

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// Initialize TTF somewhere before using fonts
	if (TTF_Init() == -1) {
		// handle error
	}

	// Create an SDL_RWops from the memory buffer
	SDL_IOStream* io = SDL_IOFromConstMem(LATO_FONT, sizeof(LATO_FONT));
	if (!io) {
		SDL_Log("Failed to load font from memory: %s\n", SDL_GetError());
		// handle error
	}

	MayFont = TTF_OpenFontIO(io, true, 24);
	if (!MayFont) {
		SDL_Log("Failed to load font: %s\n", SDL_GetError());
	}

	if (!SDL_CreateWindowAndRenderer("MNL's Super Simple Skin Converter (:", WIDTH, HEIGHT, 0, &window, &renderer)) {
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	io = SDL_IOFromConstMem(APP_ICON, sizeof(APP_ICON));
	if (!io) {
		SDL_Log("Failed to load app icon from memory: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_Surface* iconSurface = SDL_LoadBMP_IO(io, true);
	if (!iconSurface) {
		SDL_Log("Failed to load app icon: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_SetWindowIcon(window, iconSurface);
	SDL_DestroySurface(iconSurface);

	Star::InitStars(stars);

	jsonToCsmButton = new Button(".JSON -> .CSM", MayFont, (WIDTH - 200) / 2, (HEIGHT - 80) / 2, 200, 80, []() {jsonOpenFile(window);});

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
	static Uint64 lastTime = SDL_GetTicksNS();

	SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
	SDL_RenderClear(renderer);

	// Update and draw stars
	for (const auto& star : stars)
	{
		star->update();
		star->render(renderer);
	}

	// Draw copyright text
	SDL_Surface* copyrightSurface =
		TTF_RenderText_Blended(MayFont, "MattNL (c) 2025", 0, textColor);
	SDL_Surface* versionSurface =
		TTF_RenderText_Blended(MayFont, "Version 1.1", 0, textColor);

	SDL_Texture* copyrightTexture = SDL_CreateTextureFromSurface(renderer, copyrightSurface);
	SDL_Texture* versionTexture = SDL_CreateTextureFromSurface(renderer, versionSurface);

	// Get width and height from surfaces
	int cw = copyrightSurface->w;
	int ch = copyrightSurface->h;
	int vw = versionSurface->w;
	int vh = versionSurface->h;

	SDL_FRect copyrightDST = { WIDTH - cw, HEIGHT - ch, (float)cw, (float)ch }; // bottom right
	SDL_FRect versionDST = { 0, HEIGHT - vh, (float)vw, (float)vh };             // bottom left

	SDL_RenderTexture(renderer, copyrightTexture, nullptr, &copyrightDST);
	SDL_RenderTexture(renderer, versionTexture, nullptr, &versionDST);

	SDL_DestroyTexture(copyrightTexture);
	SDL_DestroyTexture(versionTexture);

	jsonToCsmButton->render(renderer);

	SDL_RenderPresent(renderer);

	Uint64 currentTime = SDL_GetTicksNS();
	Uint64 elapsed = currentTime - lastTime;

	if (elapsed < TARGET_FRAME_DURATION_NS) {
		Uint32 delayMs = (Uint32)((TARGET_FRAME_DURATION_NS - elapsed) / NANOSECONDS_PER_MILLISECOND);
		if (delayMs > 0)
			SDL_Delay(delayMs);
	}

	lastTime = SDL_GetTicksNS(); // reset for next frame

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;

    jsonToCsmButton->handleEvent(event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	SDL_Log("Shutting down!");
}
