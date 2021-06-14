#ifndef SceneObject_h
#define SceneObject_h
#include "Model.h"

#include <memory>

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
    
    id_t getId()
    {
        return id;
    }
private:
    SceneObject(id_t id) : id(id) {}
    
    id_t id;
};

#endif /* SceneObject_h */
