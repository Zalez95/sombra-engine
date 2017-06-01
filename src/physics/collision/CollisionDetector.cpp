#include "CollisionDetector.h"
#include "Collider.h"
#include "Plane.h"
#include "BoundingSphere.h"
#include "BoundingBox.h"

namespace physics {

	std::vector<Contact> CollisionDetector::collide(
		const Collider* collider1,
		const Collider* collider2
	) const
	{
		std::vector<Contact> ret;

		if (auto bs1 = dynamic_cast<const BoundingSphere*>(collider1)) {
			if (const BoundingSphere* bs2 = dynamic_cast<const BoundingSphere*>(collider2)) {
				ret = collideSpheres(bs1, bs2);
			}
			else if (const BoundingBox* box1 = dynamic_cast<const BoundingBox*>(collider2)) {
				ret = collideSphereAndBox(bs1, box1);
			}
			else if (const Plane* p2 = dynamic_cast<const Plane*>(collider2)) {
				ret = collideSphereAndPlane(bs1, p2);
			}
		}
		else if (const BoundingBox* box1 = dynamic_cast<const BoundingBox*>(collider1)) {
			if (const BoundingSphere* bs1 = dynamic_cast<const BoundingSphere*>(collider2)) {
				ret = collideSphereAndBox(bs1, box1);
			}
			else if (const BoundingBox* box2 = dynamic_cast<const BoundingBox*>(collider2)) {
				ret = collideBoxes(box1, box2);
			}
			else if (const Plane* p1 = dynamic_cast<const Plane*>(collider2)) {
				ret = collideBoxAndPlane(box1, p1);
			}
		}
		else if (const Plane* p1 = dynamic_cast<const Plane*>(collider1)) {
			if (const BoundingSphere* bs1 = dynamic_cast<const BoundingSphere*>(collider2)) {
				ret = collideSphereAndPlane(bs1, p1);
			}
			else if (const BoundingBox* box1 = dynamic_cast<const BoundingBox*>(collider2)) {
				ret = collideBoxAndPlane(box1, p1);
			}
		}

		return ret;
	}

// Private functions
	std::vector<Contact> CollisionDetector::collideSpheres(
		const BoundingSphere* sphere1,
		const BoundingSphere* sphere2
	) const
	{
		std::vector<Contact> ret;

		if (!sphere1 || !sphere2) { return ret; }

		glm::vec3 centersLine	= sphere1->getCenter() - sphere2->getCenter();
		float distance			= glm::length(centersLine);

		if (distance >= sphere1->getRadius() + sphere2->getRadius()) { return ret; }

		glm::vec3 position		= sphere1->getCenter() + 0.5f * centersLine;
		glm::vec3 normal		= centersLine / distance;
		float penetration		= sphere1->getRadius() + sphere2->getRadius() - distance;
		
		ret.push_back(Contact(penetration, position, normal));

		return ret;
	}


	std::vector<Contact> CollisionDetector::collideSphereAndPlane(
		const BoundingSphere* sphere,
		const Plane* plane
	) const
	{
		std::vector<Contact> ret;

		if (!sphere || !plane) { return ret; }
		
		float centerDistance	= glm::dot(plane->getNormal(), sphere->getCenter()) + plane->getDistance();

		if (centerDistance >= sphere->getRadius()) { return ret; }

		glm::vec3 position		= sphere->getCenter() - plane->getNormal() * (centerDistance + sphere->getRadius());
		glm::vec3 normal		= plane->getNormal();
		float penetration		= sphere->getRadius() - centerDistance;
		
		ret.push_back(Contact(penetration, position, normal));

		return ret;
	}


	std::vector<Contact> CollisionDetector::collideBoxes(
		const BoundingBox* box1,
		const BoundingBox* box2
	) const
	{
		std::vector<Contact> ret;

		if (!box1 || !box2) { return ret; }

		return ret;
	}


	std::vector<Contact> CollisionDetector::collideBoxAndPlane(
		const BoundingBox* box,
		const Plane* plane
	) const
	{
		std::vector<Contact> ret;

		if (!box || !plane) { return ret; }

		glm::vec3 planeNormal	= plane->getNormal();
		float planeDistance		= plane->getDistance();

		for (glm::vec3 vertex : box->getVertices()) {
			float distance		= glm::dot(vertex, planeNormal);

			if (distance < planeDistance) {
				float penetration	= planeDistance - distance;
				// The contact is located at the vertex minus the
				// penetration in the direction of the Plane's normal
				glm::vec3 position	= vertex - penetration * planeNormal;
				ret.push_back(Contact(penetration, position, planeNormal));
			}
		}

		return ret;
	}


	std::vector<Contact> CollisionDetector::collideSphereAndBox(
		const BoundingSphere* sphere,
		const BoundingBox* box
	) const
	{
		std::vector<Contact> ret;

		if (!sphere || !box) { return ret; }

		return ret;
	}

}
