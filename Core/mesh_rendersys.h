#pragma once

#include "engine_device.h"
#include "engine_pipeline.h"

// glm
#include <glm/gtc/matrix_transform.hpp>
// std
#include <memory>
#include <vector>
#include <cmath> // only used in perspective calculation

class CameraComponent;
#include "UserIntegrals/StaticMesh.h"

namespace EngineCore
{
	/* a render system manages a pipeline and the associated shaders */
	class MeshRenderSystem
	{
	public:

		MeshRenderSystem(EngineDevice& deviceIn, VkRenderPass renderPass);
		~MeshRenderSystem();

		MeshRenderSystem(const MeshRenderSystem&) = delete;
		MeshRenderSystem& operator=(const MeshRenderSystem&) = delete;

		void renderMeshes(VkCommandBuffer commandBuffer, std::vector<StaticMesh*>& meshes,
							CameraComponent* camera, const float& deltaTimeSeconds, float time,
							InputSystem* inputSysPtr);

	private:
		//void createPipelineLayout();
		//void createPipeline(VkRenderPass renderPass, const char* vertexShaderPath, const char* fragmentShaderPath);

		EngineDevice& device;
		//VkPipelineLayout pipelineLayout;

		static glm::mat4 orthographicMatrix(const float& n, const float& f)
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

		// TODO: not really necessary, so far
		static glm::mat4 scaleMatrix(float s)
		{
			glm::vec3 v = { s, s, s };
			return glm::mat4
			{
				v.x,	0.f,	0.f,	0.f,
				0.f,	v.y,	0.f,	0.f,
				0.f,	0.f,	v.z,	0.f,
				0.f,	0.f,	0.f,	1.f,
			};
		}

		static glm::mat4 lerpMat4(float t, glm::mat4 matA, glm::mat4 matB);

		static float lerp(const double& a, const double& b, const double& t)
		{
			return (a * (1.0 - t)) + (b * t);
		}

	};

} // namespace