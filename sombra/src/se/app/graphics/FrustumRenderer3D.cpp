#include <glm/gtc/matrix_access.hpp>
#include "se/app/graphics/FrustumRenderer3D.h"
#include "se/utils/MathUtils.h"

namespace se::app {

	FrustumRenderer3D& FrustumRenderer3D::updateFrustum(const glm::mat4& viewProjectionMatrix)
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


	void FrustumRenderer3D::submit(graphics::Renderable& renderable, graphics::Pass& pass)
	{
		auto renderable3D = dynamic_cast<graphics::Renderable3D*>(&renderable);
		auto [minAABB, maxAABB] = renderable3D->getBounds();

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
			if (utils::signedDistancePlanePoint(plane, pVertex) < 0.0f) {
				return;
			}
		}

		Renderer3D::submit(renderable, pass);
	}

}
