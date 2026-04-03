#include "keyboard_control.hpp"

#include <iostream>
#include <unordered_map>

static std::unordered_map<int, bool> keyWasPressed;

void KeyboardControl::changeCameraView(GLFWwindow* window, float dt, GameObject& gameObject) {
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

	const float sensitivity = 0.0010f;

	gameObject.transform.rotation.y += xoffset * sensitivity;
	gameObject.transform.rotation.x += yoffset * sensitivity;

	gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
	gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
}

void KeyboardControl::moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject) {

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

bool KeyboardControl::isKeyPressedOnce(GLFWwindow* window, int key) {
	bool isPressedNow = glfwGetKey(window, key) == GLFW_PRESS;
	bool pressedOnce = isPressedNow && !keyWasPressed[key];
	keyWasPressed[key] = isPressedNow;
	return pressedOnce;
}

void KeyboardControl::changeVisibleCursor(GLFWwindow* window) {
	if (isKeyPressedOnce(window, GLFW_KEY_V)) {
		int mode = glfwGetInputMode(window, GLFW_CURSOR);
		if (mode == GLFW_CURSOR_NORMAL) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);
			lastMouseX = mouseX;
			lastMouseY = mouseY;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);
			lastMouseX = mouseX;
			lastMouseY = mouseY;
		}
		firstMouseInput = true;
	}
}