#ifndef COMPOSITE_COLLIDER_H
#define COMPOSITE_COLLIDER_H

#include <memory>
#include "ConcaveCollider.h"

namespace se::collision {

	/**
	 * Class CompositeCollider, it's a Collider with a concave shape stored as
	 * a hierarchy of child Colliders following the Composite pattern.
	 * The child colliders doesn't interact between each other, only with the
	 * other ones.
	 */
	class CompositeCollider : public ConcaveCollider
	{
	private:	// Nested types
		using ColliderUPtr = std::unique_ptr<Collider>;

	private:	// Attributes
		/** The multiple pieces in which the CompositeCollider is splited */
		std::vector<ColliderUPtr> mParts;

		/** The transformation matrix of the CompositeCollider */
		glm::mat4 mTransformsMatrix;

		/** The AABB of the CompositeCollider */
		AABB mAABB;

	public:		// Functions
		/** Creates a new CompositeCollider located at the origin of coordinates
		 * from the given convex pieces
		 *
		 * @param	parts the multiple colliders in which the CompositeCollider
		 *			is splitted */
		CompositeCollider(std::vector<ColliderUPtr> parts = {});
		CompositeCollider(const CompositeCollider& other);
		CompositeCollider(CompositeCollider&& other) = default;

		/** Class destructor */
		~CompositeCollider() = default;

		/** Assignment operator */
		CompositeCollider& operator=(const CompositeCollider& other);
		CompositeCollider& operator=(CompositeCollider&& other) = default;

		/** @copydoc Collider::clone() */
		virtual std::unique_ptr<Collider> clone() const override
		{ return std::make_unique<CompositeCollider>(*this); };

		/** Updates the scale, translation and orientation of the
		 * CompositeCollider with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the
		 *			CompositeCollider */
		virtual void setTransforms(const glm::mat4& transforms) override;

		/** @return	the transformations matrix currently applied to the
		 *			CompositeCollider */
		virtual glm::mat4 getTransforms() const override
		{ return mTransformsMatrix; };

		/** @return the Axis Aligned Bounding Box that contains the
		 *			CompositeCollider */
		virtual AABB getAABB() const override { return mAABB; };

		/** @return	true if the CompositeCollider has been updated since the last
		 *			call to the resetUpdatedState function, false otherwise */
		virtual bool updated() const override;

		/** Resets the updated state of the CompositeCollider */
		virtual void resetUpdatedState() override;

		/** Adds the given Collider to the CompositeCollider
		 *
		 * @param	part a pointer to the Collider to add to the
		 *			CompositeCollider
		 * @return	a reference to the current CompositeCollider object */
		CompositeCollider& addPart(ColliderUPtr part);

		/** Calls the given callback for each of the parts of the
		 * CompositeCollider
		 *
		 * @param	callback the function to call */
		template <typename F>
		void processParts(F&& callback) const
		{ for (const auto& part : mParts) { callback(*part); } }

		/** Calls the given callback for each of the overlaping convex parts of
		 * the CompositeCollider with the given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @param	callback the function to call */
		virtual void processOverlapingParts(
			const AABB& aabb, const ConvexShapeCallback& callback
		) const override;

		/** Removes a Collider from the current CompositeCollider
		 *
		 * @param	part a pointer to the Collider to remove
		 * @return	a reference to the current CompositeCollider object */
		CompositeCollider& removePart(Collider* part);
	private:
		/** Calculates the AABB of the CompositeCollider with its Convex
		 * parts */
		void calculateAABB();
	};

}

#endif		// COMPOSITE_COLLIDER_H
