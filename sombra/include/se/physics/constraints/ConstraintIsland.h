#ifndef CONSTRAINT_ISLAND_H
#define CONSTRAINT_ISLAND_H

#include <array>
#include <vector>
#include <glm/glm.hpp>

namespace se::physics {

	class RigidBody;
	class Constraint;
	class RigidBodyWorld;


	/**
	 * Class ConstraintIsland, it's the class used for solving the physics
	 * constraints between a set of rigid bodies of the RigidBodyWorld.
	 * It uses a PGS Solver based on the article
	 * @see Iterative Dynamics with Temporal Coherence by Erin Catto
	 */
	class ConstraintIsland
	{
	private:	// Nested types
		using vec12 = std::array<float, 12>;
		using IndexPair = std::array<std::size_t, 2>;

	private:	// Attributes
		/** The maximum number of iterations that the Gauss-Seidel algorithm
		 * should run for solving the Constraints */
		std::size_t mMaxConstraintIterations;

		/** The vector that holds the registered Constraints in the
		 * ConstraintIsland. Constraints are sorted ascendently for faster
		 * searchs. */
		std::vector<Constraint*> mConstraints;

		/** The vector that holds the RigidBodies whose movement is affected
		 * by the Constraints. RigidBodies are sorted ascendently for faster
		 * searchs. */
		std::vector<RigidBody*> mRigidBodies;

		/** The vector that stores the relations between the RigidBodies and
		 * its Constraints */
		std::vector<IndexPair> mConstraintRBMap;

		/** If the constraints have to be solved again or not, maybe because new
		 * constraints have been added/removed, the state of the RigidBodies
		 * have changed, or the Constraints wasn't solved in the last update. */
		bool mSolveConstraints;

		/** The variable that will be solved by the Constraint resolver.
		 * It's a column matrix with the size of (number of Constraints). */
		std::vector<float> mLambdaMatrix;

		/** A matrix with the lower bounds of the lambda variable. It's a
		 * column matrix with the size of (number of Constraints). */
		std::vector<float> mLambdaMinMatrix;

		/** A matrix with the upper bounds of the lambda variable. It's a
		 * column matrix with the size of (number of Constraints). */
		std::vector<float> mLambdaMaxMatrix;

		/** A matrix with all the Constraints' biases. It's a row matrix with
		 * the size of (number of Constraints). */
		std::vector<float> mBiasMatrix;

		/** A matrix with the jacobians of all the Constraints. It's a matrix
		 * with a size of (number of Constraints) by 6*(number of RigidBodies),
		 * but in our case it's represented by 12*(number of Constraints)
		 * floats so we can access to its data with the ConstraintRBMap. */
		std::vector<vec12> mJacobianMatrix;

		/** The diagonal of the inverse matrix with all the masses
		 * (E3 * mass and the inertia tensor in world space) of all the
		 * RigidBodies. It's a square matrix with a size of
		 * 2*(number of RigidBodies) mat3s. */
		std::vector<glm::mat3> mInverseMassMatrix;

		/** A matrix with the velocities (linear and angular) of all the
		 * RigidBodies. It's a column matrix with a size of
		 * 2*(number of RigidBodies) vec3s. */
		std::vector<glm::vec3> mVelocityMatrix;

		/** A matrix with the external forces (forces and torques) of all the
		 * RigidBodies. It's a column matrix with a size of
		 * 2*(number of RigidBodies) vec3s. */
		std::vector<glm::vec3> mForceExtMatrix;

	public:		// Constraints
		/** Creates a new ConstraintIsland
		 *
		 * @param	maxConstraintIterations the maximum number of iterations
		 *			that the Gauss-Seidel algorithm should run for solving the
		 *			Constraints */
		ConstraintIsland(std::size_t maxConstraintIterations) :
			mMaxConstraintIterations(maxConstraintIterations),
			mSolveConstraints(false) {};

		/** Registers the given Constraint in the ConstraintIsland, so the
		 * movement of the RigidBodies that it holds will be restricted.
		 *
		 * @param	constraint a pointer to the Constraint that we want to
		 *			register */
		void addConstraint(Constraint* constraint);

		/** @return	true if the ConstraintIsland has any constraints inside,
		 *			false otherwise */
		bool hasConstraints() const { return !mConstraints.empty(); };

		/** Iterates through all the ConstraintIsland Constraints calling the
		 * given callback function
		 *
		 * @param	callback the function to call for each Constraint */
		template <typename F>
		void processConstraints(F&& callback) const;

		/** Removes the given Constraint from the ConstraintIsland, so the
		 * movement of the RigidBodies that it holds won't longer be
		 * restricted.
		 *
		 * @param	constraint a pointer to the Constraint that we want to
		 *			remove
		 * @return	true if the Constraint was removed, false otherwise */
		bool removeConstraint(Constraint* constraint);

		/** Returns if the ConstraintIsland has any constraints with the
		 * given RigidBody or not
		 *
		 * @param	rigidBody a pointer to the RigidBody to check
		 * @return	true if the ConstraintIsland has the RigidBody,
		 *			false otherwise */
		bool hasRigidBody(RigidBody* rigidBody) const;

		/** Iterates through all the ConstraintIsland RigidBodies calling the
		 * given callback function
		 *
		 * @param	callback the function to call for each RigidBody */
		template <typename F>
		void processRigidBodies(F&& callback) const;

