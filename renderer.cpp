#include "renderer.h"
#include "h2bParser.h"
#include <Windows.h>
#include <string.h>
#include "Tree.h"

Renderer::Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GVulkanSurface _vlk)
{
	//Vulkan setup
	win = _win;
	vlk = _vlk;
	unsigned int width, height;
	win.GetClientWidth(width);
	win.GetClientHeight(height);

	//Object creation
	timer = XTime();
	

	//WVP matrix init

	iProxy.Create(win);
	cProxy.Create();
	mProxy.Create();
	vProxy.Create();
	
	viewMatrices.resize(2);
	camPositions.resize(2);



	GW::MATH::GVECTORF eye = { 17,  17,  20, 1 };
	camPositions[0] = eye;
	GW::MATH::GVECTORF at = { 0, 0, 0, 1 };
	GW::MATH::GVECTORF up = { 0, 1, 0, 1 };
	mProxy.LookAtLHF(eye, at, up, viewMatrices[0]);

	eye = { 17, 10, 20, 1 };
	camPositions[1] = eye;
	mProxy.LookAtLHF(eye, at, up, viewMatrices[1]);

	float ar;
	vlk.GetAspectRatio(ar);
	mProxy.ProjectionDirectXLHF(65 * DegToRad, ar, 0.1, 100, shaderData.proj);

	//Light Initialization
	lightPos = { -1, -1, 2, 0 };
	vProxy.NormalizeF(lightPos, lightPos);
	lightPos.w = 1;
	lightCol = { 0.9f, 0.9f, 1.0f, 1.0f };
	GW::MATH::GVECTORF ambientLight = { 0.25f, 0.25f, 0.35f, 0 };

	//shaderData.view = view;
	shaderData.lightDir = lightPos;
	shaderData.lightCol = lightCol;
	shaderData.matricies[0] = GW::MATH::GIdentityMatrixF;
	shaderData.ambLight = ambientLight;
	//shaderData.camPos[0] = eye;


	/***************** GEOMETRY INTIALIZATION ******************/
	// Grab the device & physical device so we can allocate some stuff
	VkPhysicalDevice physicalDevice = nullptr;
	vlk.GetDevice((void**)&device);
	vlk.GetPhysicalDevice((void**)&physicalDevice);

	//Load the initial level
	Renderer::LoadLevel("../levels/GameLevel.txt");

	H2B::Parser parser;	
	Renderer::ParseFromFile();

	Renderer::CreateVertexIndexBuffers(physicalDevice);

	//Initialize storage buffer vectors
	unsigned frameCount;
	vlk.GetSwapchainImageCount(frameCount);
	storageData.resize(frameCount);
	storageHandle.resize(frameCount);
	descriptorSets.resize(frameCount);

	for (size_t i = 0; i < frameCount; i++)
	{
		GvkHelper::create_buffer(physicalDevice, device, sizeof(shaderData),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &storageHandle[i], &storageData[i]);
		GvkHelper::write_to_buffer(device, storageData[i], &shaderData, sizeof(shaderData));
	}

	/***************** SHADER INTIALIZATION ******************/
	// Intialize runtime shader compiler HLSL -> SPIRV
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
	shaderc_compile_options_set_invert_y(options, true); // TODO: Part 2i
#ifndef NDEBUG
	shaderc_compile_options_set_generate_debug_info(options);
#endif
	// Create Vertex Shader
	std::string vertexShaderCode = Renderer::ShaderAsString("../shaders/vs.hlsl");
	shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
		compiler, vertexShaderCode.c_str(), strlen(vertexShaderCode.c_str()),
		shaderc_vertex_shader, "main.vert", "main", options);
	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
		std::cout << "Vertex Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
	GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
		(char*)shaderc_result_get_bytes(result), &vertexShader);
	shaderc_result_release(result); // done
	// Create Pixel Shader
	std::string pixelShaderCode = Renderer::ShaderAsString("../shaders/ps.hlsl");
	result = shaderc_compile_into_spv( // compile
		compiler, pixelShaderCode.c_str(), strlen(pixelShaderCode.c_str()),
		shaderc_fragment_shader, "main.frag", "main", options);
	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
		std::cout << "Pixel Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
	GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
		(char*)shaderc_result_get_bytes(result), &pixelShader);
	shaderc_result_release(result); // done
	// Free runtime shader compiler resources
	shaderc_compile_options_release(options);
	shaderc_compiler_release(compiler);

	/***************** PIPELINE INTIALIZATION ******************/
	// Create Pipeline & Layout (Thanks Tiny!)
	VkRenderPass renderPass;
	vlk.GetRenderPass((void**)&renderPass);
	VkPipelineShaderStageCreateInfo stage_create_info[2] = {};
	// Create Stage Info for Vertex Shader
	stage_create_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stage_create_info[0].module = vertexShader;
	stage_create_info[0].pName = "main";
	// Create Stage Info for Fragment Shader
	stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stage_create_info[1].module = pixelShader;
	stage_create_info[1].pName = "main";
	// Assembly State
	VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
	assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	assembly_create_info.primitiveRestartEnable = false;
	// TODO: Part 1e
	// Vertex Input State
	VkVertexInputBindingDescription vertex_binding_description = {};
	vertex_binding_description.binding = 0;
	vertex_binding_description.stride = sizeof(Vertex);
	vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription vertex_attribute_description[3] = {
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 }, //xyz
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3}, //uvw
		{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 6}, //nrm
	};
	VkPipelineVertexInputStateCreateInfo input_vertex_info = {};
	input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	input_vertex_info.vertexBindingDescriptionCount = 1;
	input_vertex_info.pVertexBindingDescriptions = &vertex_binding_description;
	input_vertex_info.vertexAttributeDescriptionCount = 3;
	input_vertex_info.pVertexAttributeDescriptions = vertex_attribute_description;
	// Viewport State (we still need to set this up even though we will overwrite the values)
	VkViewport viewport = {
		0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1
	};
	VkRect2D scissor = { {0, 0}, {width, height} };
	VkPipelineViewportStateCreateInfo viewport_create_info = {};
	viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_create_info.viewportCount = 1;
	viewport_create_info.pViewports = &viewport;
	viewport_create_info.scissorCount = 1;
	viewport_create_info.pScissors = &scissor;
	// Rasterizer State
	VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
	rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_create_info.lineWidth = 1.0f;
	rasterization_create_info.cullMode = VK_CULL_MODE_BACK_BIT; // TURN BACK ON WHEN FIXING THE MODELS
	rasterization_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterization_create_info.depthClampEnable = VK_FALSE;
	rasterization_create_info.depthBiasEnable = VK_FALSE;
	rasterization_create_info.depthBiasClamp = 0.0f;
	rasterization_create_info.depthBiasConstantFactor = 0.0f;
	rasterization_create_info.depthBiasSlopeFactor = 0.0f;
	// Multisampling State
	VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
	multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_create_info.sampleShadingEnable = VK_FALSE;
	multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_create_info.minSampleShading = 1.0f;
	multisample_create_info.pSampleMask = VK_NULL_HANDLE;
	multisample_create_info.alphaToCoverageEnable = VK_FALSE;
	multisample_create_info.alphaToOneEnable = VK_FALSE;
	// Depth-Stencil State
	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
	depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_create_info.depthTestEnable = VK_TRUE;
	depth_stencil_create_info.depthWriteEnable = VK_TRUE;
	depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_create_info.minDepthBounds = 0.0f;
	depth_stencil_create_info.maxDepthBounds = 1.0f;
	depth_stencil_create_info.stencilTestEnable = VK_FALSE;
	// Color Blending Attachment & State
	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
	color_blend_attachment_state.colorWriteMask = 0xF;
	color_blend_attachment_state.blendEnable = VK_FALSE;
	color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
	color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
	color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
	color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
	VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
	color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_create_info.logicOpEnable = VK_FALSE;
	color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
	color_blend_create_info.attachmentCount = 1;
	color_blend_create_info.pAttachments = &color_blend_attachment_state;
	color_blend_create_info.blendConstants[0] = 0.0f;
	color_blend_create_info.blendConstants[1] = 0.0f;
	color_blend_create_info.blendConstants[2] = 0.0f;
	color_blend_create_info.blendConstants[3] = 0.0f;
	// Dynamic State 
	VkDynamicState dynamic_state[2] = {
		// By setting these we do not need to re-create the pipeline on Resize
		VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamic_create_info = {};
	dynamic_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_create_info.dynamicStateCount = 2;
	dynamic_create_info.pDynamicStates = dynamic_state;

	// DescriptorSet Layout Binding
	VkDescriptorSetLayoutBinding descriptor_layout_binding = {};
	descriptor_layout_binding.descriptorCount = 1;
	descriptor_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptor_layout_binding.binding = 0;
	descriptor_layout_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	descriptor_layout_binding.pImmutableSamplers = VK_NULL_HANDLE;

	// DescriptorSet Layout Create Info
	VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info = {};
	descriptor_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout_create_info.flags = 0;
	descriptor_layout_create_info.bindingCount = 1;
	descriptor_layout_create_info.pNext = VK_NULL_HANDLE;
	descriptor_layout_create_info.pBindings = &descriptor_layout_binding;
	vkCreateDescriptorSetLayout(device, &descriptor_layout_create_info, nullptr, &setLayout);

	// Descriptor Pool Size
	VkDescriptorPoolSize descriptor_pool_size = {};
	descriptor_pool_size.descriptorCount = frameCount;
	descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	// Descriptor Pool Create Info
	VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
	descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_create_info.maxSets = frameCount;
	descriptor_pool_create_info.flags = 0;
	descriptor_pool_create_info.pNext = VK_NULL_HANDLE;
	descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;
	descriptor_pool_create_info.poolSizeCount = 1;
	vkCreateDescriptorPool(device, &descriptor_pool_create_info, nullptr, &descriptorPool);

	// Descriptor Allocate Info
	VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
	descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_allocate_info.pSetLayouts = &setLayout;
	descriptor_set_allocate_info.pNext = VK_NULL_HANDLE;
	descriptor_set_allocate_info.descriptorSetCount = 1; //////////////////////////////////////////////////////////////////////////////////////
	descriptor_set_allocate_info.descriptorPool = descriptorPool;
	for (size_t i = 0; i < frameCount; i++)
	{
		vkAllocateDescriptorSets(device, &descriptor_set_allocate_info, &descriptorSets[i]);
	}

	// Descriptor Buffer info (vector because multiple storage buffers)
	std::vector<VkDescriptorBufferInfo> descriptor_buffer_info(frameCount);
	for (size_t i = 0; i < frameCount; i++)
	{
		descriptor_buffer_info[i].buffer = storageHandle[i];
		descriptor_buffer_info[i].offset = 0;
		descriptor_buffer_info[i].range = VK_WHOLE_SIZE;
	}

	// Write Descriptor Set (vector because multiple storage buffers)
	std::vector<VkWriteDescriptorSet> descriptor_set(frameCount);
	for (size_t i = 0; i < frameCount; i++)
	{
		descriptor_set[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_set[i].pNext = VK_NULL_HANDLE;
		descriptor_set[i].pTexelBufferView = VK_NULL_HANDLE;
		descriptor_set[i].dstSet = descriptorSets[i];
		descriptor_set[i].pBufferInfo = &descriptor_buffer_info[i];
		descriptor_set[i].pImageInfo = VK_NULL_HANDLE;
		descriptor_set[i].dstBinding = 0;
		descriptor_set[i].descriptorCount = 1;
		descriptor_set[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptor_set[i].dstArrayElement = 0;
		vkUpdateDescriptorSets(device, 1, &descriptor_set[i], 0, VK_NULL_HANDLE);
	}

	// Push Constants
	VkPushConstantRange push_constant_range = {};
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(PUSH_CONSTANTS);
	push_constant_range.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	// Descriptor pipeline layout
	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = 1;
	pipeline_layout_create_info.pSetLayouts = &setLayout;
	pipeline_layout_create_info.pushConstantRangeCount = 1;
	pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;
	vkCreatePipelineLayout(device, &pipeline_layout_create_info,
		nullptr, &pipelineLayout);
	// Pipeline State... (FINALLY) 
	VkGraphicsPipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.stageCount = 2;
	pipeline_create_info.pStages = stage_create_info;
	pipeline_create_info.pInputAssemblyState = &assembly_create_info;
	pipeline_create_info.pVertexInputState = &input_vertex_info;
	pipeline_create_info.pViewportState = &viewport_create_info;
	pipeline_create_info.pRasterizationState = &rasterization_create_info;
	pipeline_create_info.pMultisampleState = &multisample_create_info;
	pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
	pipeline_create_info.pColorBlendState = &color_blend_create_info;
	pipeline_create_info.pDynamicState = &dynamic_create_info;
	pipeline_create_info.layout = pipelineLayout;
	pipeline_create_info.renderPass = renderPass;
	pipeline_create_info.subpass = 0;
	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
		&pipeline_create_info, nullptr, &pipeline);

	/***************** CLEANUP / SHUTDOWN ******************/
	// GVulkanSurface will inform us when to release any allocated resources
	shutdown.Create(vlk, [&]() {
		if (+shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true)) {
			CleanUp(); // unlike D3D we must be careful about destroy timing
		}
		});
}

