#ifndef SSAO_NODE_H
#define SSAO_NODE_H

#include "se/graphics/BindableRenderNode.h"

namespace se::app {

	/**
	 * Class SSAONode, It's the node used for calculatin the Screen Space
	 * Ambient Occlusion based on the gBuffer contents.
	 * It has a "target" input and output FrameBuffer where the SSAO
	 * texture will be written to. It also has a "position" and "normal"
	 * texture inputs and a "plane" input where a plane Mesh must be attached
	 * for rendering
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
		/** The index of the plane Mesh used for rendering */
		std::size_t mPlaneIndex;

		/** The index of the uniform variable that holds the Camera view
		 * matrix */
		std::size_t mViewMatrixIndex;

		/** The index of the uniform variable that holds the Camera projection
		 * matrix */
		std::size_t mProjectionMatrixIndex;

	public:		// Functions
		/** Creates a new SSAONode
		 *
		 * @param	name the name of the RenderNode
		 * @param	context the Context used for creating the RenderNode
		 *			Bindables */
		SSAONode(const std::string& name, graphics::Context& context);

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

		/** @copydoc graphics::RenderNode::execute(graphics::Context::Query&) */
		virtual void execute(graphics::Context::Query& q) override;
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
