#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>

using std::cout;
using std::runtime_error;
using std::vector;
using std::cerr;

static constexpr uint32_t WIDTH = 800;
static constexpr uint32_t HEIGHT = 600;

const vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
#ifdef DEBUG
static constexpr bool enableValidationLayers = true;
#else
static constexpr bool enableValidationLayers = false;
#endif // DEBUG

class HelloTriangleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan-Hello-Triangle", nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		#ifdef DEBUG // Destroy Validation Levels
		auto destroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (destroyDebugUtilsMessenger != nullptr) {
			destroyDebugUtilsMessenger(instance, debugMessenger, nullptr);
		}
		#endif // DEBUG
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void createInstance() {
		if (enableValidationLayers && !checkValidationLayersSupport()) {
			throw runtime_error("!createInstance: !checkValidationLayersSupport");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;
		// appInfo.pNext = nullptr; // automatically set to nullptr because extension is unused

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		const vector<const char*> requiredExtensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		#ifdef DEBUG // Add Validation Levels Info
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;
		debugCreateInfo.pUserData = nullptr;

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		createInfo.pNext = &debugCreateInfo;
		#else
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
		#endif // DEBUG

		if (!extensionRequirementsSatisfiedInstance(createInfo.enabledExtensionCount, createInfo.ppEnabledExtensionNames)) {
			throw runtime_error("!createInstance: !extensionRequirementsSatisfiedInstance");
		}
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw runtime_error("!createInstance: vkCreateInstance != VK_SUCCESS");
		}

		#ifdef DEBUG // Create Validation Levels
		auto createDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (createDebugUtilsMessenger == nullptr) {
			throw runtime_error("!createInstance createDebugUtilsMessenger == nullptr");
		}
		if (createDebugUtilsMessenger(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw runtime_error("!createInstance createDebugUtilsMessenger != VK_SUCCESS");
		}
		#endif // DEBUG
	}

	bool checkValidationLayersSupport() const {
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName: validationLayers) {
			bool layerFound = false;
			for (const VkLayerProperties& layerProperties: availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				cout << "!checkValidationLayersSupport: " << layerName << " !found\n";
				return false;
			}
		}

		return true;
	}

	vector<const char*> getRequiredExtensions() const {
		uint32_t glfwRequiredExtensionsCount = 0;
		const char** glfwExtensionsNames = glfwGetRequiredInstanceExtensions(&glfwRequiredExtensionsCount);
		vector<const char*> requiredExtensions(glfwExtensionsNames, glfwExtensionsNames + glfwRequiredExtensionsCount);

		if (enableValidationLayers) {
			++glfwRequiredExtensionsCount;
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return requiredExtensions;
	}

	bool extensionRequirementsSatisfiedInstance(uint32_t numRequiredExtensions, const char* const *requiredExtensionsNames) const {
		#ifdef DEBUG
		cout << "DEBUG: Required extensions for creating VkInstance:\n";
		for (uint32_t i = 0; i < numRequiredExtensions; ++i) {
			cout << '\t' << requiredExtensionsNames[i] << '\n';
		}
		#endif // DEBUG
		if (numRequiredExtensions == 0) { // no requirements
			return true;
		}

		// get extensions count
		uint32_t numSupportedExtensions = 0;
		VkResult res = vkEnumerateInstanceExtensionProperties(nullptr, &numSupportedExtensions, nullptr);
		if (res != VK_SUCCESS) {
			throw runtime_error("!extensionRequirementsSatisfiedInstance: vkEnumerateInstanceExtensionProperties != VK_SUCCESS");
		}
		if (numSupportedExtensions < numRequiredExtensions) {
			return false;
		}
		// populate extensions array
		vector<VkExtensionProperties> supportedExtensions(numSupportedExtensions);
		res = vkEnumerateInstanceExtensionProperties(nullptr, &numSupportedExtensions, supportedExtensions.data());
		if (res != VK_SUCCESS) {
			throw runtime_error("!extensionRequirementsSatisfiedInstance: vkEnumerateInstanceExtensionProperties != VK_SUCCESS");
		}
		#ifdef DEBUG
		cout << "DEBUG: Supported Vulkan extensions for VkInstance:";
		for (const VkExtensionProperties& extension: supportedExtensions) {
			cout << '\t' << extension.extensionName << '\n';
		}
		#endif // DEBUG

		// check if all required are supported
		for (uint32_t i = 0; i < numRequiredExtensions; ++i) {
			const char* const requiredExtensionName = requiredExtensionsNames[i];
			bool found = false;
			for (const VkExtensionProperties& extension: supportedExtensions) {
				if (strcmp(extension.extensionName, requiredExtensionName) == 0) {
					found = true;
					break;
				}
			}
			if (!found) {
				cout << "!extensionRequirementsSatisfiedInstance: " << requiredExtensionName << " not found\n";
				return false;
			}
		}

		return true;
	}

	static VkBool32 debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* /*pUserData*/
	) {
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			cerr << "VL[" << messageSeverity << "]: " << pCallbackData->pMessage << ":\n";
			for (uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
				cerr << '\t' << pCallbackData->pObjects[i].pObjectName << '\n';
			}
			return VK_FALSE;
		}

		cout << "VL[" << messageSeverity << "]: " << pCallbackData->pMessage << '\n';
		return VK_TRUE;
	}

	GLFWwindow *window;
	VkInstance instance;

	#ifdef DEBUG // Validation Levels
	VkDebugUtilsMessengerEXT debugMessenger;
	#endif // DEBUG
};

int main(int /*argc*/, const char** /*argv*/) {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
