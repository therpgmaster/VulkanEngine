#include "mesh_rendersys.h"

#include "UserIntegrals/CameraComponent.h"

#include <stdexcept>
#include <array>
#include <iostream> // temporary

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace EngineCore
{
	struct SimplePushConstantData
	{
		glm::mat4 transform{ 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color; // data to gpu must be aligned to multiples of 16
	};

	MeshRenderSystem::MeshRenderSystem(EngineDevice& deviceIn, VkRenderPass renderPass) : device{deviceIn}
	{
		//createPipelineLayout();
		//createPipeline(renderPass);
	}

	MeshRenderSystem::~MeshRenderSystem()
	{
		//vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void MeshRenderSystem::renderMeshes(VkCommandBuffer commandBuffer, std::vector<StaticMesh*>& meshes,
		CameraComponent* camera, const float& deltaTimeSeconds, float time, InputSystem* inputSysPtr)
	{
		for (auto* mesh : meshes)
		{
			if (mesh == nullptr || mesh->getMaterial() == nullptr) continue;
			
			mesh->getMaterial()->bindToCommandBuffer(commandBuffer); // bind material-specific shading pipeline

			SimplePushConstantData push{};

			push.color = mesh->color;
			// spin 3D primitive
			float spinRate = 0.0f;
			mesh->transform.rotation.y = glm::mod(mesh->transform.rotation.y + (spinRate * deltaTimeSeconds), glm::two_pi<float>());
			mesh->transform.rotation.x = glm::mod(mesh->transform.rotation.x + ((spinRate / 2.f) * deltaTimeSeconds), glm::two_pi<float>());

			if (camera == nullptr) { throw std::runtime_error("renderEngineObjects null camera pointer"); }
			// camera rotation
			float mdx = inputSysPtr->getMouseDelta().x;
			float mdy = inputSysPtr->getMouseDelta().y;
			glm::vec3 mrd{ 0.f, mdy, mdx };
			camera->transform.rotation += (mrd * 0.001f);
			// camera translation
			glm::vec3 insysVec{ inputSysPtr->getAxisValue(0), -inputSysPtr->getAxisValue(1), inputSysPtr->getAxisValue(2) };
			camera->transform.translation += insysVec * 85.f * deltaTimeSeconds;

			// the view matrix and mesh transform matrix are in world space
			glm::mat4 projectionMatrix = camera->getProjectionMatrix();
			glm::mat4 viewMatrix = camera->getViewMatrix();
			glm::mat4 meshMatrix =  mesh->transform.mat4();
			push.transform = projectionMatrix * viewMatrix * meshMatrix;

			vkCmdPushConstants(commandBuffer, mesh->getMaterial()->getPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);

			// record mesh draw command
			mesh->bind(commandBuffer);
			mesh->draw(commandBuffer);
		}
	}

	/*void MeshRenderSystem::createPipelineLayout()
	{
		VkPushConstantRange pushConstRange{};
		pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstRange.offset = 0;
		pushConstRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1; // push constant
		pipelineLayoutInfo.pPushConstantRanges = &pushConstRange;
		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create pipeline layout"); }
	}

	void MeshRenderSystem::createPipeline(VkRenderPass renderPass, const char* vertexShaderPath, const char* fragmentShaderPath)
	{
		assert(pipelineLayout != nullptr && "tried to create pipeline with uninitialized layout");
		PipelineConfig pipelineConfig{};
		EnginePipeline::defaultPipelineConfig(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<EnginePipeline>(device, pipelineConfig,
			"G:/VulkanDev/VulkanEngine/Core/Shaders/shader.vert.spv",
			"G:/VulkanDev/VulkanEngine/Core/Shaders/shader.frag.spv"); // read SPIR-V shader files
	}

	void MeshRenderSystem::renderMeshes(VkCommandBuffer commandBuffer, std::vector<StaticMesh*>& meshes,
										CameraComponent* camera, const float& deltaTimeSeconds, float time, std::vector<bool> wasdrf)
	{
		pipeline->bind(commandBuffer);

		for (auto* mesh : meshes)
		{
			if (mesh == nullptr) continue;

			SimplePushConstantData push{};

			push.color = mesh->color;
			// spin 3D primitive
			float spinRate = 0.25f;
			mesh->transform.rotation.y = glm::mod(mesh->transform.rotation.y + (spinRate * deltaTimeSeconds), glm::two_pi<float>());
			mesh->transform.rotation.x = glm::mod(mesh->transform.rotation.x + ((spinRate/2.f) * deltaTimeSeconds), glm::two_pi<float>());

			//obj.transform.translation.x = obj.transform.translation.x + (0.6f * deltaTimeSeconds);
			
			if (camera == nullptr) { throw std::runtime_error("renderEngineObjects null camera pointer"); }

			//camera->transform.testTranslation(time, 0.5f, deltaTimeSeconds);
			//camera.translation.z = camera.translation.z - (1.3f * deltaTimeSeconds);

			// temporary input system
			float ix = 0.f; float iy = 0.f; float iz = 0.f;
			const float moveSpeed = 60.f;
			if (wasdrf[0]) { ix = moveSpeed * deltaTimeSeconds; }
			if (wasdrf[1]) { iy = -moveSpeed * deltaTimeSeconds; }
			if (wasdrf[2]) { ix = -moveSpeed * deltaTimeSeconds; }
			if (wasdrf[3]) { iy = moveSpeed * deltaTimeSeconds; }
			if (wasdrf[4]) { iz = moveSpeed * deltaTimeSeconds/10; }
			if (wasdrf[5]) { iz = -moveSpeed * deltaTimeSeconds/10; }
			camera->transform.translation += glm::vec3{ ix, iy, iz }; 

			glm::mat4 vMatrix = glm::inverse(camera->transform.mat4());
			glm::mat4 pMatrix = perspectiveMatrix(camera->aspectRatio, camera->vFOV, camera->nearPlane, camera->farPlane);

			push.transform = pMatrix * worldToVulkan() * vMatrix * mesh->transform.mat4();

			vkCmdPushConstants(commandBuffer, pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);
			mesh->bind(commandBuffer);
			mesh->draw(commandBuffer);
		}*/

	glm::mat4 MeshRenderSystem::lerpMat4(float t, glm::mat4 matA, glm::mat4 matB) 
	{
		glm::mat4 matOut{};

		for (int c = 0; c != 4; c++)
		{
			for (int r = 0; r != 4; r++)
			{
				matOut[c][r] = lerp(matA[c][r], matB[c][r], t);
			}
		}

		return matOut;
	}

} // namespace