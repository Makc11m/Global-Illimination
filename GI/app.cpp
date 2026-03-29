#include "app.hpp"

#include "keyboard_control.hpp"
#include "buffer.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"
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

Application::Application() {
	globalPool = 
		DescriptorPool::Builder(device)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
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
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uboBuffers[i]->map();
	}

	auto globalSetLayout = DescriptorSetLayout::Builder(device)
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.build();

	std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++) {
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		DescriptorWriter(*globalSetLayout, *globalPool)
			.writeBuffer(0, &bufferInfo)
			.build(globalDescriptorSets[i]);
	}

	SimpleRenderSystem simpleRenderSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
	PointLightSystem pointLightSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
	Camera camera{};
    camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.0f, 0.0f, 2.5f));

	auto viewerObject = GameObject::createGameObject();
	viewerObject.transform.translation = { 0.f, 0.f, -2.5f };
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
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

        if (auto commandBuffer = renderer.beginFrame()) {
			int frameIndex = renderer.getFrameIndex();
			FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects};

			//update
			GlobalUbo ubo{};	
			ubo.projection = camera.getProjection();
			ubo.view = camera.getView();
			ubo.inverseView = camera.getInverseView();
			pointLightSystem.update(frameInfo, ubo);
			uboBuffers[frameIndex]->writeToBuffer(&ubo);
			uboBuffers[frameIndex]->flush();

			//render
			renderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(frameInfo);
			pointLightSystem.render(frameInfo);
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
	flatVase.transform.translation = {-.5f, .5f, 0.f};
	flatVase.transform.scale = {3.f, 3.f, 3.f};
	gameObjects.emplace(flatVase.getId(), std::move(flatVase));

	model = Model::createModelFromFile(device, "models/smooth_vase.obj");
	auto smoothVase = GameObject::createGameObject();
	smoothVase.model = model;
	smoothVase.transform.translation = { .5f, .5f, 0.f };
	smoothVase.transform.scale = { 3.f, 3.f, 3.f };
	gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

	model = Model::createModelFromFile(device, "models/quad.obj"); 
	auto floor = GameObject::createGameObject();
	floor.model = model;
	floor.transform.translation = { .0f, .5f, 0.f };
	floor.transform.scale = { 3.f, 1.f, 3.f };
	gameObjects.emplace(floor.getId(), std::move(floor));

	//auto pointLight = GameObject::makePointLight(0.2f);
	//gameObjects.emplace(pointLight.getId(), std::move(pointLight));

	std::vector<glm::vec3> lightColors{
		//{1.f, .1f, .1f},
		//{.1f, .1f, 1.f},
		//{.1f, 1.f, .1f},
		//{1.f, 1.f, .1f},
		//{.1f, 1.f, 1.f},
		{1.f, 1.f, 1.f}  
	};

	for (int i = 0; i < lightColors.size(); i++) {
		auto pointLight = GameObject::makePointLight(0.2f);
		pointLight.color = lightColors[i];
		auto rotateLight = glm::rotate(
			glm::mat4(1.f),
			(i * glm::two_pi<float>()) / lightColors.size(),
			{ 0.f, -1.f, 0.f });
		pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, 0.25f, -1.f, 1.f));
		gameObjects.emplace(pointLight.getId(), std::move(pointLight));
	}
}