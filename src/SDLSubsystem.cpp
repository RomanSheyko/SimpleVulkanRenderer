#include "SDLSubsystem.h"
#include <stdint.h>
#include <chrono>
#include <thread>
#include <array>
#include <iostream>

SDLSubsystem::SDLSubsystem(const char* window_name, size_t width, size_t height) : width(width), height(height)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	window = nullptr;
    
    unsigned int count;
    createWindow(window_name, width, height);
    // get count of required extensions
    if(SDL_Vulkan_GetInstanceExtensions(NULL, &count, NULL) == SDL_FALSE)
        throw WindowSubsystemException("Error getting number of required extensions");
    
    static const char *const additionalExtensions[] =
    {
        RQUIRED_INSTANCE_EXTENTIONS // example additional extension
    };
    size_t additionalExtensionsCount = sizeof(additionalExtensions) / sizeof(additionalExtensions[0]);
    size_t extensionCount = count + additionalExtensionsCount;
    const char **names = (const char**)malloc(sizeof(const char *) * extensionCount);
    if(!names)
        throw WindowSubsystemException("Error allocating extansion names array");
    
    // get names of required extensions
    if(!SDL_Vulkan_GetInstanceExtensions(NULL, &count, names))
        throw WindowSubsystemException("Error getting required extensions");
    
    // copy additional extensions after required extensions
    for(size_t i = 0; i < additionalExtensionsCount; i++)
        names[i + count] = additionalExtensions[i];
    
    for(size_t i = 0; i < additionalExtensionsCount + count; i++)
    {
        requiredInstanceExtentions.push_back(names[i]);
    }
    free(names);
    renderer = std::make_unique<VulkanRenderer>(requiredInstanceExtentions);
    
    createSurface();
    loadSceneObjects();
}


void SDLSubsystem::createWindow(const char* window_name, size_t width, size_t height)
{
	window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)width, (int)height, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
    SDL_SetWindowGrab(window, SDL_TRUE);
    //SDL_SetRelativeMouseMode(SDL_TRUE);
	if (window == nullptr)
	{
		throw WindowSubsystemException("Error creating a window");
	}
}

void SDLSubsystem::createSurface()
{
	if (!SDL_Vulkan_CreateSurface(window, renderer->getInstance(), &renderer->getSurface().surface))
	{
		throw WindowSubsystemException("Error creating a surface");
	}
    renderer->init();
}

void SDLSubsystem::mainLoop()
{
	SDL_Event e;
	bool exit = false;
    
    auto timer = std::chrono::steady_clock();
    auto last_time = timer.now();
    uint64_t frame_counter = 0;
    uint64_t fps = 0;
    SDL_ShowCursor(SDL_DISABLE);
    
	while (!exit)
	{
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				exit = true;
			}
		}
        
        ++frame_counter;
        if(last_time + std::chrono::seconds(1) < timer.now())
        {
            last_time = timer.now();
            fps = frame_counter;
            frame_counter = 0;
            std::cout << "FPS: " << fps << std::endl;
        }
        static int frame = 30;
        frame = (frame + 1) % 100;
        
        renderer->beginRender();
        
        renderer->update(sceneObjects, camera);
        processInput();
        
        renderer->endRender();
	}
    
    renderer->waitIdle();
}

SDLSubsystem::~SDLSubsystem()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void SDLSubsystem::loadSceneObjects() {
    /*
    std::vector<Model::Vertex> vertices = {
        //Front face
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
        //Back face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
        //Top face
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
        //Bottom face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
        //Right face
        {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
        //Left face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    
    std::vector<uint32_t> indices = {
        0,  1,  2,      0,  2,  3,    // front
        4,  5,  6,      4,  6,  7,    // back
        8,  9,  10,     8,  10, 11,   // top
        12, 13, 14,     12, 14, 15,   // bottom
        16, 17, 18,     16, 18, 19,   // right
        20, 21, 22,     20, 22, 23    // left
    };
     */
    std::vector<Model::Vertex> vertices;
    std::vector<uint32_t> indices;
    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
        throw std::runtime_error(warn + err);
    }
    float w = 0.0f;
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Model::Vertex vertex{};
            
            vertex.positon = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            
            //vertex.color = {static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0f)), static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0f)), static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0f))};
            
            vertex.color = {1.0f, 1.0f, 1.0f};

            vertices.push_back(vertex);
            indices.push_back(indices.size());
        }
        if(w < 1.0f) w += 0.1f;
    }
    
    auto model = std::make_shared<Model>(*renderer.get(), vertices, indices);
    
    auto traingle = SceneObject::createSceneObject();
    traingle.model = model;
    //traingle.color = {.1f, .8f, .1f};
    //traingle.transform2d.translation.x = .2f;
    
    sceneObjects.push_back(std::move(traingle));
}

void SDLSubsystem::processInput() {
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if(state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) camera.moveRight();
    if(state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) camera.moveLeft();
    if(state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) camera.moveForward();
    if(state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) camera.moveBack();
    
    int xpos, ypos;
    SDL_GetMouseState(&xpos, &ypos);
    if(xpos == 0 || ypos == 0 || xpos == width - 1 || ypos == height - 1)
    {
        int x = width/2;
        int y = height/2;
        SDL_WarpMouseInWindow(window, x, y);
        camera.reset_mouse_pos(static_cast<double>(x), static_cast<double>(y));
        return;
    }
    
    //std::cout << "x: " << xpos << "\ny: " << ypos << std::endl;
    
    camera.update(static_cast<double>(xpos), static_cast<double>(ypos));
}


