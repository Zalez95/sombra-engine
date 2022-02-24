#include "se/app/graphics/Tex3DViewerNode.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/io/MeshLoader.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/TypeRefs.h"

namespace se::app {

	Tex3DViewerNode::Tex3DViewerNode(const std::string& name, graphics::Context& context, std::size_t maxSize) :
		BindableRenderNode(name), mMaxSize(maxSize), mMinPosition(0.0f), mMaxPosition(0.0f), mNumInstances(0)
	{
		ProgramRef program;
		auto result = ShaderLoader::createProgram("res/shaders/vertexTex3DViewer.glsl", nullptr, "res/shaders/fragmentTex3DViewer.glsl", context, program);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}
		addBindable(program);

		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uTexture3D", kTextureUnit)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		mModelMatrixIndex = addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		mMipMapLevelIndex = addBindable(
			context.create<graphics::UniformVariableValue<float>>("uMipMapLevel")
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);

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
		rawMesh.indices = {
			0, 1, 2,	2, 3, 0,
			1, 5, 6,	6, 2, 1,
			7, 6, 5,	5, 4, 7,
			4, 0, 3,	3, 7, 4,
			4, 5, 1,	1, 0, 4,
			3, 2, 6,	6, 7, 3
		};

		mCubeIndex = addBindable( MeshLoader::createGraphicsMesh(context, rawMesh) );
	}


	void Tex3DViewerNode::setSceneBounds(const glm::vec3& minPosition, const glm::vec3& maxPosition)
	{
		mMinPosition = minPosition;
		mMaxPosition = maxPosition;

		auto sceneVector = mMaxPosition - mMinPosition;
		glm::vec3 sceneCenter = mMinPosition + 0.5f * sceneVector;

		glm::mat4 T = glm::translate(glm::mat4(1.0f), sceneCenter - 0.5f * sceneVector);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(sceneVector));
		UniformVVRef<glm::mat4>::from( getBindable(mModelMatrixIndex) ).edit([=](auto& uniform) {
			uniform.setValue(T * S);
		});
	}


	void Tex3DViewerNode::setMipMapLevel(float mipmapLevel)
	{
		UniformVVRef<float>::from( getBindable(mMipMapLevelIndex) ).edit([=](auto& uniform) {
			uniform.setValue(mipmapLevel);
		});

		std::size_t currentSize = static_cast<std::size_t>(std::pow(2.0f, mipmapLevel));
		mNumInstances = currentSize * currentSize * currentSize;
	}


	void Tex3DViewerNode::execute(graphics::Context::Query& q)
	{
		auto cube = q.getTBindable( MeshRef::from(getBindable(mCubeIndex)) );

		graphics::SetOperation opCulling(graphics::Operation::Culling);		opCulling.bind();
		graphics::SetOperation opDepthTest(graphics::Operation::DepthTest);	opDepthTest.bind();

		bind(q);
		graphics::GraphicsOperations::drawIndexedInstanced(
			graphics::PrimitiveType::Triangle,
			cube->getIBO()->getIndexCount(),
			cube->getIBO()->getIndexType(),
			0, mNumInstances
		);

		opDepthTest.unbind();
		opCulling.unbind();
	}

}
