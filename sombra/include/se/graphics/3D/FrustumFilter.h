#ifndef FRUSTUM_FILTER_H
#define FRUSTUM_FILTER_H

#include <array>
#include <glm/glm.hpp>
#include "Renderer3D.h"

namespace se::graphics {

	/**
	 * Class FrustumFilter, it's a Renderer3D Filter used for testing if
	 * Renderable3Ds are outside the field of view of the camera so they won't
	 * be submitted for rendering for improving the performance
	 */
	class FrustumFilter : public Renderer3D::Filter
	{
	private:	// Attributes
		/** The frustum planes */
		std::array<glm::vec4, 6> mFrustumPlanes = {};

	public:		// Functions
		/** Updates the frustum needed for the frustum culling
		 *
		 * @param	viewProjectionMatrix the view projection matrix of the
		 *			camera used for calculating the frustum */
		virtual FrustumFilter& updateFrustum(
			const glm::mat4& viewProjectionMatrix
		);

		/** @copydoc Renderer3D::Filter::shouldBeRendered(Renderable3D&) */
		virtual bool shouldBeRendered(Renderable3D& renderable) override;
	};

}

#endif		// FRUSTUM_FILTER_H
