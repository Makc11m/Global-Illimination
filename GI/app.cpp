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

	initImGui();
}

Application::~Application() {}

void Application::initImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// --- GLFW ---
	ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);

	// --- Descriptor Pool ---
	std::array<VkDescriptorPoolSize, 1> poolSizes = { {
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }
	} };

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();

	vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &imguiPool);

	// --- Vulkan ---
	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = device.getInstance();
	initInfo.PhysicalDevice = device.getPhysicalDevice();
	initInfo.Device = device.device();
	initInfo.Queue = device.graphicsQueue();
	initInfo.DescriptorPool = imguiPool;
	initInfo.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
	initInfo.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
	initInfo.PipelineInfoMain.RenderPass = renderer.getSwapChainRenderPass();
	ImGui_ImplVulkan_Init(&initInfo);	
}

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
    camera.setViewTarget(glm::vec3(-1.f, -1.f, 2.f), glm::vec3(0.0f, 0.0f, 2.5f));

	auto viewerObject = GameObject::createGameObject();
	viewerObject.transform.translation = { 0.f, -0.5f, -5.5f };
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

		if (auto it = gameObjects.find(controlledLightId); it != gameObjects.end() && it->second.pointLight) {
			auto& light = it->second.pointLight;

			const float delta = 0.5f * frameTime; 
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_UP) == GLFW_PRESS) {
				light->lightIntensity += delta;
			}
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_DOWN) == GLFW_PRESS) {
				light->lightIntensity -= delta;
			}

			light->lightIntensity = glm::clamp(light->lightIntensity, 0.f, 8.f);
		}

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
			//ImGUI
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Scene");
			ImGui::End();

            simpleRenderSystem.renderGameObjects(frameInfo);
			pointLightSystem.render(frameInfo);

			ImGui::Render();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

			renderer.endSwapChainRenderPass(commandBuffer);

			renderer.endFrame();

        }
    }

    vkDeviceWaitIdle(device.device());
}


void Application::loadGameObjects() {
	std::shared_ptr<Model> model = Model::createModelFromFile(device, "models/smooth_vase.obj");
	auto smoothVase = GameObject::createGameObject();
	smoothVase.model = model;
	smoothVase.transform.translation = { .5f, .5f, 0.f };
	smoothVase.transform.rotation = { .0f, .0f, glm::two_pi<float>() };
	smoothVase.transform.scale = { 3.f, 3.f, 3.f };
	gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

	model = Model::createModelFromFile(device, "models/quad.obj");
	auto floor = GameObject::createGameObject();
	floor.model = model;
	floor.transform.translation = { .0f, .5f, 0.f };
	floor.transform.scale = { 9.f, 1.f, 9.f };
	gameObjects.emplace(floor.getId(), std::move(floor));

	model = Model::createModelFromFile(device, "models/quad.obj");
	auto wall_1 = GameObject::createGameObject();
	wall_1.model = model;
	wall_1.transform.translation = { -1.3f, 0.f, 0.f };
	wall_1.transform.rotation = { 0.0f, 0.f, static_cast<float>(glm::half_pi<float>()) };
	wall_1.transform.scale = { 1.f, 1.f, 1.3f };
	gameObjects.emplace(wall_1.getId(), std::move(wall_1));

	model = Model::createModelFromFile(device, "models/quad.obj");
	auto wall_1_b = GameObject::createGameObject();
	wall_1_b.model = model;
	wall_1_b.transform.translation = { -1.301f, 0.f, 0.f };
	wall_1_b.transform.rotation = { 0.0f, 0.f, static_cast<float>(-glm::half_pi<float>()) };
	wall_1_b.transform.scale = { 1.f, 1.f, 1.3f };
	gameObjects.emplace(wall_1_b.getId(), std::move(wall_1_b));

	model = Model::createModelFromFile(device, "models/quad.obj");
	auto wall_2 = GameObject::createGameObject();
	wall_2.model = model;
	wall_2.transform.translation = { 1.3f, 0.f, 0.f };
	wall_2.transform.rotation = { 0.0f, 0.f, static_cast<float>(-glm::half_pi<float>()) };
	wall_2.transform.scale = { 1.f, 1.f, 1.3f };
	gameObjects.emplace(wall_2.getId(), std::move(wall_2));

	model = Model::createModelFromFile(device, "models/quad.obj");
	auto wall_2_b = GameObject::createGameObject();
	wall_2_b.model = model;
	wall_2_b.transform.translation = { 1.301f, 0.f, 0.f };
	wall_2_b.transform.rotation = { 0.0f, 0.f, static_cast<float>(glm::half_pi<float>()) };
	wall_2_b.transform.scale = { 1.f, 1.f, 1.3f };
	gameObjects.emplace(wall_2_b.getId(), std::move(wall_2_b));

	model = Model::createModelFromFile(device, "models/quad.obj");
	auto wall_3 = GameObject::createGameObject();
	wall_3.model = model;
	wall_3.transform.translation = { 0.f, 0.f, 1.301f };
	wall_3.transform.rotation = { 0.0f, static_cast<float>(-glm::half_pi<float>()) , static_cast<float>(glm::half_pi<float>()) };
	wall_3.transform.scale = { 1.f, 1.f, 1.3f };
	gameObjects.emplace(wall_3.getId(), std::move(wall_3));

	model = Model::createModelFromFile(device, "models/quad.obj");
	auto wall_3_b = GameObject::createGameObject();
	wall_3_b.model = model;
	wall_3_b.transform.translation = { 0.f, 0.f, 1.3f };
	wall_3_b.transform.rotation = { 0.0f, static_cast<float>(glm::half_pi<float>()) , static_cast<float>(glm::half_pi<float>()) };
	wall_3_b.transform.scale = { 1.f, 1.f, 1.3f };
	gameObjects.emplace(wall_3_b.getId(), std::move(wall_3_b));

	model = Model::createModelFromFile(device, "models/quad.obj");
	auto roof = GameObject::createGameObject();
	roof.model = model;
	roof.transform.translation = { .0f, -1.f, 0.f };
	roof.transform.rotation = { 0.0f, 0.f, static_cast<float>(glm::pi<float>()) };
	roof.transform.scale = { 1.3f, 1.f, 1.3f };
	gameObjects.emplace(roof.getId(), std::move(roof));

	model = Model::createModelFromFile(device, "models/quad.obj");
	auto roof_b = GameObject::createGameObject();
	roof_b.model = model;
	roof_b.transform.translation = { .0f, -1.001f, 0.f };
	roof_b.transform.rotation = { 0.0f, 0.f, 0.f };
	roof_b.transform.scale = { 1.3f, 1.f, 1.3f };
	gameObjects.emplace(roof_b.getId(), std::move(roof_b));

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
		auto pointLight = GameObject::makePointLight(.5f);
		controlledLightId = pointLight.getId();
		pointLight.color = lightColors[i];
		auto rotateLight = glm::rotate(
			glm::mat4(1.f),
			(i * glm::two_pi<float>()) / lightColors.size(),
			{ 0.f, -1.f, 0.f });
		pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.5f, 0.25f, -1.5f, 1.f));
		gameObjects.emplace(pointLight.getId(), std::move(pointLight));
	}
}