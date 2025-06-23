#include "Star.h"

#define WIDTH	512
#define HEIGHT	512

const double PI = 3.141592653589793; // yummy :3

const float STAR_SIZE = 50;
const float STAR_SPIN_SPEED = 0.0125;
const float STAR_SPEED = 1.5f;
const float X_SPACE = STAR_SIZE * 2.5f;
const float Y_SPACE = STAR_SIZE * 2.5f;
const int cols = (int)(WIDTH / X_SPACE);
const int rows = (int)(HEIGHT / Y_SPACE);

// why SDL Vertexes need SDL_FColor instead of SDL_Color when literally everything else uses SDL_Color is beyond me, but it makes me not happy lol
SDL_FColor starsColor{
	float(0xC7) / 255.0f,
	float(0x9E) / 255.0f,
	float(0xF0) / 255.0f,
	float(0xFF) / 255.0f
};

Star::Star(float x, float y) :
	mInitialX{ x },
	mPosX{ x },
	mPosY{ y },
	mAngle{ 0.0f }
{

}

void Star::render(SDL_Renderer* renderer)
{
		SDL_FPoint points[10];
		CreateStarPoints(mPosX, mPosY, mAngle, points);

		SDL_Vertex verts[11]; // center + 10 outer points
		int centerIndex = 10;

		verts[centerIndex].position.x = mPosX;
		verts[centerIndex].position.y = mPosY;
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

void Star::update()
{
	mPosX -= STAR_SPEED;
	mPosY += STAR_SPEED;

	// Wrap horizontally
	if (mPosX < -STAR_SIZE)
		mPosX += (cols + 2) * X_SPACE;

	// Wrap vertically
	if (mPosY > HEIGHT + STAR_SIZE)
		mPosY -= (rows + 2) * Y_SPACE;

	mAngle += STAR_SPIN_SPEED;
	if (mAngle > 2 * PI)
		mAngle -= 2 * PI;
}

void Star::InitStars(std::vector<Star*>& stars)
{
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
			stars.push_back(new Star(baseX, y));
		}
	}
}

void Star::CreateStarPoints(float x, float y, float angle, SDL_FPoint* outPoints)
{
	for (int i = 0; i < 10; ++i) {
		float a = angle + i * (PI / 5);
		float r = (i % 2 == 0) ? STAR_SIZE : STAR_SIZE * 0.5f;
		outPoints[i].x = x + cosf(a) * r;
		outPoints[i].y = y + sinf(a) * r;
	}
}