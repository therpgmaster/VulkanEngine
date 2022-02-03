#pragma once

#include "Core/Types/CommonTypes.h"
#include <iostream>

/* a virtual camera, this is you */
class Camera
{
public:
	Camera()
	{
	};
	Camera(const float& verticalFOV, const float& near, const float& far)
	{
		nearPlane = near;
		farPlane = far;
		setFOVh(verticalFOV);
	};

	Camera& operator=(const Camera&) = default;
	Camera& operator=(Camera&&) = default;
	bool operator==(Camera* comparePtr) const { return comparePtr == this; }

	Transform transform;

	/* camera settings */
	float nearPlane = 0.01f;
	float farPlane = 15.f;
	float vFOV = 45.f;
	float aspectRatio = 1.333f;

	void setFOVh(const float& deg) { vFOV = (float)Transform::degToRad((float)deg); }

	glm::mat4 getProjectionMatrix_legacy() 
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

	// returns a 3D projection matrix, consistent with a certain free and open source program "B"
	glm::mat4 getProjectionMatrix() 
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

	glm::mat4 getViewMatrix(const bool& legacyMethod = false) 
	{
		if (legacyMethod) 
		{
			// using the camera's transform directly
			return glm::inverse(transform.mat4());
		}

		const glm::vec3& position = transform.translation;
		const glm::vec3& rotation = transform.rotation;
		// calculate the view matrix based on rotation and position
		const float c3 = glm::cos(rotation.x);
		const float s3 = glm::sin(rotation.x);
		const float c2 = glm::cos(rotation.y);
		const float s2 = glm::sin(rotation.y);
		const float c1 = glm::cos(rotation.z);
		const float s1 = glm::sin(rotation.z);
		const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
		const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
		const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
		glm::mat4 viewMatrix = glm::mat4{ 1.f };
		viewMatrix[0][0] = u.x;
		viewMatrix[1][0] = u.y;
		viewMatrix[2][0] = u.z;
		viewMatrix[0][1] = v.x;
		viewMatrix[1][1] = v.y;
		viewMatrix[2][1] = v.z;
		viewMatrix[0][2] = w.x;
		viewMatrix[1][2] = w.y;
		viewMatrix[2][2] = w.z;
		viewMatrix[3][0] = -glm::dot(u, position);
		viewMatrix[3][1] = -glm::dot(v, position);
		viewMatrix[3][2] = -glm::dot(w, position);
		return viewMatrix;
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

	void moveInPlaneXY(const Vector2D<double>& lookInput, const float& moveFwd, const float& moveRight, const float& moveUp, const float& deltaTime)
	{
		float lookSpeed = 6.8f;
		float moveSpeed = 12.f;

		float yawInput = lookInput.x != 0 ? lookInput.x / abs(lookInput.x) : 0.f; 
		float pitchInput = lookInput.y != 0 ? lookInput.y / abs(lookInput.y) : 0.f;
		auto rotV = Vec{ 0.f, pitchInput, -yawInput };

		if (Vec::dot(rotV, rotV) > std::numeric_limits<float>::epsilon())
		{ transform.rotation += rotV.getNormalized() * lookSpeed * deltaTime; }
		
		// limit pitch values to exactly 85 degrees
		transform.rotation.y = glm::clamp(transform.rotation.y, 
				(float)Transform::degToRad(-85.f), (float)Transform::degToRad(85.f));
		// prevent overflow from continous yawing
		transform.rotation.z = glm::mod(transform.rotation.z, glm::two_pi<float>());

		const Vec forwardDir = transform.getForwardVector();
		const Vec rightDir = Vec{ forwardDir.y, -forwardDir.x, 0.f };
		const Vec upDir{ 0.f, 0.f, 1.f };

		Vec moveDir{ 0.f };
		if (moveFwd > 0.f) { moveDir += forwardDir; }
		else if (moveFwd < 0.f) { moveDir -= forwardDir; }

		if (moveRight > 0.f) { moveDir += rightDir; }
		else if (moveRight < 0.f) { moveDir -= rightDir; }

		if (moveUp > 0.f) { moveDir += upDir; }
		else if (moveUp < 0.f) { moveDir -= upDir; }

		transform.translation += moveDir * moveSpeed * deltaTime;
	}

	static glm::mat4 getWorldBasisMatrix() 
	{
		/*	identity matrix rearranged to swap axes 
		*	resulting in a right-handed z-up/x-forward basis:
		*	-y (up) becomes z, x (right) becomes -y, z (forward) becomes x */
		glm::mat4 basis{ 0.f };
		basis[1][0] = -1.f; // x moved to y (row 2)
		basis[2][1] = 1.f; // y moved to z (row 3)
		basis[0][2] = -1.f; // z moved to x (row 1)
		basis[3][3] = 1.f; // always 1
		return basis;
	}

};
