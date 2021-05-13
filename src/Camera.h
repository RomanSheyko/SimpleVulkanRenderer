#ifndef Camera_h
#define Camera_h

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <chrono>

class Camera
{
public:
    Camera();
    Camera(const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp, float cameraSpeed);
    
    void moveForward();
    void moveBack();
    void moveLeft();
    void moveRight();
    
    void update(double xpos, double ypos);
    
    glm::vec3 getPos() const;
    glm::vec3 getFront() const;
    glm::vec3 getUp() const;
private:
    std::chrono::duration<float> deltaTime;                          // Time between current frame and last frame
    std::chrono::time_point<std::chrono::steady_clock> lastFrame;    // Time of last frame
    std::chrono::steady_clock clock;
    float cameraSpeed;
    bool firstMouse;
    void look_around_update(double xpos, double ypos);
    float lastX;
    float lastY;
    float yaw;
    float pitch;
    
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
};

#endif /* Camera_h */
