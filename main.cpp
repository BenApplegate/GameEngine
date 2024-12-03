#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <vector>
#include <cstring>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData) {
    std::cerr << "Validation Layer: " << pCallbackData->pMessage << "\n";
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

class GameEngine {
public:
    void run() {
        initWindow();
        initVulkan();
        engineLoop();
        destroy();
    }
private:
    GLFWwindow* window;

    void initWindow() {
        //Init GLFW
        glfwInit();
        //Only use GLFW for window, no OpenGL Context needed
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //Disable user resizing
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        //Create the actual window
        window = glfwCreateWindow(1920, 1080, "Game Engine Testing", nullptr, nullptr);
    }

    VkInstance vulkanInstance;
    VkDebugUtilsMessengerEXT debugMessenger;
    const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
    };
    //Enable validation layers on debug only
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    void initVulkan(){
        createVulkanInstance();
        setupDebugMessenger();
    }

    void createVulkanInstance(){
        //Check validation layer compatibility
        if(enableValidationLayers && !checkValidationLayerSupport()){
            throw std::runtime_error("Validation Layers requested are unavailable");
        }

        //Set App Info Structure
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Engine Testing";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.pEngineName = "tbd";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        //Get GLFW extenstions
        auto extensions = getRequiredExtensions();

        //Set Create Info structure
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();


        std::cout << "Loaded extensions: " << '\n';
        for(auto & extension : extensions){
            std::cout << extension << '\n';
        }

        //Create the instance and throw if it fails
        if(vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS){
            throw std::runtime_error("Failed to create Vulkan Instance");
        }
    }

    bool checkValidationLayerSupport(){
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for(const char* layerName : validationLayers){
            bool layerFound = false;

            for(const auto& layerProperties : availableLayers){
                if(strcmp(layerName, layerProperties.layerName) == 0){
                    layerFound = true;
                    break;
                }
            }

            if(!layerFound) return false;
        }

        return true;
    }

    std::vector<const char*> getRequiredExtensions(){
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if(enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(vulkanInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void engineLoop() {
        //Loop until the engine closes
        while(!glfwWindowShouldClose(window)) {
            //Update events before updating further
            glfwPollEvents();


        }
    }

    void destroy() {

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(vulkanInstance, debugMessenger, nullptr);
        }

        //Destroy vulkan instance
        vkDestroyInstance(vulkanInstance, nullptr);

        //Destroy window and close GLFW
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};


int main() {
    GameEngine engine;

    try{
        engine.run();
    } catch (const std::exception& ex){
        std::cerr << ex.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

