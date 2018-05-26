#ifndef POLYTOPE_H
#define POLYTOPE_H

#include <deque>
#include <vector>
#include "fe/collision/SupportPoint.h"
#include "Triangle.h"

namespace fe { namespace collision {

	class ConvexCollider;


	struct PolytopeFace
	{
		Triangle	triangle;
		bool		obsolete;
		glm::vec3	closestPoint;
		float		distance;
		bool		inside;
		glm::vec3	closestPointBarycentricCoords;

		PolytopeFace(SupportPoint* a, SupportPoint* b, SupportPoint* c, float precision);
	};


	/**
	 * Class Polytope, it's the class that holds the polytope data that the EPA
	 * algorithm must expand.
	 * This class also handles the Polytope creation from the simplex generated
	 * by the GJK algorithm, which also serves as input to the EPA algorithm.
	 */
	class Polytope
	{
	private:	// Attributes
		static constexpr float kEpsilon = 0.0001f;
	public:
		/** The vertices of the Polytope
		 * @note	we use deque instead of vectors to avoid reference
		 *			invalidations when pushing back SupportPoints */
		std::deque<SupportPoint> vertices;

		/** The faces of the Polytope */
		std::deque<PolytopeFace> faces;

	public:		//Functions
		/** Creates a new Polytope from the given simplex points by expanding it
		 * until a tetrahedron is created
		 *
		 * @param	collider1 the first of the colliders needed for creating
		 *			the polytope
		 * @param	collider2 the second of the colliders needed for creating
		 *			the polytope
		 * @param	simplex a vector with the initial simplex points
		 * @note	if the size of the simplex is less than 2 it won't create
		 *			the polytope */
		Polytope(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			const std::vector<SupportPoint>& simplex, float precision
		);

		/** Pushes the face at the front of the faces queue if it's closer to
		 * the origin than the current head, at the back otherwise
		 *
		 * @param	polytopeFace the new face of the Polytope */
		void addFace(const PolytopeFace& polytopeFace);
	private:
		/** Creates a tetrahedron from the points of the given simplex
		 *
		 * @param	collider1 the first of the colliders that we will use to
		 *			create the polytope
		 * @param	collider2 the second of the colliders that we will use to
		 *			create the polytope
		 * @param	the initial simplex used to create the polytope. In this
		 *			case the simplex must be an edge (size = 2) */
		void tetrahedronFromEdge(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			const std::vector<SupportPoint>& simplex, float precision
		);

		/** Creates a tetrahedron from the points of the given simplex
		 *
		 * @param	collider1 the first of the colliders that we will use to
		 *			create the polytope
		 * @param	collider2 the second of the colliders that we will use to
		 *			create the polytope
		 * @param	the initial simplex used to create the polytope. In this
		 *			case the simplex must be a triangle (size = 3) */
		void tetrahedronFromTriangle(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			const std::vector<SupportPoint>& simplex, float precision
		);

		/** Creates the polytope faces from the tetrahedron points */
		void createTetrahedronFaces(float precision);
	};

}}

#endif		// POLYTOPE_H
