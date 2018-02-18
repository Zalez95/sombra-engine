#ifndef POLYTOPE_H
#define POLYTOPE_H

#include <list>
#include <vector>
#include <glm/glm.hpp>
#include "SupportPoint.h"

namespace fe { namespace collision {

	class ConvexCollider;


	struct Edge
	{
		SupportPoint* mP1;
		SupportPoint* mP2;

		Edge(SupportPoint* p1, SupportPoint* p2) : mP1(p1), mP2(p2) {};

		~Edge() {};

		bool operator==(Edge e) const
		{ return ((mP1 == e.mP1) && (mP2 == e.mP2)); };
	};


	struct Triangle
	{
		Edge mAB;
		Edge mBC;
		Edge mCA;
		glm::vec3 mNormal;

		Triangle(SupportPoint* a, SupportPoint* b, SupportPoint* c) :
			mAB(a, b), mBC(b, c), mCA(c, a)
		{
			mNormal = glm::normalize(glm::cross(
				mAB.mP2->getCSOPosition() - mAB.mP1->getCSOPosition(),
				mCA.mP2->getCSOPosition() - mCA.mP1->getCSOPosition()
			));
		};

		~Triangle() {};
	};


	struct Polytope
	{
		static const float sKEpsilon;

		std::list<SupportPoint> mVertices;
		std::list<Triangle> mFaces;

		Polytope(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			std::vector<SupportPoint>& simplex
		);

		~Polytope() {};
	};

}}

#endif		// POLYTOPE_H
