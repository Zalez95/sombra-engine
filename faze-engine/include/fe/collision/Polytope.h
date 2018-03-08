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
		SupportPoint* p1;
		SupportPoint* p2;

		Edge(SupportPoint* p1, SupportPoint* p2) : p1(p1), p2(p2) {};

		~Edge() {};

		bool operator==(Edge e) const
		{ return ((p1 == e.p1) && (p2 == e.p2)); };
	};


	struct Triangle
	{
		Edge ab;
		Edge bc;
		Edge ca;
		glm::vec3 normal;

		Triangle(SupportPoint* a, SupportPoint* b, SupportPoint* c) :
			ab(a, b), bc(b, c), ca(c, a)
		{
			normal = glm::normalize(glm::cross(
				ab.p2->getCSOPosition() - ab.p1->getCSOPosition(),
				ca.p2->getCSOPosition() - ca.p1->getCSOPosition()
			));
		};

		~Triangle() {};
	};


	struct Polytope
	{
		static const float sKEpsilon;

		std::list<SupportPoint> vertices;
		std::list<Triangle> faces;

		Polytope(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			std::vector<SupportPoint>& simplex
		);

		~Polytope() {};
	};

}}

#endif		// POLYTOPE_H
