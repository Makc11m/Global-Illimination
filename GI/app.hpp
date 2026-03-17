#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "swap_chain.hpp"
#include "window.hpp"
#include "game_object.hpp"

// std
#include <memory>
#include <vector>

class Application {
public:
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;

	Application();
	~Application();

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	void run();

private:
	void loadGameObjects();
	void createPipelineLayout();
	void createPipeline();
	void createCommandBuffers();
	void freeCommandBuffers();
	void drawFrame();
	void recreateSwapChain();
	void recordCommandBuffer(int imageIndex);
	void renderGameObjects(VkCommandBuffer commandBuffer);

	Window window{ WIDTH, HEIGHT, "Hello Vulkan!" };
	Device device{ window };
	std::unique_ptr<SwapChain> swapChain;
	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<GameObject> gameObjects;
};
