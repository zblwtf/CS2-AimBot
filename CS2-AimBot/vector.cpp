#include "vector.h"

#include "matrix.h"
#include "qangle.h"

// used: m_rad2deg
#include "math.h"
extern int screenWidth;
extern int screenHeight;
[[nodiscard]] Vector_t Vector_t::Transform(const Matrix3x4_t& matTransform) const
{
	return {
		this->DotProduct(matTransform[0]) + matTransform[0][3],
		this->DotProduct(matTransform[1]) + matTransform[1][3],
		this->DotProduct(matTransform[2]) + matTransform[2][3]
	};
}

[[nodiscard]] QAngle_t Vector_t::ToAngles() const
{
	float flPitch, flYaw;
	if (this->x == 0.0f && this->y == 0.0f)
	{
		flPitch = (this->z > 0.0f) ? 270.f : 90.f;
		flYaw = 0.0f;
	}
	else
	{
		flPitch = M_RAD2DEG(std::atan2f(-this->z, this->Length2D()));

		if (flPitch < 0.f)
			flPitch += 360.f;

		flYaw = M_RAD2DEG(std::atan2f(this->y, this->x));

		if (flYaw < 0.f)
			flYaw += 360.f;
	}

	return { flPitch, flYaw, 0.0f };
}

[[nodiscard]] Matrix3x4_t Vector_t::ToMatrix() const
{
	Vector_t vecRight = {}, vecUp = {};
	this->ToDirections(&vecRight, &vecUp);

	Matrix3x4a_t matOutput = {};
	matOutput.SetForward(*this);
	matOutput.SetLeft(-vecRight);
	matOutput.SetUp(vecUp);
	return matOutput;
}

Vector_t Vector_t::WTS(view_matrix_t matrix) const
{
	float _x = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z + matrix[0][3];
	float _y = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z + matrix[1][3];

	float w = matrix[3][0] * x + matrix[3][1] * y + matrix[3][2] * z + matrix[3][3];

	if (w < 0.01f)
		return { 0,0,0 };

	float inv_w = 1.f / w;
	_x *= inv_w;
	_y *= inv_w;

	float nx = screenWidth / 2;
	float ny = screenHeight / 2;

	nx += 0.5f * _x * screenWidth + 0.5f;
	ny -= 0.5f * _y * screenHeight + 0.5f;

	return { nx,ny,w };
}