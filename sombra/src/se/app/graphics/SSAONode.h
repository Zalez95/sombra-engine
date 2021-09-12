#ifndef SSAO_NODE_H
#define SSAO_NODE_H

#include "se/graphics/BindableRenderNode.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/3D/Mesh.h"
#include "se/app/Repository.h"

namespace se::app {

	/**
	 * Class SSAONode, It's the node used for calculatin the Screen Space
	 * Ambient Occlusion based on the gBuffer contents.
	 * It has a "target" input and output FrameBuffer where the SSAO
	 * texture will be written to. It also has a "position" and "normal"
	 * texture inputs.
	 */
	class SSAONode : public graphics::BindableRenderNode
	{
	public:		// Nested types
		struct TexUnits
		{
			static constexpr int kPosition		= 0;
			static constexpr int kNormal		= 1;
			static constexpr int kRotationNoise	= 2;
		};

	private:	// Attributes
		/** The program used by the SSAONode */
		Repository::ResourceRef<graphics::Program> mProgram;

		/** The plane used for rendering */
		Repository::ResourceRef<graphics::Mesh> mPlane;

		/** The rotation kernel texture used for creating noise */
		Repository::ResourceRef<graphics::Texture> mRotationNoiseTexture;

		/** The uniform variable that holds the Camera view matrix */
		std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>
			mViewMatrix;

		/** The uniform variable that holds the Camera projection matrix */
		std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>
			mProjectionMatrix;

	public:		// Functions
		/** Creates a new SSAONode
		 *
		 * @param	name the name of the RenderNode
		 * @param	repository the Repository that holds the programs */
		SSAONode(const std::string& name, Repository& repository);

		/** Class destructor */
		virtual ~SSAONode() = default;

		/** Sets the view matrix of the camera
		 *
		 * @param	viewMatrix the new view matrix of the Camera */
		void setViewMatrix(const glm::mat4& viewMatrix);

		/** Sets the projection matrix of the camera
		 *
		 * @param	projectionMatrix the new projection matrix of the Camera */
		void setProjectionMatrix(const glm::mat4& projectionMatrix);

		/** Executes the current RenderNode */
		virtual void execute() override;
	private:
		/** Calculates random samples inside an hemisphere
		 *
		 * @param	kernel the vector where the samples will added
		 * @param	numSamples the number of samples to add */
		static void addHemisphereSamples(
			std::vector<glm::vec3>& kernel, std::size_t numSamples
		);

		/** Calculates random rotation vectors
		 *
		 * @param	kernel the vector where the vectors will added
		 * @param	numVectors the number of vectors to add */
		static void addRotationNoise(
			std::vector<glm::vec3>& kernel, std::size_t numVectors
		);
	};

}

#endif		// SSAO_NODE_H
