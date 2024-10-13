#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <map>
#include <cstring>
#include <optional>

#include "debug.h"

using std::cout;
using std::cerr;
using std::vector;
using std::runtime_error;

static constexpr uint32_t WIDTH = 800;
static constexpr uint32_t HEIGHT = 600;

#ifdef DEBUG
const vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

VkResult CreateDebugUtilsManager(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger
) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		dlog("    CreateDebugUtilsManager vkCreateDebugUtilsMessengerEXT found. Calling...");
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		dlog("    !CreateDebugUtilsManager vkCreateDebugUtilsMessengerEXT NOT FOUND");
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		dlog("    DestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT found. Calling...");
		func(instance, debugMessenger, pAllocator);
	}
	else {
		dlog("    !DestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT NOT FOUND");
	}
}
#endif // DEBUG

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	bool isComplete() const { return graphicsFamily.has_value(); }
};


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
		pickPhysicalDevice();
		createLogicalDevice();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		vkDestroyDevice(device, nullptr);
		dlog("cleanup: vkDestroyDevice");

		#ifdef DEBUG // Destroy Validation Levels
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		dlog("cleanup: vkDestroyDebugUtilsMessengerEXT");
		#endif // DEBUG

		vkDestroyInstance(instance, nullptr);
		dlog("cleanup: vkDestroyInstance");

		glfwDestroyWindow(window);
		dlog("cleanup: glfwDestroyWindow");

		glfwTerminate();
		dlog("cleanup: glfwTerminate");
	}

	void createInstance() {
		#ifdef DEBUG
		if (!checkValidationLayersSupport()) {
			throw runtime_error("!createInstance: !checkValidationLayersSupport");
		}
		#endif // DEBUG

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
		dlog("SUCCESS! createInstance");

		#ifdef DEBUG
		if (CreateDebugUtilsManager(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw runtime_error("!createInstance CreateDebugUtilsManager != VK_SUCCESS");
		}
		#endif // DEBUG
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw runtime_error("!pickPhysicalDevice vkEnumeratePhysicalDevices deviceCount=0");
		}
		dlog("pickPhysicalDevice vkEnumeratePhysicalDevices found N=", deviceCount, " devices");

		vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		std::multimap<size_t, VkPhysicalDevice> candidates;
		for (const VkPhysicalDevice& device : devices) {
			size_t score = rateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}

		if (candidates.rbegin()->first > 0) { // check if best candidate is suitable
			physicalDevice = candidates.rbegin()->second;
		}
		if(physicalDevice == VK_NULL_HANDLE) {
			throw runtime_error("!pickPhysicalDevice vkEnumeratePhysicalDevices no suitable");
		}
		dlog("pickPhysicalDevice picked device[", physicalDevice, ']');
	}

	size_t rateDeviceSuitability(VkPhysicalDevice device) {
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// App can't run without geometry shader
		if (!deviceFeatures.geometryShader) {
			dlog("rateDeviceSuitability !geometryShader");
			return 0;
		}

		// Graphics queue is required
		if (!findQueueFamilies(device).isComplete()) {
			dlog("rateDeviceSuitability !queue with VK_QUEUE_GRAPHICS_BIT");
			return 0;
		}

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		size_t score = 0;
		// Discrete GPU gives significant performance advantage
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 10'000;
		}
		// Max possible size of textures affects graphics quality
		score += deviceProperties.limits.maxImageDimension2D;

		dlog("rateDeviceSuitability[", device,"] deviceName='", deviceProperties.deviceName, " score=", score);
		return score;
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		uint32_t graphicsQueueIndex = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = graphicsQueueIndex;
				break;
			}
			graphicsQueueIndex++;
		}
		dlog("findQueueFamilies(", device, ") graphicsQueueIndex=", graphicsQueueIndex);

		return indices;
	}

	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		if (!indices.isComplete()) {
			throw runtime_error("createLogicalDevice !findQueueFamilies");
		}

		VkDeviceQueueCreateInfo queueCreateInfo{};
		float queuePriority = 1.0f;
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = 0;
		createInfo.enabledLayerCount = 0;
		#ifdef DEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		#endif // DEBUG

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw runtime_error("createLogicalDevice !vkCreateDevice");
		}
		dlog("createLogicalDevice vkCreateDevice SUCCESS");

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		if (graphicsQueue == VK_NULL_HANDLE) {
			log("createLogicalDevice !vkGetDeviceQueue ERROR");
		}
		dlog("createLogicalDevice vkGetDeviceQueue SUCCESS");
	}

	#ifdef DEBUG
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
	#endif // DEBUG

	vector<const char*> getRequiredExtensions() const {
		uint32_t glfwRequiredExtensionsCount = 0;
		const char** glfwExtensionsNames = glfwGetRequiredInstanceExtensions(&glfwRequiredExtensionsCount);
		vector<const char*> requiredExtensions(glfwExtensionsNames, glfwExtensionsNames + glfwRequiredExtensionsCount);

		#ifdef DEBUG
		++glfwRequiredExtensionsCount;
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		#endif // DEBUG

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
			cerr << "VL_E[" << messageSeverity << "]: " << pCallbackData->pMessage << ":\n";
			for (uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
				cerr << '\t' << pCallbackData->pObjects[i].pObjectName << '\n';
			}
			return VK_FALSE;
		}

		//cout << "VL[" << messageSeverity << "]: " << pCallbackData->pMessage << '\n';
		return VK_TRUE;
	}



private:
	GLFWwindow       *window = nullptr;
	VkInstance        instance = VK_NULL_HANDLE;
	VkPhysicalDevice  physicalDevice = VK_NULL_HANDLE;
	VkDevice          device = VK_NULL_HANDLE;
	VkQueue           graphicsQueue = VK_NULL_HANDLE;

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
