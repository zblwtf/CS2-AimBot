#include "vector.h"
class CView
{
public:
	float Matrix[4][4]{};

	bool WorldToScreen(const Vector_t& Pos, Vector2D_t& ToPos,unsigned int window_width,unsigned int window_height)
	{
		float View = 0.f;
		float SightX = window_width / 2.0f, SightY = window_height / 2.0f;

		View = Matrix[3][0] * Pos.x + Matrix[3][1] * Pos.y + Matrix[3][2] * Pos.z + Matrix[3][3];

		if (View <= 0.01)
			return false;

		ToPos.x = SightX + (Matrix[0][0] * Pos.x + Matrix[0][1] * Pos.y + Matrix[0][2] * Pos.z + Matrix[0][3]) / View * SightX;
		ToPos.y = SightY - (Matrix[1][0] * Pos.x + Matrix[1][1] * Pos.y + Matrix[1][2] * Pos.z + Matrix[1][3]) / View * SightY;

		return true;
	}
};