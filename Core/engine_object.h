#pragma once

#include "engine_model.h"

// std
#include <memory>
// glm
#include <glm/gtc/matrix_transform.hpp>

namespace EngineCore 
{
	struct TransformComponent
	{
		glm::vec3 translation;
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};
		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
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
			// Matrix corresponds to Translate * Rz * Rx * Ry
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

	class EngineObject 
	{
	public:
		using id_t = unsigned int;

		EngineObject(const EngineObject&) = delete;
		EngineObject& operator=(const EngineObject&) = delete;
		EngineObject(EngineObject&&) = default;
		EngineObject& operator=(EngineObject&&) = default;

		static EngineObject createObject() 
		{
			static id_t currentId = 0;
			return EngineObject{currentId++};
		}

		const id_t getId() { return id; }

		std::shared_ptr<EngineModel> model{};
		glm::vec3 color;
		TransformComponent transform{};

	private:
		EngineObject(id_t objId) : id{objId} {}
		id_t id;
	};
}
