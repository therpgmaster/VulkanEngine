#define TINYOBJLOADER_IMPLEMENTATION // mesh file loader
#include "Core/ECS/StaticMesh.h"

// std
#include <cassert>
#include <cstring>

namespace EngineCore
{
	StaticMesh::StaticMesh(EngineDevice& device, const MeshBuilder& builder) : engineDevice{ device }
	{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	StaticMesh::StaticMesh(EngineDevice& device, const std::vector<Vertex>& vertices) : engineDevice{ device }
	{
		createVertexBuffers(vertices);
	}

	StaticMesh::StaticMesh(EngineDevice& device) : engineDevice{ device } 
	{
		StaticMesh::MeshBuilder builder{};
		builder.makeCubeMesh();
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	StaticMesh::~StaticMesh(){}

	void StaticMesh::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "vertexCount cannot be below 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);
		// temporary buffer to transfer from CPU (host) to GPU (device)
		GBuffer stagingBuffer
		{ 
			engineDevice, vertexSize, vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data()); // write vertices

		// destination buffer, GPU only for speed (not host accessible)
		vertexBuffer = std::make_unique<GBuffer>(engineDevice, vertexSize, vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		engineDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	void StaticMesh::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;
		if (!hasIndexBuffer) { return; }
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);
		// same as for vertex buffer
		GBuffer stagingBuffer
		{
			engineDevice, indexSize, indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<GBuffer>(engineDevice, indexSize, indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // note INDEX_BUFFER_BIT

		engineDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
	}

	void StaticMesh::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		if (hasIndexBuffer) { vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32); }
	}

	void StaticMesh::draw(VkCommandBuffer commandBuffer)
	{
		if (hasIndexBuffer) { vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0); }
		else { vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0); }
	}

	void StaticMesh::MeshBuilder::loadFromFile(const std::string& path) 
	{
		// TODO: support different mesh formats
		// OBJ format mesh loader, using TinyObjLoader (for now)
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) 
		{ throw std::runtime_error("error loading mesh from file: " + warn + err); }
		vertices.clear();
		indices.clear(); // indices = 0 for OBJ format to indicate non-indexed primitive
		for (const auto& shape : shapes) 
		{
			for (const auto& index : shape.mesh.indices) 
			{
				Vertex vert{};
				if (index.vertex_index >= 0) 
				{
					vert.position = { attrib.vertices[3 * index.vertex_index],
									attrib.vertices[3 * index.vertex_index + 1],
									attrib.vertices[3 * index.vertex_index + 2] };
				}
				if (index.normal_index >= 0)
				{
					vert.normal = { attrib.normals[3 * index.normal_index],
									attrib.normals[3 * index.normal_index + 1],
									attrib.normals[3 * index.normal_index + 2] };
				}
				if (index.texcoord_index >= 0)
				{
					vert.uv = { attrib.texcoords[2 * index.texcoord_index],
								attrib.texcoords[2 * index.texcoord_index + 1] };
				}
				vertices.push_back(vert); // add vertex
			}
		}
	}

	void StaticMesh::MeshBuilder::makeCubeMesh()
	{
		vertices = {
			// -X red
			{{-.5f, -.5f, -.5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, -.5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},

			// X red
			{{.5f, -.5f, -.5f}, {.8f, .05f, .05f}},
			{{.5f, .5f, .5f}, {.8f, .05f, .05f}},
			{{.5f, -.5f, .5f}, {.8f, .05f, .05f}},
			{{.5f, .5f, -.5f}, {.8f, .05f, .05f}},

			// -Y green
			{{-.5f, -.5f, -.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, .5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.1f, .8f, .1f}},

			// Y green
			{{-.5f, .5f, -.5f}, {.05f, .8f, .05f}},
			{{.5f, .5f, .5f}, {.05f, .8f, .05f}},
			{{-.5f, .5f, .5f}, {.05f, .8f, .05f}},
			{{.5f, .5f, -.5f}, {.05f, .8f, .05f}},

			// Z blue
			{{-.5f, -.5f, 0.5f}, {.05f, .05f, .8f}},
			{{.5f, .5f, 0.5f}, {.05f, .05f, .8f}},
			{{-.5f, .5f, 0.5f}, {.05f, .05f, .8f}},
			{{.5f, -.5f, 0.5f}, {.05f, .05f, .8f}},

			// -Z blue
			{{-.5f, -.5f, -0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, -0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, -0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, -0.5f}, {.1f, .1f, .8f}},
		};
		indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
								12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };
	}

	std::vector<VkVertexInputBindingDescription> StaticMesh::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> StaticMesh::Vertex::getAttributeDescriptions()
	{
		return
		{
			// location, binding, format, offset
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) },
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) },
			{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) },
			{ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) }
		};
	}
}