void Renderer::Render()
{
	// grab the current Vulkan commandBuffer
	unsigned int currentBuffer;
	vlk.GetSwapchainCurrentImage(currentBuffer);
	VkCommandBuffer commandBuffer;
	vlk.GetCommandBuffer(currentBuffer, (void**)&commandBuffer);
	// what is the current client area dimensions?
	unsigned int width, height;
	win.GetClientWidth(width);
	win.GetClientHeight(height);
	// setup the pipeline's dynamic settings
	VkViewport viewport[2] = {
		{0, 0, static_cast<float>(width), static_cast<float>(height), 0.25f, 1},
		{0, 0, static_cast<float>(width) / 3, static_cast<float>(height) / 3, 0, 0.25f}
	};
	VkRect2D scissor = { {0, 0}, {width, height} };
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	// now we can draw
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &/*models["Grass_Flat"].*/vertexHandle, offsets);
	vkCmdBindIndexBuffer(commandBuffer, /*models["Grass_Flat"].*/indexHandle, 0, VK_INDEX_TYPE_UINT32);
	//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentBuffer], 0, VK_NULL_HANDLE);
	// Draw submeshes
	for (size_t i = 0; i < viewMatrices.size(); i++)
	{
		shaderData.view[i] = viewMatrices[i];
		shaderData.camPos[i] = camPositions[i];
	}

	GvkHelper::write_to_buffer(device, storageData[currentBuffer], &shaderData, sizeof(shaderData));
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentBuffer], 0, VK_NULL_HANDLE);
	for (size_t camera = 0 ; camera < viewMatrices.size(); camera++)
	{
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport[camera]);
		pc.camIndex = camera;

		for (auto item : models)
		{
			for (size_t i = 0; i < item.second.meshCount; i++)
			{
				pc.materialIndex = item.second.materialOffset + item.second.meshes[i].materialIndex;

				pc.modelIndex = item.second.matOffset;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(PUSH_CONSTANTS), &pc);
				vkCmdDrawIndexed(commandBuffer, item.second.meshes[i].drawInfo.indexCount, item.second.matCount, item.second.meshes[i].drawInfo.indexOffset + item.second.indexOffset, item.second.vertOffset, 0);
			}
		}
	}
}

