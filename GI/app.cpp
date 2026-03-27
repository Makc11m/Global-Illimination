#include "app.hpp"

#include "keyboard_control.hpp"
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

struct SimplePushConstantData {
    glm::mat2 transform{ 1.f };
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

Application::Application() {
    loadGameObjects();
}

Application::~Application() {}

void Application::run() {
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
			renderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
			renderer.endSwapChainRenderPass(commandBuffer);
			renderer.endFrame();

        }
    }

    vkDeviceWaitIdle(device.device());
}


void Application::loadGameObjects() {
	std::shared_ptr<Model> cubeModel = Model::createModelFromFile(device, "models/smooth_vase.obj");

	auto gameObj = GameObject::createGameObject();
	gameObj.model = cubeModel;
	gameObj.transform.translation = {.0f, .0f, 2.5f};
	gameObj.transform.scale = glm::vec3(3.f);
	gameObjects.push_back(std::move(gameObj));
}