#pragma once

#include "ActorComponent.h"

#include <iostream>

/* a virtual camera, not necessarily associated with an actor, thus owner might be null */
class CameraComponent : public ActorComponent
{
public:
	CameraComponent()
	{
		setNoParent(); // cameras do not always need parent actors
	};
	CameraComponent(const float& verticalFOV, const float& near, const float& far)
	{
		setNoParent();
		nearPlane = near;
		farPlane = far;
		setFOVh(verticalFOV);
	};

	CameraComponent& operator=(const CameraComponent&) = default;
	CameraComponent& operator=(CameraComponent&&) = default;
	bool operator==(CameraComponent* comparePtr) const { return comparePtr == this; }

	/* camera settings */
	float nearPlane = 0.01f;
	float farPlane = 15.f;
	float vFOV = 45.f;
	float aspectRatio = 1.333f;

	void setFOVh(const float& deg) { vFOV = (float)Transform3D::degToRad((float)deg); }

	glm::mat4 getProjectionMatrix() 
	{
		/*
		float left = -aspectRatio;
		float right = aspectRatio;
		float top = -1.f;
		float bottom = 1.f;

		float Xdelta = right - left;
		float Ydelta = top - bottom;
		float Zdelta = farPlane - nearPlane;

		glm::mat4 mat{0.f};
		mat[0][0] = nearPlane * 2.f / Xdelta;
		mat[1][1] = nearPlane * 2.f / Ydelta;
		mat[2][0] = (right + left) / Xdelta; // note: negate Z
		mat[2][1] = (top + bottom) / Ydelta;
		mat[2][2] = -(farPlane + nearPlane) / Zdelta;
		mat[2][3] = -1.f;
		mat[3][2] = (-2.f * nearPlane * farPlane) / Zdelta;
		return mat;
		*/

		// lve: up = -Y,  right = +X,  forward = +Z
		const float tanHalfFovy = tan(vFOV / 2.f);
		float x = 1.f / (aspectRatio * tanHalfFovy);
		float y = 1.f / (tanHalfFovy);
		float z = farPlane / (farPlane - nearPlane);
		float w = -(farPlane * nearPlane) / (farPlane - nearPlane);

		glm::mat4 lveMat
		{
			x,   0.f, 0.f, 0.f,
			0.f, y,   0.f, 0.f,
			0.f, 0.f, z,   1.f,
			0.f, 0.f, w,   0.f,
		};

		return lveMat;
	}

	glm::mat4 getProjectionMatrixBlender() 
	{
		// for X-forward Z-up: rotate camera 90 deg counter-clockwise on Y, 90 deg clockwise on X
		const float b = nearPlane * tan(vFOV / 2);
		const float X_d = (aspectRatio * b) * 2;
		const float Y_d = -b * 2;
		const float Z_d = farPlane - nearPlane;

		const float x = nearPlane * 2.f / X_d;
		const float y = nearPlane * 2.f / Y_d;
		const float A = -(farPlane + nearPlane) / Z_d;
		const float B = (-2.f * nearPlane * farPlane) / Z_d;

		glm::mat4 mat
		{
			x,   0.f, 0.f,  0.f, // X
			0.f, y,   0.f,  0.f, // Y
			0.f, 0.f, A,    B,   // Z
			0.f, 0.f, -1.f, 0.f, // w
		};
		return mat;
	}

	glm::mat4 getViewMatrix(const bool& inverse = true)
	{
		if (inverse) { return glm::inverse(transform.mat4()); }
		else { return transform.mat4(); }
	}

	glm::mat4 getProjectionMatrixAlt()
	{
		const float n = nearPlane;
		const float f = farPlane;
		const float ar = aspectRatio;
		const float vFovTanh = tan(vFOV / 2.f);

		/*
		glm::mat4 pmatrix{ 0.f };

		// x scale
		pmatrix[0][0] = 1.f / (ar * vFovTanh);
		// y scale
		pmatrix[1][1] = 1.f / (vFovTanh);
		// z scale
		pmatrix[2][2] = f / (f - n);
		// w col 2
		pmatrix[2][3] = 1.f;
		// "forward" translation
		pmatrix[3][2] = -(f * n) / (f - n);

		glm::mat4 pmatrix - same as above, visualized
		{
			x,   0.f, 0.f, 0.f,
			0.f, y,   0.f, 0.f,
			0.f, 0.f, A,   1.f,
			0.f, 0.f, B,   0.f,
		};
		*/

		float x = 1.f / (ar * vFovTanh);
		float y = 1.f / (vFovTanh);
		float A = f / (f - n);
		float B = -(f * n) / (f - n);

		

		glm::mat4 pmatrix
		{
			A,   0.f, 0.f, 1.f, // X
			0.f, -x,  0.f, 0.f, // Y
			0.f, 0.f, -y,  0.f, // Z
			B,   0.f, 0.f, 0.f, // w
		};

		return pmatrix;
	}

	// alternate projection method (flips an axis, mismatch with blender)
	glm::mat4 getProjectionMatrixCookbook()
	{
		const float n = nearPlane;
		const float f = farPlane;
		const float a = aspectRatio;
		float x = 1.f / tan(glm::radians(vFOV / 2.f));
		//	"cookbook" projection 
		glm::mat4 pmatrix =
		{
			x / a, 0.0f, 0.0f, 0.0f,
			0.0f, x, 0.0f, 0.0f, /* [1][1]: x for y=down (default), -x for y=up */
			0.0f, 0.0f, f / (n - f), -1.0f,
			0.0f, 0.0f, -(f * n) / (f - n), 1.0f 
		};
		return pmatrix;
	}

};
