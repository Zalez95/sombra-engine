#include <glm/gtc/matrix_transform.hpp>
#include "se/app/graphics/VoxelizationNode.h"
#include "se/app/io/ShaderLoader.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/TypeRefs.h"

namespace se::app {

	VoxelizationNode::VoxelizationNode(const std::string& name, graphics::Context& context, std::size_t maxVoxels) :
		Renderer3D(name), mMaxVoxels(maxVoxels), mMinPosition(0.0f), mMaxPosition(0.0f)
	{
		ProgramRef program;
		auto result = ShaderLoader::createProgram(
			"res/shaders/vertexVoxelization.glsl", "res/shaders/geometryVoxelization.glsl",
			"res/shaders/fragmentVoxelization.glsl", context, program
		);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}
		addBindable(program);

		for (std::size_t i = 0; i < 3; ++i) {
			mProjectionMatrices[i] = addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrices[" + std::to_string(i) + "]")
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			);
		}

		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uMaxVoxels", static_cast<int>(mMaxVoxels))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uVoxelImage", kVoxelImageUnit)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);

		mVoxelImage = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("texture3D", this, mVoxelImage) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("texture3D", this, mVoxelImage) );
	}


	void VoxelizationNode::setSceneBounds(const glm::vec3& minPosition, const glm::vec3& maxPosition)
	{
		mMinPosition = minPosition;
		mMaxPosition = maxPosition;
	}


	void VoxelizationNode::execute(graphics::Context::Query& q)
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
		q.getTBindable( UniformVVRef<glm::mat4>::from(getBindable(mProjectionMatrices[2])) )->setValue(T * RY * S);	// X projection
		q.getTBindable( UniformVVRef<glm::mat4>::from(getBindable(mProjectionMatrices[1])) )->setValue(T * RX * S);	// Y projection
		q.getTBindable( UniformVVRef<glm::mat4>::from(getBindable(mProjectionMatrices[2])) )->setValue(T * S);		// Z projection

		graphics::Renderer3D::execute(q);

		graphics::GraphicsOperations::imageMemoryBarrier();
		q.getTBindable( TextureRef::from(getBindable(mVoxelImage)) )->generateMipMap();

		opCulling.unbind();
		opDepthTest.unbind();
		opBlending.unbind();
		graphics::GraphicsOperations::setColorMask(true, true, true, true);
		graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);
	}

}
