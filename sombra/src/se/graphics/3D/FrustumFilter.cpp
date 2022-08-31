#include <glm/gtc/matrix_access.hpp>
#include "se/graphics/core/GraphicsMath.h"
#include "se/graphics/3D/FrustumFilter.h"
#include "se/graphics/3D/Renderable3D.h"

namespace se::graphics {

	FrustumFilter& FrustumFilter::updateFrustum(const glm::mat4& viewProjectionMatrix)
	{
		// See https://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html

		// Left Frustum Plane
		mFrustumPlanes[0] = glm::row(viewProjectionMatrix, 3) + glm::row(viewProjectionMatrix, 0);

		// Right Frustum Plane
		mFrustumPlanes[1] = glm::row(viewProjectionMatrix, 3) - glm::row(viewProjectionMatrix, 0);

		// Bottom Frustum Plane
		mFrustumPlanes[2] = glm::row(viewProjectionMatrix, 3) + glm::row(viewProjectionMatrix, 1);

		// Top Frustum Plane
		mFrustumPlanes[3] = glm::row(viewProjectionMatrix, 3) - glm::row(viewProjectionMatrix, 1);

		// Near Frustum Plane
		mFrustumPlanes[4] = glm::row(viewProjectionMatrix, 3) + glm::row(viewProjectionMatrix, 2);

		// Far Frustum Plane
		mFrustumPlanes[5] = glm::row(viewProjectionMatrix, 3) - glm::row(viewProjectionMatrix, 2);

		// Normalize the planes
		for (auto& plane : mFrustumPlanes) {
			glm::vec3 p2(plane);
			plane *= glm::inversesqrt( glm::dot(p2, p2) );
		}

		return *this;
	}


	bool FrustumFilter::shouldBeRendered(Renderable3D& renderable)
	{
		auto [minAABB, maxAABB] = renderable.getBounds();

		for (auto& plane : mFrustumPlanes) {
			// Pick the AABB positive vertex, aka the furthest vertex along
			// the plane normal
			glm::vec3 pVertex = {
				(plane.x < 0.0f)? minAABB.x : maxAABB.x,
				(plane.y < 0.0f)? minAABB.y : maxAABB.y,
				(plane.z < 0.0f)? minAABB.z : maxAABB.z
			};

			// Calculate the signed distance from the positive vertex to
			// the plane, if it's negative the renderable AABB is outside the
			// frustum and should be culled
			if (signedDistancePlanePoint(plane, pVertex) < 0.0f) {
				return false;
			}
		}

		return true;
	}

}
