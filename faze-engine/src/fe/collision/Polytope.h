#ifndef POLYTOPE_H
#define POLYTOPE_H

#include <array>
#include "fe/collision/SupportPoint.h"
#include "fe/collision/HalfEdgeMesh.h"

namespace fe { namespace collision {

	class ConvexCollider;


	/**
	 * Struct FaceDistanceData, it stores the distance data of a Face to
	 * the origin
	 */
	struct FaceDistanceData
	{
		/** The 3D coordinates of the closest point in the Face to the
		 * origin */
		glm::vec3 closestPoint;

		/** The distance of the closest Point to the origin */
		float distance;

		/** If the calculated closest point is inside the face or not */
		bool inside;

		/** The barycentric coordinates of the cloest point in the face to
		 * the origin */
		glm::vec3 closestPointBarycentricCoords;
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
		static constexpr float sKEpsilon = 0.0001f;

		/** The precision of the calculated projections */
		float mPrecision;

		/** The Half-Edge Mesh with the convex hull of the current Mesh */
		HalfEdgeMesh mMesh;

		/** Maps each HalfEdgeMesh HEFace with its normal vector */
		std::map<int, glm::vec3> mFaceNormals;

		/** Maps each HalfEdgeMesh HEFace with its distance data */
		std::map<int, FaceDistanceData> mFaceDistances;

		/** Maps each HalfEdgeMesh HEVertex with its SupportPoint */
		std::map<int, SupportPoint> mVertexSupportPoints;

	public:		//Functions
		/** Creates a new Polytope from the given simplex points by expanding it
		 * until a tetrahedron is created
		 *
		 * @param	simplex the 4 initial simplex points
		 * @param	precision the precision of the projected points of the
		 *			polytope
		 * @throw	runtime_error if the simplex doesn't have 4 points */
		Polytope(const std::array<SupportPoint, 4>& simplex, float precision);

		/** @return	the HalfEdgeMesh of the Polytope */
		const HalfEdgeMesh& getMesh() const { return mMesh; };

		/** @return	the map with the normal vectors of the HEFaces of the
		 *			Polytope's HalfEdgeMesh */
		const std::map<int, glm::vec3>& getNormalsMap() const
		{ return mFaceNormals; };

		/** Returns the SupportPoint of the given Polytope HEVertex
		 *
		 * @param	iVertex the index of the HEVertex in the Polytope
		 * @return	the SupportPoint of the HEVertex
		 * @throw	runtime_error if the HEVertex isn't found */
		const SupportPoint& getSupportPoint(int iVertex) const;

		/** Returns the normal vector of the given Polytope HEFace
		 *
		 * @param	iFace the index of the HEFace in the Polytope
		 * @return	the normal vector of the HEFace
		 * @throw	runtime_error if the HEFace isn't found */
		const glm::vec3& getNormal(int iFace) const;

		/** Returns the FaceDistanceData of the given Polytope HEFace
		 *
		 * @param	iFace the index of the HEFace in the Polytope
		 * @return	the FaceDistanceData of the HEFace
		 * @throw	runtime_error if the HEFace isn't found */
		const FaceDistanceData& getDistanceData(int iFace) const;

		/** Creates a new HEVertex from the given SupportPoint and adds it to
		 * the Polytope Mesh
		 *
		 * @param	sp the SupportPoint used to create the HEVertex */
		int addVertex(const SupportPoint& sp);

		/** Creates a HEFace and pushes it at the front of the faces queue if
		 * it's closer to the origin than the current head, at the back
		 * otherwise
		 *
		 * @param	faceIndices the indices of the new face's vertices */
		int addFace(const std::array<int, 3>& faceIndices);

		/** Removes the given HEFace and its data from the Polytope
		 *
		 * @param	iFace the index of the HEFace to remove */
		void removeFace(int iFace);
	};

}}

#endif		// POLYTOPE_H
