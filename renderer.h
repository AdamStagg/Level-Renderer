#pragma once
// minimalistic code to draw a single triangle, this is not part of the API.
// TODO: Part 1b
#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
//#include "FSLogo.h"
#include "XTime.h"
#include "vulkan/vulkan_core.h"
#include "Defines.h"
#include "../Gateware/Gateware.h"
#include "InputData.h"
#include "RendererStructs.h"
#include "Tree.h"

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
	VkBuffer vertexLineHandle = nullptr;
	VkDeviceMemory vertexData = nullptr;
	VkDeviceMemory vertexLineData = nullptr;
	VkBuffer indexHandle = nullptr;
	VkBuffer indexLineHandle = nullptr;
	VkDeviceMemory indexData = nullptr;
	VkDeviceMemory indexLineData = nullptr;
	//std::map<std::string, std::vector<GW::MATH::GMATRIXF>> meshes;
	std::map<std::string, modelInfo> models;
	Tree collisionHierarchy;

	GW::INPUT::GInput iProxy;
	GW::INPUT::GController cProxy;
	GW::MATH::GMatrix mProxy;
	GW::MATH::GVector vProxy;
	GW::MATH::GCollision gProxy;

	std::vector<VkBuffer> storageHandle;
	std::vector<VkDeviceMemory> storageData;

	std::vector<Vertex> verts;
	std::vector<Vertex> vertsLine;
	std::vector<int> indices;
	std::vector<int> indicesLine;

	VkShaderModule vertexShader = nullptr;
	VkShaderModule pixelShader = nullptr;
	VkShaderModule vertexShaderCollisions = nullptr;
	VkShaderModule pixelShaderCollisions = nullptr;
	// pipeline settings for drawing (also required)
	VkPipeline pipeline = nullptr;
	VkPipeline pipelineLine = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	VkDescriptorSetLayout setLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
		
	XTime timer;
	std::vector<GW::MATH::GMATRIXF> viewMatrices;
	std::vector<GW::MATH::GVECTORF> camPositions;
	GW::MATH::GVECTORF lightPos, lightCol;
	SHADER_MODEL_DATA shaderData;
	PUSH_CONSTANTS pc;
public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GVulkanSurface _vlk);
	void Render();
	InputData GetAllInput();
	GW::GReturn CheckCollision(GW::MATH::GVECTORF cameraPosition);
	void Changelevel(InputData input);
	void UpdateCamera(InputData, float);
	void SignalTimer();
private:
	// Load a shader file as a string of characters.
	GW::GReturn CheckCollision(GW::MATH::GVECTORF camPos, Tree::Node* currNode, int depth);
	std::string ShaderAsString(const char* shaderFilePath);
	void CreateVertexIndexBuffers(VkPhysicalDevice& physicalDevice);
	GW::GReturn LoadLevel(const char* _filepath);
	void Renderer::ParseFromFile();
	std::string OpenFile();
	void CleanUp();
};