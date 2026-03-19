#pragma once

#include "device.hpp"
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

	Window window{ WIDTH, HEIGHT, "Hello Vulkan!" };
	Device device{ window };
	Renderer renderer{ window, device };

	std::vector<GameObject> gameObjects;
};
