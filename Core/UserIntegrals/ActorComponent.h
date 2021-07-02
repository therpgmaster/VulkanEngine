#pragma once

#include "GenericDataObject.h"
// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class GenericActor;

// simple physical transform data
struct Transform3D
{
public:
	glm::vec3 translation;
	glm::vec3 scale{ 1.f, 1.f, 1.f };
	glm::vec3 rotation{};
	// Matrix corresponds to Translate * Ry * Rx * Rz * Scale
	// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
	// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	glm::mat4 mat4()
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		return glm::mat4
		{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{translation.x, translation.y, translation.z, 1.0f} };
	}
	glm::mat4 mat4NatSys()
	{
		// Matrix corresponds to Translate * Rz * Rx * Ry (different rotation order)
		// Rotations corresponds to Tait-bryan angles of Z(1), X(2), Y(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		// col 0
		const float c0r0 = c1 * c3 - s1 * s2 * s3;
		const float c0r1 = c3 * s1 + c1 * s2 * s3;
		const float c0r2 = -c2 * s3;
		// col 1
		const float c1r0 = -c2 * s1;
		const float c1r1 = c1 * c2;
		const float c1r2 = s2;
		// col 2
		const float c2r0 = c1 * s3 + c3 * s1 * s2;
		const float c2r1 = s1 * s3 - c1 * c3 * s2;
		const float c2r2 = c2 * c3;

		return glm::mat4
		{
			c0r0,	c1r0,	c2r0,	translation.x,
			c0r1,	c1r1,	c2r1,	translation.y,
			c0r2,	c1r2,	c2r2,	translation.z,
			0.f,	0.f,	0.f,	1.f,
		};
	}

	// quick and dirty way to test movement in all six directions
	void testTranslation(const float& time, const float& maxDistance, const float& delta)
	{
		glm::vec3 v{};
		int t = (int)time % 7;
		if (t <= 1)
		{
			v = { 1.f,0.f,0.f };
		}
		else if (t <= 2)
		{
			v = { -1.f,0.f,0.f };
		}
		else if (t <= 3)
		{
			v = { 0.f,1.f,0.f };
		}
		else if (t <= 4)
		{
			v = { 0.f,-1.f,0.f };
		}
		else if (t <= 5)
		{
			v = { 0.f,0.f,1.f };
		}
		else
		{
			v = { 0.f,0.f,-1.f };
		}
		v = v * maxDistance;
		translation = translation + (v * maxDistance * delta);
	}
};

class ActorComponent : public GenericDataObject
{
public:
	ActorComponent() {};
	~ActorComponent();

	ActorComponent& operator=(const ActorComponent&) = default;
	ActorComponent& operator=(ActorComponent&&) = default;
	bool operator==(ActorComponent* comparePtr) const { return comparePtr == this; }

	/* must be called immediately when the component is created */
	void setParentActor(GenericActor& newParentActor) { parentActor = &newParentActor; }

	/* attempts to make this component standalone, returns false if unsuccessful 
	components already assigned to an actor cannot be orphaned */
	bool setNoParent();

	GenericActor* getParentActor() { return parentActor; }

	/* used by parent actor to decide if this component has user defined logic to run on tick */
	bool getComponentCanTick() const { return canTick; }

	bool getComponentHasPhysicalPresence() const { return hasPhysicalPresence; }

	bool getComponentHasOwner() const { return hasOwner; }

	Transform3D transform{};

	virtual void tick(const float& deltaTime) {};

protected:
	/* if false, parent actor will not attempt to call user defined tick functions on this component */
	bool canTick = true;
	/* should be true if the component type has any primitives which can be visually rendered */
	bool hasPhysicalPresence = false;

private:
	/* if hasOwner is false, the parentActor pointer is allowed to remain null */
	GenericActor* parentActor = nullptr;
	bool hasOwner = true;

	

	
};
