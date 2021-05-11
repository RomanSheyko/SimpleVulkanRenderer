#ifndef SceneObject_h
#define SceneObject_h
#include "Model.h"

#include <memory>

struct Transform2DComponent
{
    glm::vec2 translation {};
    glm::vec2 scale {1.f, 1.f};
    float rotation;
    
    glm::mat2 mat2() {
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);
        glm::mat2 rotMatrix{{c, s}, {-s, c}};
        
        glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
        return rotMatrix * scaleMat;
    }
};

class SceneObject
{
public:
    using id_t = unsigned int;
    static SceneObject createSceneObject()
    {
        static id_t currentId = 0;
        return SceneObject {currentId++};
    }
    
    SceneObject(const SceneObject&) = delete;
    SceneObject& operator=(const SceneObject&) = delete;
    SceneObject(SceneObject&&) = default;
    SceneObject& operator=(SceneObject&&) = default;
    
    std::shared_ptr<Model> model{};
    glm::vec3 color{};
    //Transform2DComponent transform2d{};
    
    id_t getId()
    {
        return id;
    }
private:
    SceneObject(id_t id) : id(id) {}
    
    id_t id;
};

#endif /* SceneObject_h */
