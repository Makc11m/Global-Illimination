#pragma once

#include "descriptors.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "renderer.hpp"
#include "window.hpp"
#include "extentions/ImGUI/imgui.h"
#include "extentions/ImGUI/imgui_impl_glfw.h"
#include "extentions/ImGUI/imgui_impl_vulkan.h"


// std
#include <memory>
#include <vector>

class Application {
public:
	static constexpr int WIDTH = 1920;
	static constexpr int HEIGHT = 1080;

	Application();
	~Application();

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	void run();

private:
	void initImGui();
	void loadGameObjects();

	VkDescriptorPool imguiPool;
	Window window{ WIDTH, HEIGHT, "GI" };
	Device device{ window };
	Renderer renderer{ window, device };



	std::unique_ptr<DescriptorPool> globalPool{};
	GameObject::Map gameObjects;
	//GameObject::id_t controlledLightId{};
};
