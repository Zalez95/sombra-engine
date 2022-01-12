#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/DeferredAmbientRenderer.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/graphics/TypeRefs.h"

namespace se::app {

	DeferredAmbientRenderer::DeferredAmbientRenderer(const std::string& name, graphics::Context& context) :
		BindableRenderNode(name)
	{
		ProgramRef program;
		auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredAmbient.glsl", context, program);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}
		addBindable(program);

		mViewPositionIndex = addBindable(
			context.create<graphics::UniformVariableValue<glm::vec3>>("uViewPosition", glm::vec3(0.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uIrradianceMap", TexUnits::kIrradianceMap)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uPrefilterMap", TexUnits::kPrefilterMap)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uBRDFMap", TexUnits::kBRDFMap)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uPosition", TexUnits::kPosition)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uNormal", TexUnits::kNormal)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uAlbedo", TexUnits::kAlbedo)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uMaterial", TexUnits::kMaterial)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uEmissive", TexUnits::kEmissive)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uSSAO", TexUnits::kSSAO)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);

		auto targetIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", this, targetIndex) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", this, targetIndex) );

		mPlaneIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Mesh>>("plane", this, mPlaneIndex) );

		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("irradiance", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("prefilter", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("brdf", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("position", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("normal", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("albedo", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("material", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("emissive", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("ssao", this, addBindable()) );
	}


	void DeferredAmbientRenderer::setViewPosition(const glm::vec3& position)
	{
		UniformVVRef<glm::vec3>::from( getBindable(mViewPositionIndex) ).edit([=](auto& uniform) {
			uniform.setValue(position);
		});
	}


	void DeferredAmbientRenderer::execute(graphics::Context::Query& q)
	{
		auto plane = q.getTBindable( MeshRef::from(getBindable(mPlaneIndex)) );

		bind(q);
		graphics::GraphicsOperations::drawIndexed(
			graphics::PrimitiveType::Triangle,
			plane->getIBO()->getIndexCount(), plane->getIBO()->getIndexType()
		);
	}

}
