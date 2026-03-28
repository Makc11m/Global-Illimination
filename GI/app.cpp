#include "app.hpp"

#include "keyboard_control.hpp"
#include "buffer.hpp"
#include "simple_render_system.hpp"
#include "camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <iostream>
#include <array>
#include <chrono>
#include <cassert>
#include <stdexcept>

//struct SimplePushConstantData {
//    glm::mat2 transform{ 1.f };
//    glm::vec2 offset;
//    alignas(16) glm::vec3 color;
//};

struct GlobalUbo {
	glm::mat4 projectionView{ 1.f };
	glm::vec3 lightDirection = glm::normalize(glm::vec3(1.f, -3.f, -1.f));
};

Application::Application() {
    loadGameObjects();
}

Application::~Application() {}

void Application::run() {

	std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < uboBuffers.size(); i++) {
		uboBuffers[i] = std::make_unique<Buffer>(
			device,
			sizeof(GlobalUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uboBuffers[i]->map();
	}

	SimpleRenderSystem simpleRenderSystem{ device, renderer.getSwapChainRenderPass() };
	Camera camera{};
    camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.0f, 0.0f, 2.5f));

	auto viewerObject = GameObject::createGameObject();
	KeyboardControl cameraControl{};

	auto currentTime = std::chrono::high_resolution_clock::now();

    while (!window.shouldClose()) {
        glfwPollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		cameraControl.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
		camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

		float aspect = renderer.getAspectRatio();
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

        if (auto commandBuffer = renderer.beginFrame()) {
			int frameIndex = renderer.getFrameIndex();
			FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera };

			//update
			GlobalUbo ubo{};	
			ubo.projectionView = camera.getProjection() * camera.getView();
			uboBuffers[frameIndex]->writeToBuffer(&ubo);
			uboBuffers[frameIndex]->flush();

			//render
			renderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
			renderer.endSwapChainRenderPass(commandBuffer);
			renderer.endFrame();

        }
    }

    vkDeviceWaitIdle(device.device());
}


void Application::loadGameObjects() {
	std::shared_ptr<Model> model = Model::createModelFromFile(device, "models/flat_vase.obj");
	auto flatVase = GameObject::createGameObject();
	flatVase.model = model;
	flatVase.transform.translation = {-.5f, .5f, 2.5f};
	flatVase.transform.scale = {3.f, 3.f, 3.f};
	gameObjects.push_back(std::move(flatVase));

	model = Model::createModelFromFile(device, "models/smooth_vase.obj");
	auto smoothVase = GameObject::createGameObject();
	smoothVase.model = model;
	smoothVase.transform.translation = { .5f, .5f, 2.5f };
	smoothVase.transform.scale = { 3.f, 3.f, 3.f };
	gameObjects.push_back(std::move(smoothVase));
}