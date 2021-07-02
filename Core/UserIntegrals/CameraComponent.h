#pragma once

#include "ActorComponent.h"

// a virtual camera, not necessarily associated with an actor, thus owner might be null
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
		vFOV = verticalFOV;
	};

	CameraComponent& operator=(const CameraComponent&) = default;
	CameraComponent& operator=(CameraComponent&&) = default;
	bool operator==(CameraComponent* comparePtr) const { return comparePtr == this; }

	/* camera settings */
	float nearPlane = 0.01f;
	float farPlane = 10.f;
	float vFOV = 45.f;
	float aspectRatio = 1.333f;
};