InputData Renderer::GetAllInput() {

	InputData result = {};

	//up/down
	iProxy.GetState(G_KEY_SPACE, result.kSpace);
	iProxy.GetState(G_KEY_LEFTSHIFT, result.kShift);

	cProxy.GetState(0, G_RIGHT_TRIGGER_AXIS, result.cRTrigger);
	cProxy.GetState(0, G_LEFT_TRIGGER_AXIS, result.cLTrigger);

	//fowards/backwards
	iProxy.GetState(G_KEY_W, result.kW);
	iProxy.GetState(G_KEY_S, result.kS);

	cProxy.GetState(0, G_LY_AXIS, result.cLYAxis);

	//right/left
	iProxy.GetState(G_KEY_A, result.kA);
	iProxy.GetState(G_KEY_D, result.kD);

	cProxy.GetState(0, G_LX_AXIS, result.cLXAxis);

	//rotation
	GW::GReturn mResult = iProxy.GetMouseDelta(result.mX, result.mY);

	if (!G_PASS(mResult) || mResult == GW::GReturn::REDUNDANT)
	{
		result.mX = 0;
		result.mY = 0;
	}

	cProxy.GetState(0, G_RY_AXIS, result.cRYAxis);
	cProxy.GetState(0, G_RX_AXIS, result.cRXAxis);

	//open file
	iProxy.GetState(G_KEY_LEFTCONTROL, result.kCtrl);
	iProxy.GetState(G_KEY_O, result.kO);

	//window height and witdh
	win.GetClientHeight(result.winHeight);
	win.GetClientWidth(result.winWidth);

	result.winFOV = 65 * (DegToRad);

	vlk.GetAspectRatio(result.winAR);

	unsigned uiFloatCount = sizeof(InputData) / sizeof(float);
	for (unsigned i = 0; i < uiFloatCount - 4; ++i)
	{
		float fValue = ((float*)&result)[i];
		if (fValue != 0.0f)
			int nDebug = 0;
	}

	return result;
}

