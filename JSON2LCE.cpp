#include "JSON2LCE.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#define SDL_MAIN_USE_CALLBACKS 1

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

#define WIDTH	512
#define HEIGHT	512
#define FPS		60

const float STAR_SIZE = 50;
const float STAR_SPIN_SPEED = 0.0125;
const float STAR_SPEED = 1.5f;
const float X_SPACE = STAR_SIZE * 2.5f;
const float Y_SPACE = STAR_SIZE * 2.5f;

struct Star {
	float baseX, x, y;
	double angle;
};

std::vector<Star> stars;

TTF_Font* MayFont{}; // yeah so what if I'm keeping the name lol

// cap framerate. SDL timer counts in nanoseconds
const Uint64 NANOSECONDS_PER_SECOND = 1000000000;
const Uint64 NANOSECONDS_PER_MILLISECOND = 1000000;
const Uint64 TARGET_FRAME_DURATION_NS = NANOSECONDS_PER_SECOND / FPS;

SDL_Color textColor{ 0xFF, 0xFF, 0xFF, 0xFF };
SDL_Color backgroundColor{ 0xA0, 0x4D, 0xFF, 0xFF };
// why SDL Vertexes need FColor instead of Color is beyond me, but it makes me not happy lol
SDL_FColor starsColor{
	float(0xC7) / 255.0f,
	float(0x9E) / 255.0f,
	float(0xF0) / 255.0f,
	float(0xFF) / 255.0f
};

const double PI = 3.141592653589793;
void CreateStarPoints(float cx, float cy, float size, float angle, SDL_FPoint* outPoints)
{
	for (int i = 0; i < 10; ++i) {
		float a = angle + i * (PI / 5);
		float r = (i % 2 == 0) ? size : size * 0.5f;
		outPoints[i].x = cx + cosf(a) * r;
		outPoints[i].y = cy + sinf(a) * r;
	}
}

void RenderFilledStar(float cx, float cy, float size, float angle)
{
	SDL_FPoint points[10];
	CreateStarPoints(cx, cy, size, angle, points);

	SDL_Vertex verts[11]; // center + 10 outer points
	int centerIndex = 10;

	verts[centerIndex].position.x = cx;
	verts[centerIndex].position.y = cy;
	verts[centerIndex].color = starsColor;

	for (int i = 0; i < 10; ++i)
	{
		verts[i].position = points[i];
		verts[i].color = starsColor;
	}

	// Create index buffer for triangles (fan-style)
	SDL_Texture* tex = nullptr;
	for (int i = 0; i < 10; ++i)
	{
		const int indices[3] = {
			centerIndex,
			i,
			(i + 1) % 10
		};
		SDL_RenderGeometry(renderer, tex, verts, 11, indices, 3);
	}
}

int cols = (int)(WIDTH / X_SPACE);
int rows = (int)(HEIGHT / Y_SPACE);

void InitStars() {
	stars.clear();

	float totalStarsWidth = X_SPACE * (cols - 1);
	float startXOffset = (WIDTH - totalStarsWidth) / 2;

	const float secondRowOffset = STAR_SIZE;
	const int loopRows = rows + 2;   // +2 for seamless background looping
	const int loopCols = cols + 2;

	for (int row = -1; row < loopRows - 1; ++row) {
		for (int col = -1; col < loopCols - 1; ++col) {
			float xOffset = ((row + 1) % 2 == 0) ? secondRowOffset : 0;
			float baseX = startXOffset + col * X_SPACE + xOffset;
			float y = Y_SPACE * (row + 1) - STAR_SIZE;
			stars.push_back({ baseX, baseX, y, 0.0 });
		}
	}
}

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

	MayFont = TTF_OpenFontIO(io, true, 32);
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

	InitStars();

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
	static Uint64 lastTime = SDL_GetTicksNS();

	SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
	SDL_RenderClear(renderer);

	for (int i = 0; i < stars.size(); ++i) {
		stars[i].x -= STAR_SPEED;
		stars[i].y += STAR_SPEED;

		// Wrap horizontally
		if (stars[i].x < -STAR_SIZE)
			stars[i].x += (cols + 2) * X_SPACE;

		// Wrap vertically
		if (stars[i].y > HEIGHT + STAR_SIZE)
			stars[i].y -= (rows + 2) * Y_SPACE;

		stars[i].angle += STAR_SPIN_SPEED;
		if (stars[i].angle > 2 * PI)
			stars[i].angle -= 2 * PI;
	}

	// Draw stars
	for (const auto& star : stars)
	{
		RenderFilledStar(star.x, star.y, STAR_SIZE, star.angle);
	}

	// Draw copyright text
	SDL_Surface* copyrightSurface =
		TTF_RenderText_Blended(MayFont, "MattNL (c) 2025", 0, textColor);
	SDL_Surface* versionSurface =
		TTF_RenderText_Blended(MayFont, "Version 1.0", 0, textColor);

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
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	SDL_Log("Shutting down!");
}
