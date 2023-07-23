#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>

using std::cout;
using std::runtime_error;

static constexpr uint32_t WIDTH = 800;
static constexpr uint32_t HEIGHT = 600;

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
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void createInstance() {
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
		createInfo.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(&createInfo.enabledExtensionCount);
		createInfo.enabledLayerCount = 0;

		if (!extensionRequirementsSatisfiedInstance(createInfo.enabledExtensionCount, createInfo.ppEnabledExtensionNames)) {
			throw runtime_error("!createInstance: !extensionRequirementsSatisfiedInstance");
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw runtime_error("!createInstance: vkCreateInstance != VK_SUCCESS");
		}
	}

	bool extensionRequirementsSatisfiedInstance(uint32_t numRequiredExtensions, const char* const *requiredExtensionsNames) {
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
		std::vector<VkExtensionProperties> supportedExtensions(numSupportedExtensions);
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
				return false;
			}
		}

		return true;
	}

	GLFWwindow *window;
	VkInstance instance;
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