		/** Removes all the Constraints that constains the given RigidBody
		 * from the ConstraintIsland.
		 *
		 * @param	rigidBody a pointer to the RigidBody whose Constraints we
		 *			want to remove
		 * @return	true if the RigidBody was removed, false otherwise */
		bool removeRigidBody(RigidBody* rigidBody);

		/** Iterates through all the Constraints of the ConstraintIsland that
		 * containts the given RigidBody calling the given callback function
		 *
		 * @param	rigidBody a pointer to the RigidBody whose Constraints we
		 *			want to check
		 * @param	callback the function to call for each RigidBody */
		template <typename F>
		void processRigidBodyConstraints(
			RigidBody* rigidBody, F&& callback
		) const;

		/** Moves the constraints from the given ConstraintIsland to the
		 * current one
		 *
		 * @param	source the source ConstraintIsland, it will be empty of
		 *			Constraints after the merge operation */
		void merge(ConstraintIsland& source);

		/** Applies the constraints stored in the ConstraintIsland
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void update(float deltaTime);
	private:
		/** Updates the @see mSolveConstraints flag with the changes made to the
		 * RigidBodies or Constraints */
		void updateSolveConstraints();

		/** Updates the lambda min and lambda max matrices value */
		void updateLambdaBoundsMatrices();

		/** Updates the bias matrix value */
		void updateBiasMatrix();

		/** Updates the jacobian matrix value */
		void updateJacobianMatrix();

		/** Updates the inverse mass matrix value */
		void updateInverseMassMatrix();

		/** Updates the velocity matrix value */
		void updateVelocityMatrix();

		/** Updates the external forces matrix value */
		void updateForceExtMatrix();

		/** Runs the Gauss-Seidel algorithm for solving the mLambdaMatrix in:
		 * mJacobianMatrix * mInverseMassMatrix * transpose(mJacobianMatrix)
		 *	* mLambdaMatrix = etaMatrix
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void calculateGaussSeidel(float deltaTime);

		/** Calculates the transposed invMassJacobianMatrix which is
		 * equal to mInverseMassMatrix * transpose(mJacobianMatrix)
		 *
		 * @return	the invMassJacobianMatrix matrix. Its size is the same than
		 *			(number of RigidBodies) * (number of Constraints), but
		 *			because it has the same sparsity than mJacobianMatrix
		 *			it is stored in the same way */
		std::vector<vec12> calculateInvMassJacobianMatrix() const;

		/** Calculates the Eta Matrix, which is equal to
		 * mBiasMatrix / deltaTime - mJacobianMatrix *
		 * (mVelocityMatrix / deltaTime + mInverseMassMatrix * mForceExtMatrix)
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds
		 * @return	the Eta matrix. Its size is (number of Constraints) */
		std::vector<float> calculateEtaMatrix(float deltaTime) const;

		/** Calculates the invMJLambdaMatrix which is equal to
		 * B * lambda
		 *
		 * @param	invMassJacobianMatrix the matrix equal to
		 *			mInverseMassMatrix * transpose(mJacobianMatrix)
		 * @param	lambdaMatrix the lambda matrix
		 * @return	the A matrix. Its size is 6*(number of RigidBodies) */
		std::vector<float> calculateInvMJLambdaMatrix(
			const std::vector<vec12>& invMassJacobianMatrix,
			const std::vector<float>& lambdaMatrix
		) const;

		/** Calculates the diagonal of the JMJMatrix, which is equal to,
		 * mJacobianMatrix * invMassJacobianMatrix
		 *
		 * @param	jacobianMatrix the jacobian matrix
		 * @param	invMassJacobianMatrix the matrix equal to
		 *			mInverseMassMatrix * transpose(mJacobianMatrix)
		 * @return	the diagonal of the JMJMatrix. Its size is
		 *			(number of Constraints) */
		std::vector<float> calculateDiagonalJInvMJMatrix(
			const std::vector<vec12>& jacobianMatrix,
			const std::vector<vec12>& invMassJacobianMatrix
		) const;

		/** Updates the velocity and position of the RigidBodies with the
		 * calculated lambda matrix
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void updateRigidBodies(float deltaTime);

		/** Deletes the RigidBody located at the given index if it has zero or
		 * one Constraint users
		 *
		 * @param	iRB the index of the RigidBody in mRigidBodies
		 * @return	true if the RigidBody was removed, false otherwise */
		bool tryRemoveRigidBody(std::size_t iRB);
	};


	template <typename F>
	void ConstraintIsland::processConstraints(F&& callback) const
	{
		for (Constraint* constraint : mConstraints) {
			callback(constraint);
		}
	}


	template <typename F>
	void ConstraintIsland::processRigidBodies(F&& callback) const
	{
		for (RigidBody* rigidBody : mRigidBodies) {
			callback(rigidBody);
		}
	}


	template <typename F>
	void ConstraintIsland::processRigidBodyConstraints(
		RigidBody* rigidBody, F&& callback
	) const
	{
		auto itRB = std::lower_bound(
			mRigidBodies.begin(), mRigidBodies.end(), rigidBody
		);
		if ((itRB != mRigidBodies.end()) && (*itRB == rigidBody)) {
			std::size_t iRigidBody = std::distance(mRigidBodies.begin(), itRB);

			for (std::size_t i = 0; i < mConstraints.size(); ++i) {
				if ((mConstraintRBMap[i][0] == iRigidBody)
					|| (mConstraintRBMap[i][1] == iRigidBody)
				) {
					callback(mConstraints[i]);
				}
			}
		}
	}

}

#endif		// CONSTRAINT_ISLAND_H