void Renderer::UpdateCamera(InputData input, float cameraSpeed) {
	
	//Create proxy
	mProxy.RotateYLocalF(viewMatrices[1], timer.Delta() * .5f, viewMatrices[1]);
	camPositions[1] = viewMatrices[1].row4;
	//Inverse camera
	mProxy.InverseF(viewMatrices[0], viewMatrices[0]);

	float perFrameSpeed = cameraSpeed * timer.Delta();

	//up and down
	viewMatrices[0].row4.y += (input.kSpace - input.kShift + input.cRTrigger - input.cLTrigger) * perFrameSpeed;

	//forward/backward
	float totalZChange = input.kW - input.kS + input.cLYAxis;

	//left/right
	float totalXChange = input.kD - input.kA + input.cLXAxis;

	//apply translations
	GW::MATH::GVECTORF translation = { totalXChange * perFrameSpeed, 0, totalZChange * perFrameSpeed, 0 };
	GW::MATH::GMATRIXF translationMat;
	mProxy.TranslateGlobalF(GW::MATH::GIdentityMatrixF, translation, translationMat);
	mProxy.MultiplyMatrixF(translationMat, viewMatrices[0], viewMatrices[0]);

	//rotation
	float rotSpeed = PI * timer.Delta();

	float pitch = input.winFOV * (input.mY / input.winHeight) - rotSpeed * input.cRYAxis;
	GW::MATH::GMATRIXF updown = GW::MATH::GIdentityMatrixF;
	mProxy.RotationYawPitchRollF(0, pitch, 0, updown);

	float yaw = input.winFOV * input.winAR * (input.mX / input.winWidth) + (rotSpeed * input.cRXAxis);
	GW::MATH::GMATRIXF leftright = GW::MATH::GIdentityMatrixF;
	mProxy.RotationYawPitchRollF(yaw, 0, 0, leftright);

	mProxy.MultiplyMatrixF(updown, viewMatrices[0], viewMatrices[0]); //local pitch
	GW::MATH::GVECTORF camPos = viewMatrices[0].row4; //store position
	mProxy.MultiplyMatrixF(viewMatrices[0], leftright, viewMatrices[0]); //global yaw
	this->camPositions[0] = camPos;
	viewMatrices[0].row4 = camPos;	//restore position

	//Reinverse camera
	mProxy.InverseF(viewMatrices[0], viewMatrices[0]);
}

