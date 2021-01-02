#include "se/app/graphics/Tex3DViewerNode.h"
#include "se/app/loaders/ShaderLoader.h"
#include "se/app/loaders/MeshLoader.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	Tex3DViewerNode::Tex3DViewerNode(const std::string& name, utils::Repository& repository, std::size_t maxSize) :
		BindableRenderNode(name), mMaxSize(maxSize), mMinPosition(0.0f), mMaxPosition(0.0f), mNumInstances(0)
	{
		auto program = repository.find<std::string, graphics::Program>("programTex3DViewer");
		if (!program) {
			program = ShaderLoader::createProgram(
				"res/shaders/vertexTex3DViewer.glsl", nullptr, "res/shaders/fragmentTex3DViewer.glsl"
			);
			repository.add(std::string("programTex3DViewer"), program);
		}
		addBindable(program);

		mModelMatrix = addBindable( std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(0.0f)) );
		addBindable( std::make_shared<graphics::UniformVariableValue<int>>("uTexture3D", *program, kTextureUnit) );
		mMipMapLevel = addBindable( std::make_shared<graphics::UniformVariableValue<float>>("uMipMapLevel", *program) );

		auto tex3DIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("texture3D", this, tex3DIndex) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("texture3D", this, tex3DIndex) );

		auto targetIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", this, targetIndex) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", this, targetIndex) );

		setMipMapLevel( std::log2(static_cast<float>(mMaxSize)) );

		RawMesh rawMesh;
		rawMesh.positions = {
			{ 0.0f, 0.0f, 1.0f },
			{ 1.0f, 0.0f, 1.0f },
			{ 1.0f, 1.0f, 1.0f },
			{ 0.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f },
			{ 1.0f, 1.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f }
		};
		rawMesh.faceIndices = {
			0, 1, 2,	2, 3, 0,
			1, 5, 6,	6, 2, 1,
			7, 6, 5,	5, 4, 7,
			4, 0, 3,	3, 7, 4,
			4, 5, 1,	1, 0, 4,
			3, 2, 6,	6, 7, 3
		};

		mCube = std::make_unique<graphics::Mesh>(MeshLoader::createGraphicsMesh(rawMesh));
	}


	void Tex3DViewerNode::setSceneBounds(const glm::vec3& minPosition, const glm::vec3& maxPosition)
	{
		mMinPosition = minPosition;
		mMaxPosition = maxPosition;

		auto sceneVector = mMaxPosition - mMinPosition;
		glm::vec3 sceneCenter = mMinPosition + 0.5f * sceneVector;

		glm::mat4 T = glm::translate(glm::mat4(1.0f), sceneCenter - 0.5f * sceneVector);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(sceneVector));
		std::static_pointer_cast<graphics::UniformVariableValue<glm::mat4>>( getBindable(mModelMatrix) )->setValue(T * S);
	}


	void Tex3DViewerNode::setMipMapLevel(float mipmapLevel)
	{
		std::static_pointer_cast<graphics::UniformVariableValue<float>>( getBindable(mMipMapLevel) )->setValue(mipmapLevel);

		std::size_t currentSize = static_cast<std::size_t>(std::pow(2.0f, mipmapLevel));
		mNumInstances = currentSize * currentSize * currentSize;
	}


	void Tex3DViewerNode::execute()
	{
		graphics::SetOperation opCulling(graphics::Operation::Culling);		opCulling.bind();
		graphics::SetOperation opDepthTest(graphics::Operation::DepthTest);	opDepthTest.bind();

		bind();
		mCube->bind();
		graphics::GraphicsOperations::drawIndexedInstanced(
			graphics::PrimitiveType::Triangle,
			mCube->getIBO().getIndexCount(), mCube->getIBO().getIndexType(), 0,
			mNumInstances
		);

		opDepthTest.unbind();
		opCulling.unbind();
	}

}
