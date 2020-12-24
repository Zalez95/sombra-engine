#ifndef FRUSTUM_RENDERER_3D_H
#define FRUSTUM_RENDERER_3D_H

#include <array>
#include <glm/glm.hpp>
#include "../../graphics/3D/Renderer3D.h"
#include "../../graphics/3D/Renderable3D.h"

namespace se::app {

	/**
	 * Class FrustumRenderer3D, it's a Renderer3D that can filter Renderable3Ds
	 * outside the field of view of the camera so they won't be submitted for
	 * rendering for improving the performance
	 */
	class FrustumRenderer3D : public graphics::Renderer3D
	{
	private:	// Attributes
		/** The frustum planes */
		std::array<glm::vec4, 6> mFrustumPlanes;

	public:		// Functions
		/** Creates a new FrustumRenderer3D
		 *
		 * @param	name the name of the new Renderer3D */
		FrustumRenderer3D(const std::string& name) :
			Renderer3D(name), mFrustumPlanes{} {};

		/** Class destructor */
		virtual ~FrustumRenderer3D() = default;

		/** Updates the frustum needed for the frustum culling
		 *
		 * @param	viewProjectionMatrix the view projection matrix of the
		 *			camera used for calculating the frustum */
		virtual FrustumRenderer3D& updateFrustum(
			const glm::mat4& viewProjectionMatrix
		);

		/** @copydoc graphics::Renderer::submit(Renderable&, Pass&) */
		virtual void submit(
			graphics::Renderable& renderable, graphics::Pass& pass
		) override;
	};

}

#endif		// FRUSTUM_RENDERER_3D_H
