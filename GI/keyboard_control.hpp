#pragma once

#include "game_object.hpp"
#include "window.hpp"

class KeyboardControl {
public:
    struct KeyMappings {
        int moveLeft = GLFW_KEY_A;
        int moveRight = GLFW_KEY_D;
        int moveForward = GLFW_KEY_W;
        int moveBackward = GLFW_KEY_S;
        int moveUp = GLFW_KEY_SPACE;
        int moveDown = GLFW_KEY_LEFT_CONTROL;
        int lookLeft = GLFW_KEY_LEFT;
        int lookRight = GLFW_KEY_RIGHT;
        int lookUp = GLFW_KEY_UP;
        int lookDown = GLFW_KEY_DOWN;
        int quit = GLFW_KEY_ESCAPE;
    };

	void moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject);

    KeyMappings keys{};
    float moveSpeed{3.f};
    float lookSpeed{1.5f};

	double lastMouseX = 0.0;
	double lastMouseY = 0.0;
	bool firstMouseInput = true;
};