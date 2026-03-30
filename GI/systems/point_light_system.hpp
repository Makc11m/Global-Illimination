#pragma once

#include "../device.hpp"
#include "../game_object.hpp"
#include "../pipeline.hpp"
#include "../camera.hpp"
#include "../frame_info.hpp"

// std
#include <memory>
#include <vector>

class PointLightSystem {
public:
	PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	~PointLightSystem();

	PointLightSystem(const PointLightSystem&) = delete;
	PointLightSystem& operator=(const PointLightSystem&) = delete;

	float getLightAngle() const { return lightAngle; }
	void update(FrameInfo& frameInfo,  GlobalUbo &ubo);
	void render(FrameInfo& frameInfo);

private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);

	Device& device;

	float lightAngle = 0.f;
	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};