void Renderer::SignalTimer()
{
	timer.Signal();
}

std::string Renderer::ShaderAsString(const char* shaderFilePath) {
	std::string output;
	unsigned int stringLength = 0;
	GW::SYSTEM::GFile file; file.Create();
	file.GetFileSize(shaderFilePath, stringLength);
	if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
		output.resize(stringLength);
		file.Read(&output[0], stringLength);
	}
	else
		std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
	return output;
}

void Renderer::CreateVertexIndexBuffers(VkPhysicalDevice& physicalDevice)
{
	// Create Vertex Buffer
	// Transfer triangle data to the vertex buffer. (staging would be prefered here)
	GvkHelper::create_buffer(physicalDevice, device, verts.size() * sizeof(Vertex),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexHandle, &vertexData);
	GvkHelper::write_to_buffer(device, vertexData, verts.data(), verts.size() * sizeof(Vertex));

	// Create Index Buffer
	// Transfer index data to index buffer
	GvkHelper::create_buffer(physicalDevice, device, sizeof(int) * indices.size(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &indexHandle, &indexData);
	GvkHelper::write_to_buffer(device, indexData, indices.data(), sizeof(int) * indices.size());
}

GW::GReturn Renderer::LoadLevel(const char* filepath) 
{
	std::ifstream stream(filepath);

	if (!stream.is_open())
	{
		return GW::GReturn::FILE_NOT_FOUND;
	}

	while (!stream.eof())
	{
		std::string line;

		std::getline(stream, line);

		if (!strcmp(line.c_str(), "MESH")) //checks to see if we found the MESH keyword
		{
			std::getline(stream, line);

			line = line.substr(0, line.find('.'));
			std::string meshName = line;
			GW::MATH::GMATRIXF worldMatrix;
		
			for (size_t i = 0; i < 4; i++)
			{
				//get line and trim it to only numbers or ,
				std::getline(stream, line);
				size_t start = line.find('(') + 1;
				line = line.substr(start, line.find(')') - start);
				//get each element of data
				for (size_t j = 0; j < 4; j++)
				{
					size_t end = line.find(',');
					worldMatrix.data[(i << 2) + j] = std::stof(line.substr(0, end));
					line = line.substr(end + 1);
				}
			}
			if (models.find(meshName) == models.end())
			{

				GW::MATH::GVECTORF bbCenter = {};
				GW::MATH::GVECTORF bbExtents = {};
				{
					std::getline(stream, line);
					size_t start = line.find('(') + 1;
					line = line.substr(start, line.find(')') - start);
					for (size_t j = 0; j < 3; j++)
					{
						size_t end = line.find(',');
						bbCenter.data[j] = std::stof(line.substr(0, end));
						line = line.substr(end + 1);
					}
				}
				{
					std::getline(stream, line);
					size_t start = line.find('(') + 1;
					line = line.substr(start, line.find(')') - start);
					for (size_t j = 0; j < 3; j++)
					{
						size_t end = line.find(',');
						bbExtents.data[j] = (std::stof(line.substr(0, end))) * 0.5f;
						line = line.substr(end + 1);
					}
				}
				models[meshName].boundBoxCenter = bbCenter;
				vProxy.AddVectorF(bbCenter, bbExtents, models[meshName].boundBoxExtents);
			}
			models[meshName].matrices.push_back(worldMatrix);
		}
	}

	return GW::GReturn::SUCCESS;
}

void Renderer::ParseFromFile()
{
	H2B::Parser parser;
	std::vector<GW::MATH::GMATRIXF> worlds;
	std::vector<OBJATTRIBUTES> materials;
	for (auto& item : models)
	{
		//read the h2b file
		bool parseSuccess = parser.Parse(std::string("../assets/" + item.first + ".h2b").c_str());

		//if failed, continue
		if (!parseSuccess)
			continue;

		//copy vertices
		item.second.vertOffset = verts.size();

		for (size_t i = 0; i < parser.vertices.size(); i++)
		{
			verts.push_back((Vertex&)parser.vertices[i]);
		}
		item.second.vertCount = verts.size() - item.second.vertOffset;

		//copy indices
		item.second.indexOffset = indices.size();
		indices.insert(indices.end(), parser.indices.begin(), parser.indices.end());
		item.second.indexCount = indices.size() - item.second.indexOffset;

		//copy counts
		item.second.materialCount = parser.materialCount;
		item.second.meshCount = parser.meshCount;

		// Copy mateirals
		item.second.materialOffset = materials.size();
		for (size_t i = 0; i < item.second.materialCount; i++)
		{
			OBJATTRIBUTES att = (OBJATTRIBUTES&)parser.materials[i].attrib;

			if (att.Ns == 0)
			{
				att.Ns = 2048;
			}

			materials.push_back(att);

		}

		// Copy mesh and batch
		for (size_t i = 0; i < item.second.meshCount; item.second.meshes.push_back(parser.meshes[i]), i++);
		for (size_t i = 0; i < parser.batches.size(); item.second.batches.push_back(parser.batches[i]), i++);

		//Copy world matrices
		item.second.matOffset = worlds.size();
		for (size_t i = 0; i < item.second.matrices.size(); worlds.push_back(item.second.matrices[i]), i++);
		item.second.matCount = item.second.matrices.size();
	}

	//Copy materials and world matrices to the shader data
	for (size_t i = 0; i < worlds.size(); shaderData.matricies[i] = worlds[i], i++);
	for (size_t i = 0; i < materials.size(); shaderData.materials[i] = materials[i], i++);
}

void Renderer::Changelevel(InputData input)
{
	if (input.kO == 0 || input.kCtrl == 0)
	{
		return;
	}
	std::string newFilePath = Renderer::OpenFile();
	if (newFilePath == "")
	{
		return;
	}
	vkDeviceWaitIdle(device);
	vkDestroyBuffer(device, vertexHandle, nullptr);
	vkFreeMemory(device, vertexData, nullptr);
	vkDestroyBuffer(device, indexHandle, nullptr);
	vkFreeMemory(device, indexData, nullptr);

	
	verts.clear();
	indices.clear();
	models.clear();

	VkPhysicalDevice physicalDevice = nullptr;
	vlk.GetPhysicalDevice((void**)&physicalDevice);
	
	Renderer::LoadLevel(newFilePath.c_str());
	Renderer::ParseFromFile();
	Renderer::CreateVertexIndexBuffers(physicalDevice);
	

}

std::string Renderer::OpenFile()
{
	OPENFILENAME ofn = { 0 };
	TCHAR szFile[260] = { 0 };
	// Initialize remaining fields of OPENFILENAME structure
	ofn.lStructSize = sizeof(ofn);
	//ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"All\0*.*\0Text\0*.txt\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		// use ofn.lpstrFile here
		std::wstring wide = ofn.lpstrFile;
		return std::string(wide.begin(), wide.end());
	}
	return "";	
}

void Renderer::CleanUp()
{
	// wait till everything has completed
	vkDeviceWaitIdle(device);
	// Release allocated buffers, shaders & pipeline
	vkDestroyBuffer(device, indexHandle, nullptr);
	vkFreeMemory(device, indexData, nullptr);
	for (size_t i = 0; i < storageData.size(); i++)
	{
		vkDestroyBuffer(device, storageHandle[i], nullptr);
		vkFreeMemory(device, storageData[i], nullptr);
	}

	vkDestroyBuffer(device, vertexHandle, nullptr);
	vkFreeMemory(device, vertexData, nullptr);
	vkDestroyShaderModule(device, vertexShader, nullptr);
	vkDestroyShaderModule(device, pixelShader, nullptr);
	vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyPipeline(device, pipeline, nullptr);
}