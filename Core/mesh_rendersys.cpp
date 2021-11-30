#include "mesh_rendersys.h"

#include "ECS/CameraComponent.h"

#include <stdexcept>
#include <array>
#include <limits>
#include <iostream> // temporary

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace EngineCore
{
	void MeshRenderSystem::renderMeshes(VkCommandBuffer commandBuffer, std::vector<StaticMesh*>& meshes,
			CameraComponent* camera, const float& deltaTimeSeconds, float time, InputSystem* inputSysPtr, 
			VkDescriptorSet sceneGlobalDescriptorSet)
	{
		for (auto* pMesh : meshes)
		{
			if (!pMesh || !pMesh->getMaterial()) { continue; }
			auto& mesh = *pMesh;
			auto& material = *mesh.getMaterial();

			material.bindToCommandBuffer(commandBuffer); // bind material-specific shading pipeline
			// bind scene global descriptor set for every primitive
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material.getPipelineLayout(),
									0, 1, &sceneGlobalDescriptorSet, 0, nullptr);

			// spin 3D primitive - TODO: remove
			float spinRate = 0.0f;
			mesh.transform.rotation.y = glm::mod(mesh.transform.rotation.y + (spinRate * deltaTimeSeconds), glm::two_pi<float>());
			mesh.transform.rotation.x = glm::mod(mesh.transform.rotation.x + ((spinRate / 2.f) * deltaTimeSeconds), glm::two_pi<float>());

			/*if (camera != nullptr)
			{
				// camera rotation
				//camera->transform.rotation += glm::vec3{ -x, y, 0.0 } * 0.03f;
				glm::vec3 rot = { -inputSysPtr->getMouseDelta().x, inputSysPtr->getMouseDelta().y, 0.f};
				rot = { Transform3D::degToRad(rot.x), Transform3D::degToRad(rot.y), 0.f };
				camera->transform.rotation += rot * 0.03f;
				auto x = camera->transform.rotation.x; auto y = camera->transform.rotation.y; auto z = camera->transform.rotation.z;
				std::cout << "x: " << x << " y: " << y << " z: " << z << "\n \n \n";

				// camera translation
				glm::vec3 camFwdVec = camera->transform.getForwardVector();
				float fwdInput = inputSysPtr->getAxisValue(0);
				float constexpr epsilon = std::numeric_limits<float>::epsilon();
				if ((glm::dot(camFwdVec, camFwdVec) > epsilon) && (fwdInput > epsilon || fwdInput < -epsilon))
				{
					camera->transform.translation += glm::normalize(camFwdVec) * (fwdInput * 1.2f * deltaTimeSeconds);
				}
				camera->transform.translation.y += -inputSysPtr->getAxisValue(1) * deltaTimeSeconds * 1.2f;
				camera->transform.translation.z += inputSysPtr->getAxisValue(2) * deltaTimeSeconds * 1.2f;
			}
			else 
			{ throw std::runtime_error("renderEngineObjects null camera pointer"); }*/

			auto lookInput = inputSysPtr->getMouseDelta();
			auto mf = inputSysPtr->getAxisValue(0);
			auto mr = inputSysPtr->getAxisValue(1);
			auto mu = inputSysPtr->getAxisValue(2);
			camera->moveInPlaneXZ(lookInput, mf, mr, mu, deltaTimeSeconds);

			/* view matrix and mesh transform matrix
			glm::mat4 projectionMatrix = camera->getProjectionMatrixBlender();
			glm::mat4 viewMatrix = camera->getViewMatrix(true);
			glm::mat4 meshMatrix =  mesh.transform.mat4();
			glm::mat4 worldMatrix = CameraComponent::getWorldBasisMatrix();
			// old way of sending matrices to gpu
			push.transform = projectionMatrix * worldMatrix * viewMatrix * meshMatrix;
			vkCmdPushConstants(commandBuffer, mesh->getMaterial()->getPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);*/

			Material::MeshPushConstants push{};
			push.transform = mesh.transform.mat4();
			material.writePushConstantsForMesh(commandBuffer, push);

			// record mesh draw command
			mesh.bind(commandBuffer);
			mesh.draw(commandBuffer);
		}
	}

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