#include "Camera.h"

Camera::Camera(const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp, float cameraSpeed) :
cameraSpeed(cameraSpeed),
cameraPos(cameraPos),
cameraFront(cameraFront),
cameraUp(cameraUp),
clock(std::chrono::steady_clock()),
firstMouse(true)
{
    auto currentFrame = clock.now();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    yaw = -90.0f;
    pitch = 0.0f;
}

Camera::Camera() :
cameraSpeed(3.0f),
cameraPos(glm::vec3(0.0f, 0.0f,  3.0f)),
cameraFront(glm::vec3(0.0f, 0.0f, -1.0f)),
cameraUp(glm::vec3(0.0f, 1.0f,  0.0f)),
clock(std::chrono::steady_clock()),
firstMouse(true)
{
    auto currentFrame = clock.now();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    yaw = -90.0f;
    pitch = 0.0f;
}

glm::vec3 Camera::getUp() const { 
    return cameraUp;
}


glm::vec3 Camera::getFront() const { 
    return cameraFront;
}


glm::vec3 Camera::getPos() const { 
    return cameraPos;
}


void Camera::moveRight() { 
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * deltaTime.count();
}


void Camera::moveLeft() { 
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * deltaTime.count();
}


void Camera::moveBack() { 
    cameraPos -= cameraSpeed * deltaTime.count() * cameraFront;
}


void Camera::moveForward() { 
    cameraPos += cameraSpeed * deltaTime.count() * cameraFront;
}

void Camera::update(double xpos, double ypos) {
    auto currentFrame = clock.now();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    look_around_update(xpos, ypos);
}

void Camera::look_around_update(double xpos, double ypos) { 
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
  
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void Camera::reset_mouse_pos(double xpos, double ypos) {
    lastX = xpos;
    lastY = ypos;
}




