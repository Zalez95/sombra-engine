#include <glm/gtc/matrix_transform.hpp>
#include "se/app/graphics/VoxelizationNode.h"
#include "se/app/io/ShaderLoader.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	VoxelizationNode::VoxelizationNode(const std::string& name, utils::Repository& repository, std::size_t maxVoxels) :
		Renderer3D(name), mMaxVoxels(maxVoxels), mMinPosition(0.0f), mMaxPosition(0.0f)
	{
		auto program = repository.find<std::string, graphics::Program>("programVoxelization");
		if (!program) {
			auto result = ShaderLoader::createProgram(
				"res/shaders/vertexVoxelization.glsl", "res/shaders/geometryVoxelization.glsl",
				"res/shaders/fragmentVoxelization.glsl", program
			);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			repository.add(std::string("programVoxelization"), program);
		}
		addBindable(program);

		for (std::size_t i = 0; i < 3; ++i) {
			mProjectionMatrices[i] = addBindable(
				std::make_shared<graphics::UniformVariableValue<glm::mat4>>(("uProjectionMatrices[" + std::to_string(i) + "]").c_str(), program)
			);
		}

		addBindable( std::make_shared<graphics::UniformVariableValue<int>>("uMaxVoxels", program, static_cast<int>(mMaxVoxels)) );
		addBindable( std::make_shared<graphics::UniformVariableValue<int>>("uVoxelImage", program, kVoxelImageUnit) );

		mVoxelImage = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("texture3D", this, mVoxelImage) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("texture3D", this, mVoxelImage) );
	}


	void VoxelizationNode::setSceneBounds(const glm::vec3& minPosition, const glm::vec3& maxPosition)
	{
		mMinPosition = minPosition;
		mMaxPosition = maxPosition;
	}


	void VoxelizationNode::render()
	{
		int originX, originY;
		std::size_t dimensionsX, dimensionsY;
		graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
		graphics::GraphicsOperations::setViewport(0, 0, mMaxVoxels, mMaxVoxels);
		graphics::GraphicsOperations::setColorMask(false, false, false, false);
		graphics::SetOperation opCulling(graphics::Operation::Culling, false);		opCulling.bind();
		graphics::SetOperation opDepthTest(graphics::Operation::DepthTest, false);	opDepthTest.bind();
		graphics::SetOperation opBlending(graphics::Operation::Blending, false);	opBlending.bind();

		auto sceneVector = mMaxPosition - mMinPosition;
		glm::vec3 sceneCenter = mMinPosition + 0.5f * sceneVector;

		glm::mat4 T = glm::translate(glm::mat4(1.0f), -sceneCenter);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f / sceneVector));
		glm::mat4 RY = glm::rotate(glm::mat4(1.0f),-glm::half_pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 RX = glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
		std::static_pointer_cast<graphics::UniformVariableValue<glm::mat4>>( getBindable(mProjectionMatrices[0]) )
			->setValue(T * RY * S);	// X projection
		std::static_pointer_cast<graphics::UniformVariableValue<glm::mat4>>( getBindable(mProjectionMatrices[1]) )
			->setValue(T * RX * S);	// Y projection
		std::static_pointer_cast<graphics::UniformVariableValue<glm::mat4>>( getBindable(mProjectionMatrices[2]) )
			->setValue(T * S);		// Z projection

		graphics::Renderer3D::render();

		graphics::GraphicsOperations::imageMemoryBarrier();
		std::static_pointer_cast<graphics::Texture>( getBindable(mVoxelImage) )->generateMipMap();

		opCulling.unbind();
		opDepthTest.unbind();
		opBlending.unbind();
		graphics::GraphicsOperations::setColorMask(true, true, true, true);
		graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);
	}

}
