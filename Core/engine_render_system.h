#pragma once

#include "engine_device.h"
#include "engine_object.h"
#include "engine_pipeline.h"

// std
#include <memory>
#include <vector>
#include <cmath> // only used in perspective calculation

namespace EngineCore
{
	class Camera
	{
	public:
		Camera(const float& verticalFOV, const float& near, const float& far)
		{
			nearPlane = near;
			farPlane = far;
			vFOV = verticalFOV;
		};
		float nearPlane = 0.01f;
		float farPlane = 10.f;
		float vFOV = 45.f;
		float aspectRatio = 1.333f;
		TransformComponent transform{};
	};

	class EngineRenderSystem
	{
	public:

		EngineRenderSystem(EngineDevice& deviceIn, VkRenderPass renderPass);
		~EngineRenderSystem();

		EngineRenderSystem(const EngineRenderSystem&) = delete;
		EngineRenderSystem& operator=(const EngineRenderSystem&) = delete;

		void renderEngineObjects(VkCommandBuffer commandBuffer, std::vector<EngineObject>& engineObjects,
							Camera* camera, const float& deltaTimeSeconds, float time,
							std::vector<bool> wasdrf);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		EngineDevice& device;
		std::unique_ptr<EnginePipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		glm::mat4 orthographicMatrix(const float& n, const float& f)
		{
			float r = 1.f;
			float l = -1.f;
			float t = -1.f;
			float b = 1.f;

			float x1 = 2 / (r - l);
			float x2 = -(r + l) / (r - l);
			float x3 = 2 / (b - t); /* swap b and t for y axis up (default is y = down, b-t) */
			float x4 = -(b + t) / (b - t);
			float x5 = 1 / (f - n);
			float x6 = -n / (f - n);

			glm::mat4 projection 
			{
				x1,		0.f,	0.f,	x2,
				0.f,	x3,		0.f,	x4,
				0.f,	0.f,	x5,		x6,
				0.f,	0.f,	0.f,	1.f, 
			};

			return projection;
		}

		glm::mat4 perspectiveMatrix(float aspectRatio, float fov, float n, float f) 
		{
			float x = 1.0f / tan(glm::radians(0.5f * fov));
			//	"cookbook" projection 
			glm::mat4 perspectiveMatrix =
			{ 
				x / aspectRatio, 0.0f, 0.0f, 0.0f,
				0.0f, x, 0.0f, 0.0f, /* [1][1]: x for y=down (default), -x for y=up */
				0.0f, 0.0f, f / (n - f), -1.0f,
				0.0f, 0.0f, (n * f) / (n - f), 1.0f // [3][3]: (last) changed from 0 to 1
			};
			return perspectiveMatrix;
		}

		glm::mat4 wsToVulkanMatrix()
		{
			// produces a matrix for converting vectors between coordinate systems
			glm::vec3 X = { 0.f, 1.f, 0.f }; // vulkan x = world y
			glm::vec3 Y = { 0.f, 0.f, -1.f }; // vulkan y = world -z
			glm::vec3 Z = { 1.f, 0.f, 0.f }; // vulkan z = world x
			// last column at zero leaves relative position unchanged
			return glm::mat4 /* invert row to flip axis */
			{
				-X.x,	-Y.x,	-Z.x,	0.f,
				X.y,	Y.y,	Z.y,	0.f,
				X.z,	Y.z,	Z.z,	0.f,
				0.f,	0.f,	0.f,	1.f, 
			};
		}

		glm::mat4 worldToVulkan()
		{
			// produces a matrix for converting vectors between coordinate systems
			glm::vec3 X = { 0.f, 1.f, 0.f }; // vulkan x = world y
			glm::vec3 Y = { 0.f, 0.f, -1.f }; // vulkan y = world -z
			glm::vec3 Z = { 1.f, 0.f, 0.f }; // vulkan z = world x
			// last column at zero leaves relative position unchanged
			return glm::mat4 /* invert row to flip axis */
			{
				-X.x,	-Y.x,	-Z.x,	0.f,
				X.y,	Y.y,	Z.y,	0.f,
				X.z,	Y.z,	Z.z,	0.f,
				0.f,	0.f,	0.f,	1.f,
			};
		}

		glm::mat4 lerpMat4(float t, glm::mat4 matA, glm::mat4 matB);

		float lerp(const double& a, const double& b, const double& t)
		{
			return (a * (1.0 - t)) + (b * t);
		}
	};

} // namespace