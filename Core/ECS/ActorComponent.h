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
	
	glm::mat4 mat4()
	{
		return  makeMatrix(rotation, scale, translation); 
	}

	glm::mat4 makeMatrix(const glm::vec3& rotationIn, const glm::vec3& scaleIn, const glm::vec3& translationIn = { 0.f, 0.f, 0.f })
	{
		/* returns Translation * Rz * Ry * Rx * Scale
		* Tait-bryan angles Z(1)-Y(2)-X(3) rotation order 
		* https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix */
		const float c3 = glm::cos(rotationIn.x);
		const float s3 = glm::sin(rotationIn.x);
		const float c2 = glm::cos(rotationIn.y);
		const float s2 = glm::sin(rotationIn.y);
		const float c1 = glm::cos(rotationIn.z);
		const float s1 = glm::sin(rotationIn.z);
		return glm::mat4
		{
			{
				scaleIn.x * (c1 * c2),
				scaleIn.x * (c2 * s1),
				scaleIn.x * (-s2),
				0.0f,
			},
			{
				scaleIn.y * (c1 * s2 * s3 - c3 * s1),
				scaleIn.y * (c1 * c3 + s1 * s2 * s3),
				scaleIn.y * (c2 * s3),
				0.0f,
			},
			{
				scaleIn.z * (s1 * s3 + c1 * c3 * s2),
				scaleIn.z * (c3 * s1 * s2 - c1 * s3),
				scaleIn.z * (c2 * c3),
				0.0f,
			},
			{translationIn.x, translationIn.y, translationIn.z, 1.0f}
		};

		// old Tait-bryan angles Y(1)-X(2)-Z(3) rotation order, for vk space (-y up, x right, z forward)
		/*
		const float c3 = glm::cos(rotationIn.z);
		const float s3 = glm::sin(rotationIn.z);
		const float c2 = glm::cos(rotationIn.x);
		const float s2 = glm::sin(rotationIn.x);
		const float c1 = glm::cos(rotationIn.y);
		const float s1 = glm::sin(rotationIn.y);
		return glm::mat4
		{
			{
				scaleIn.x * (c1 * c3 + s1 * s2 * s3),
				scaleIn.x * (c2 * s3),
				scaleIn.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scaleIn.y * (c3 * s1 * s2 - c1 * s3),
				scaleIn.y * (c2 * c3),
				scaleIn.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scaleIn.z * (c2 * s1),
				scaleIn.z * (-s2),
				scaleIn.z * (c1 * c2),
				0.0f,
			},
			{translationIn.x, translationIn.y, translationIn.z, 1.0f} 
		};
		*/
	}

	glm::mat4 makeMatrix(const glm::vec3& rotationIn) 
	{ 
		return makeMatrix(rotationIn, { 1.f, 1.f, 1.f }); 
	}

	glm::vec3 getForwardVector() 
	{ 
		return { sin(rotation.y), 0.f, cos(rotation.y) };
		//return rotateVector(quaternionFromRotation(rotation), glm::vec3{ 1.f, 0.f, 0.f });
	}

	static double degToRad(const double& degrees) { return (degrees * 0.01745329251); }

	static glm::vec3 rotateVector(const glm::vec4& rotQuat, const glm::vec3& v3) 
	{
		const glm::vec4 v4 = { v3.x, v3.y, v3.z, 0.f };
		glm::vec4 qw = { rotQuat.w, rotQuat.w, rotQuat.w, rotQuat.w };
		// cross product of vector and quat-rotation, times 2
		glm::vec4 t = crossVec4(rotQuat, v4);
		t = t + t;
		glm::vec4 r = ((qw * t) + v4) + crossVec4(rotQuat, t);
		return { r.x, r.y, r.z };
	}

	// replacement for missing overload of cross() that accepts 4-component vectors (w=0)
	static glm::vec4 crossVec4(const glm::vec4& a, const glm::vec4& b) 
	{
		const glm::vec3 a3 = { a.x, a.y, a.z };
		const glm::vec3 b3 = { b.x, b.y, b.z };
		glm::vec3 p = glm::cross(a3, b3);
		return glm::vec4{ p.x, p.y, p.z, 0.f };
	}

	// assumes yaw = z, pitch = y, roll = x
	static glm::vec4 quaternionFromRotation(const glm::vec3& v)
	{
		// abbreviations for the angular functions
		const auto cy = cos(v.z); // in original formula all components were * 0.5 (halved)
		const auto sy = sin(v.z);
		const auto cp = cos(v.y);
		const auto sp = sin(v.y);
		const auto cr = cos(v.x);
		const auto sr = sin(v.x);

		glm::vec4 q{};
		q.w = cr * cp * cy + sr * sp * sy;
		q.x = sr * cp * cy - cr * sp * sy;
		q.y = cr * sp * cy + sr * cp * sy;
		q.z = cr * cp * sy - sr * sp * cy;

		return q;
	}

	// TODO: the below function is probably not used for anything
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
