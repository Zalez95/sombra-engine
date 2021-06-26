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

		/** @copydoc Collider::setTransforms() */
		virtual void setTransforms(const glm::mat4& transforms) override;

		/** @copydoc Collider::getTransforms() */
		virtual glm::mat4 getTransforms() const override
		{ return mTransformsMatrix; };

		/** @copydoc Collider::getAABB() */
		virtual AABB getAABB() const override { return mAABB; };

		/** @copydoc Collider::updated() */
		virtual bool updated() const override;

		/** @copydoc Collider::resetUpdatedState() */
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

		/** @copydoc ConcaveCollider::processOverlapingParts() */
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
