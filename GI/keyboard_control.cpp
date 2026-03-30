#include "keyboard_control.hpp"

void KeyboardControl::moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject) {
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	if (firstMouseInput) {
		lastMouseX = mouseX;
		lastMouseY = mouseY;
		firstMouseInput = false;
	}

	float xoffset = static_cast<float>(mouseX - lastMouseX);
	float yoffset = static_cast<float>(lastMouseY - mouseY);

	lastMouseX = mouseX;
	lastMouseY = mouseY;

	const float sensitivity = 0.0015f;

	gameObject.transform.rotation.y += xoffset * sensitivity;
	gameObject.transform.rotation.x += yoffset * sensitivity;

	glm::vec3 rotate{0.f};

	rotate.x += xoffset * sensitivity;
	rotate.y += yoffset * sensitivity;

	if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
		gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
	}

	gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
	gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

	float yaw = gameObject.transform.rotation.y;
	const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
	const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
	const glm::vec3 upDir{ 0.f, -1.f, 0.f };

	glm::vec3 moveDir{0.f};
	if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) {
		moveDir += forwardDir;
	}
	if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) {
		moveDir -= forwardDir;
	}
	if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) {
		moveDir += rightDir;
	}
	if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) {
		moveDir -= rightDir;
	}
	if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) {
		moveDir += upDir;
	}
	if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) {
		moveDir -= upDir;
	}
	if (glfwGetKey(window, keys.quit) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	
	if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
		gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
	}
}

void KeyboardControl::changeLight(GLFWwindow* window, float dt, GameObject& gameObject) {
	if (glfwGetKey(window, keys.lightUp) == GLFW_PRESS) {
		gameObject.pointLight->lightIntensity += 0.5f * dt;
	}
	if (glfwGetKey(window, keys.lightDown) == GLFW_PRESS) {
		gameObject.pointLight->lightIntensity -= 0.5f * dt;
	}
	if (glfwGetKey(window, keys.lightUp) == GLFW_PRESS) {
		gameObject.pointLight->lightIntensity += 0.5f * dt;
	}
	if (glfwGetKey(window, keys.lightDown) == GLFW_PRESS) {
		gameObject.pointLight->lightIntensity -= 0.5f * dt;
	}
}