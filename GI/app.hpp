#pragma once

#include "descriptors.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "renderer.hpp"
#include "window.hpp"

// std
#include <memory>
#include <vector>

class Application {
public:
	static constexpr int WIDTH = 1200;
	static constexpr int HEIGHT = 800;

	Application();
	~Application();

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	void run();

private:
	void loadGameObjects();

	Window window{ WIDTH, HEIGHT, "Hello Vulkan!" };
	Device device{ window };
	Renderer renderer{ window, device };



	std::unique_ptr<DescriptorPool> globalPool{};
	GameObject::Map gameObjects;
	GameObject::id_t controlledLightId{};
};
