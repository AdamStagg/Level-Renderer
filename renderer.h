#pragma once
// minimalistic code to draw a single triangle, this is not part of the API.
// TODO: Part 1b
#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#include "FSLogo.h"
#include "XTime.h"
#include "vulkan/vulkan_core.h"
#include "Defines.h"
#include "../Gateware/Gateware.h"
#include "InputData.h"
#include "RendererStructs.h"

#ifdef _WIN32 // must use MT platform DLL libraries on windows
	#pragma comment(lib, "shaderc_combined.lib") 
#endif

const double PI = 3.1415926535897932384626433832795028841971;
const double DegToRad = PI / 180.0;


// Simple Vertex Shader
extern const char* vertexShaderSource;
// Simple Pixel Shader
extern const char* pixelShaderSource;

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GVulkanSurface vlk;
	GW::CORE::GEventReceiver shutdown;
	
	// what we need at a minimum to draw a triangle
	VkDevice device = nullptr;
	VkBuffer vertexHandle = nullptr;
	VkDeviceMemory vertexData = nullptr;
	VkBuffer indexHandle = nullptr;
	VkDeviceMemory indexData = nullptr;
	//std::map<std::string, std::vector<GW::MATH::GMATRIXF>> meshes;
	std::map<std::string, model> models;

	std::vector<VkBuffer> storageHandle;
	std::vector<VkDeviceMemory> storageData;

	VkShaderModule vertexShader = nullptr;
	VkShaderModule pixelShader = nullptr;
	// pipeline settings for drawing (also required)
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	VkDescriptorSetLayout setLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
		
	XTime timer;
	GW::MATH::GMATRIXF world, view, proj;
	GW::MATH::GVECTORF lightPos, lightCol;
	SHADER_MODEL_DATA shaderData;
	PUSH_CONSTANTS pc;
public:

	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GVulkanSurface _vlk);
	void Render();
	InputData GetAllInput();
	void UpdateCamera(InputData, float);
	void SignalTimer();
private:
	// Load a shader file as a string of characters.
	std::string ShaderAsString(const char* shaderFilePath);
	GW::GReturn LoadLevel(const char* _filepath);
	void CleanUp();
};