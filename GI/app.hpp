#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "renderer.hpp"
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
	void renderGameObjects(VkCommandBuffer commandBuffer);

	Window window{ WIDTH, HEIGHT, "Hello Vulkan!" };
	Device device{ window };
	Renderer renderer{ window, device };

	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<GameObject> gameObjects;
};
