#ifndef CONSTRAINT_BOUNDS_H
#define CONSTRAINT_BOUNDS_H

namespace fe { namespace physics {

	/**
	 * Class ConstraintBounds. It defines the bounds of the lambda variable of
	 * a Constraint.
	 */
	class ConstraintBounds
	{
	private:	// Attributes
		/** The minimum value that the alpha variable of the constraint could
		 * have */
		float mAlphaMin;

		/** The maximum value that the alpha variable of the constraint could
		 * have */
		float mAlphaMax;

	public:		// Functions
		/** Creates a new ConstraintBounds */
		ConstraintBounds(float alphaMin, float alphaMax) :
			mAlphaMin(alphaMin), mAlphaMax(alphaMax) {};

		/** Class destructor */
		~ConstraintBounds() {};

		/** @return the minimum value that the alpha variable of the
		 *			constraint could have */
		inline float getAlphaMin() const { return mAlphaMin; };

		/** @return the maximum value that the alpha variable of the
		 *			constraint could have */
		inline float getAlphaMax() const { return mAlphaMax; };
	};

}}

#endif		// CONSTRAINT_BOUNDS_H
