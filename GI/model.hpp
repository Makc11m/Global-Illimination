#pragma once

#include "device.hpp"
#include "buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <string>

class Image {
public:
	Image(Device& device, const std::string& filepath);
	~Image();

	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;

	VkImageView getImageView() const { return imageView; }
	VkSampler getSampler() const { return sampler; }
	VkDescriptorImageInfo descriptorInfo() const;
private:
	void createTextureImage(const std::string& filepath);
	void createTextureImageView();
	void createTextureSampler();

	Device& device;

	VkImage image = VK_NULL_HANDLE;
	VkSampler sampler;
	VkDeviceMemory imageMemory = VK_NULL_HANDLE;
	VkImageView imageView;

	VkDeviceSize imageSize = 0;
	uint32_t texWidth;
	uint32_t texHeight;
};

class Model {
public:
	struct Vertex {
		glm::vec3 position{};
		glm::vec3 color{};
		glm::vec3 normal{};
		glm::vec2 uv{};

		static std::vector<VkVertexInputBindingDescription> getBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

		bool operator==(const Vertex& other) const {
			return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
		}
	};

	struct Builder {
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		void loadModel(const std::string& filepath);
	};

	Model(Device& device, const Model::Builder &builder);
	~Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	static std::unique_ptr<Model> createModelFromFile(Device& device, const std::string& filepath);

	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

private:
	void createVertexBuffers(const std::vector<Vertex>& verticies);
	void createIndexBuffers(const std::vector<uint32_t>& indices);

	Device& device;
	std::unique_ptr<Image> ptrtextureImage;
	VkImage image;
	VkImageView imageView;
	VkSampler sampler;
	VkDeviceMemory imageMemory;

	std::unique_ptr<Buffer> vertexBuffer;
	uint32_t vertexCount;

	bool hasIndexBuffer = false;
	std::unique_ptr<Buffer> indexBuffer;
	uint32_t indexCount;
};