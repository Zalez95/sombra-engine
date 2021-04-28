#include <numeric>
#include <algorithm>
#include "se/physics/RigidBody.h"
#include "se/physics/RigidBodyDynamics.h"
#include "se/physics/constraints/Constraint.h"
#include "se/physics/constraints/ConstraintManager.h"

namespace se::physics {

	void ConstraintManager::addConstraint(Constraint* constraint)
	{
		if (!constraint) { return; }

		IndexPair rbIndices;
		for (std::size_t i = 0; i < 2; ++i) {
			RigidBody* rb = constraint->getRigidBody(i);

			// Get the index of the RigidBody in the mRigidBodies vector or add
			// it if it isn't yet
			auto it = std::find(mRigidBodies.begin(), mRigidBodies.end(), rb);
			if (it != mRigidBodies.end()) {
				std::size_t iRB = std::distance(mRigidBodies.begin(), it);
				mShouldSolveMatrix[iRB] = true;
				rbIndices[i] = iRB;
			}
			else {
				mRigidBodies.emplace_back(rb);
				mShouldSolveMatrix.emplace_back(true);
				mInverseMassMatrix.emplace_back(rb->getConfig().invertedMass);
				mInverseMassMatrix.emplace_back(rb->getInvertedInertiaTensorWorld());
				mVelocityMatrix.emplace_back(rb->getData().linearVelocity);
				mVelocityMatrix.emplace_back(rb->getData().angularVelocity);
				mForceExtMatrix.emplace_back(rb->getData().forceSum);
				mForceExtMatrix.emplace_back(rb->getData().torqueSum);
				rbIndices[i] = mRigidBodies.size() - 1;
			}
		}

		// Add the constraint and its data
		auto cb = constraint->getConstraintBounds();
		mConstraints.emplace_back(constraint);
		mConstraintRBMap.emplace_back(rbIndices);
		mLambdaMatrix.emplace_back(0.0f);
		mLambdaMinMatrix.emplace_back(cb.lambdaMin);
		mLambdaMaxMatrix.emplace_back(cb.lambdaMax);
		mBiasMatrix.emplace_back(constraint->getBias());
		mJacobianMatrix.emplace_back(constraint->getJacobianMatrix());
	}


	void ConstraintManager::removeConstraint(Constraint* constraint)
	{
		auto itConstraint = std::find(mConstraints.begin(), mConstraints.end(), constraint);
		if (itConstraint == mConstraints.end()) { return; }

		std::size_t iConstraint = std::distance(mConstraints.begin(), itConstraint);

		// Remove the RigidBodies if the constraint to remove is the only one
		// that uses them
		for (std::size_t iRB : mConstraintRBMap[iConstraint]) {
			if (!tryRemoveRigidBody(iRB)) {
				// Change the mShouldSolveMatrix to solve the rest of
				// the Constraints in the next update
				mShouldSolveMatrix[iRB] = true;
			}
		}

		// Remove the constraint and its cached data
		mConstraints.erase(itConstraint);
		mConstraintRBMap.erase(mConstraintRBMap.begin() + iConstraint);
		mLambdaMatrix.erase(mLambdaMatrix.begin() + iConstraint);
		mLambdaMinMatrix.erase(mLambdaMinMatrix.begin() + iConstraint);
		mLambdaMaxMatrix.erase(mLambdaMaxMatrix.begin() + iConstraint);
		mBiasMatrix.erase(mBiasMatrix.begin() + iConstraint);
		mJacobianMatrix.erase(mJacobianMatrix.begin() + iConstraint);
	}


	void ConstraintManager::removeRigidBody(RigidBody* rigidBody)
	{
		auto itRigidBody = std::find(mRigidBodies.begin(), mRigidBodies.end(), rigidBody);
		if (itRigidBody == mRigidBodies.end()) { return; }

		std::size_t iRB = std::distance(mRigidBodies.begin(), itRigidBody);
		for (std::size_t iConstraint = 0; iConstraint < mConstraints.size();) {
			// Check if any of the Constraint RigidBodies is the RigidBody to
			// remove
			bool shouldRemove = false;
			std::size_t iiRB = 0, iRB2 = 0;
			if (iRB == mConstraintRBMap[iConstraint][0]) {
				shouldRemove = true;
				iiRB = 0;
				iRB2 = mConstraintRBMap[iConstraint][1];
			}
			else if (iRB == mConstraintRBMap[iConstraint][1]) {
				shouldRemove = true;
				iiRB = 1;
				iRB2 = mConstraintRBMap[iConstraint][0];
			}

			if (shouldRemove) {
				// Remove the other RigidBody if the constraint to remove is
				// the only one that uses it
				if (!tryRemoveRigidBody(iRB2)) {
					// Change the mShouldSolveMatrix to solve the rest of
					// the Constraints in the next update
					mShouldSolveMatrix[iRB2] = true;
				}
				else {
					// Update the iRB index with its new value due to the
					// removal stage
					iRB = mConstraintRBMap[iConstraint][iiRB];
				}

				// Remove the constraint and its cached data
				mConstraints.erase(mConstraints.begin() + iConstraint);
				mConstraintRBMap.erase(mConstraintRBMap.begin() + iConstraint);
				mLambdaMatrix.erase(mLambdaMatrix.begin() + iConstraint);
				mLambdaMinMatrix.erase(mLambdaMinMatrix.begin() + iConstraint);
				mLambdaMaxMatrix.erase(mLambdaMaxMatrix.begin() + iConstraint);
				mBiasMatrix.erase(mBiasMatrix.begin() + iConstraint);
				mJacobianMatrix.erase(mJacobianMatrix.begin() + iConstraint);
			}
			else {
				++iConstraint;
			}
		}

		// Remove the current RigidBody
		tryRemoveRigidBody(iRB);
	}


	void ConstraintManager::update(float deltaTime)
	{
		// 1. Update the matrices
		updateLambdaBoundsMatrices();
		updateBiasMatrix();
		updateJacobianMatrix();
		updateShouldSolveMatrix();
		updateInverseMassMatrix();
		updateVelocityMatrix();
		updateForceExtMatrix();

		// 2. Solve the lambda values in:
		// mJacobianMatrix * mInverseMassMatrix * transpose(mJacobianMatrix)
		//	* mLambdaMatrix = etaMatrix
		// With the Gauss-Seidel algorithm
		calculateGaussSeidel(deltaTime);

		// 3. Update the velocity and position of the RigidBodies
		updateRigidBodies(deltaTime);
	}

// Private functions
	void ConstraintManager::updateLambdaBoundsMatrices()
	{
		for (std::size_t iConstraint = 0; iConstraint < mConstraints.size(); ++iConstraint) {
			auto cb = mConstraints[iConstraint]->getConstraintBounds();
			mLambdaMinMatrix[iConstraint] = cb.lambdaMin;
			mLambdaMaxMatrix[iConstraint] = cb.lambdaMax;
		}
	}


	void ConstraintManager::updateBiasMatrix()
	{
		for (std::size_t iConstraint = 0; iConstraint < mConstraints.size(); ++iConstraint) {
			mBiasMatrix[iConstraint] = mConstraints[iConstraint]->getBias();
		}
	}


	void ConstraintManager::updateJacobianMatrix()
	{
		for (std::size_t iConstraint = 0; iConstraint < mConstraints.size(); ++iConstraint) {
			mJacobianMatrix[iConstraint] = mConstraints[iConstraint]->getJacobianMatrix();
		}
	}


	void ConstraintManager::updateShouldSolveMatrix()
	{
		for (std::size_t iRB = 0; iRB < mRigidBodies.size(); ++iRB) {
			mShouldSolveMatrix[iRB] = mShouldSolveMatrix[iRB]
				|| mRigidBodies[iRB]->checkState(RigidBodyState::Integrated)
				|| !mRigidBodies[iRB]->checkState(RigidBodyState::Sleeping);
		}
	}


	void ConstraintManager::updateInverseMassMatrix()
	{
		for (std::size_t iRB = 0; iRB < mRigidBodies.size(); ++iRB) {
			mInverseMassMatrix[2*iRB] = glm::mat3(mRigidBodies[iRB]->getConfig().invertedMass);
			mInverseMassMatrix[2*iRB + 1] = mRigidBodies[iRB]->getInvertedInertiaTensorWorld();
		}
	}


	void ConstraintManager::updateVelocityMatrix()
	{
		for (std::size_t iRB = 0; iRB < mRigidBodies.size(); ++iRB) {
			mVelocityMatrix[2*iRB] = mRigidBodies[iRB]->getData().linearVelocity;
			mVelocityMatrix[2*iRB + 1] = mRigidBodies[iRB]->getData().angularVelocity;
		}
	}


	void ConstraintManager::updateForceExtMatrix()
	{
		for (std::size_t iRB = 0; iRB < mRigidBodies.size(); ++iRB) {
			mForceExtMatrix[2*iRB] = mRigidBodies[iRB]->getData().forceSum;
			mForceExtMatrix[2*iRB + 1] = mRigidBodies[iRB]->getData().torqueSum;
		}
	}


	void ConstraintManager::calculateGaussSeidel(float deltaTime)
	{
		const std::vector<float> etaMatrix = calculateEtaMatrix(deltaTime);
		const std::vector<vec12> invMassJacobianMatrix = calculateInvMassJacobianMatrix();

		// The factorization is for skipping the storage of the matrix
		// mJacobianMatrix * mInverseMassMatrix * transpose(mJacobianMatrix)
		// which is too big, by exploiting the sparsity of invMassJacobianMatrix
		// and mJacobian matrices
		const std::vector<float> diagonalJInvMJMatrix = calculateDiagonalJInvMJMatrix(mJacobianMatrix, invMassJacobianMatrix);
		std::vector<float> invMJLambdaMatrix = calculateInvMJLambdaMatrix(invMassJacobianMatrix, mLambdaMatrix);

		// We use a fixed number of iterations for the Gauss-Seidel algorithm
		for (int iteration = 0; iteration < kMaxIterations; ++iteration) {
			for (std::size_t i = 0; i < mConstraints.size(); ++i) {
				std::size_t iRB1 = mConstraintRBMap[i][0], iRB2 = mConstraintRBMap[i][1];

				// Check if the constraint needs to be solved
				if (!mShouldSolveMatrix[iRB1] && !mShouldSolveMatrix[iRB2]) { continue; }
				mShouldSolveMatrix[iRB1] = true;
				mShouldSolveMatrix[iRB2] = true;

				// Calculate the current change to lambda
				float curJInvMJLambda = std::inner_product(
					mJacobianMatrix[i].begin(), mJacobianMatrix[i].begin() + 6,
					invMJLambdaMatrix.begin() + 6*iRB1,
					0.0f
				);
				curJInvMJLambda += std::inner_product(
					mJacobianMatrix[i].begin() + 6, mJacobianMatrix[i].end(),
					invMJLambdaMatrix.begin() + 6*iRB2,
					0.0f
				);

				float deltaLambda = (etaMatrix[i] - curJInvMJLambda) / diagonalJInvMJMatrix[i];

				// Clamp the change to lambda to the Constraint lambda bounds
				float oldLambda = mLambdaMatrix[i];
				mLambdaMatrix[i] = std::clamp(oldLambda + deltaLambda, mLambdaMinMatrix[i], mLambdaMaxMatrix[i]);

				// Update the invMJLambdaMatrix with the current change
				deltaLambda = mLambdaMatrix[i] - oldLambda;
				for (std::size_t j = 0; j < 6; ++j) {
					invMJLambdaMatrix[6*iRB1 + j] += deltaLambda * invMassJacobianMatrix[i][j];
					invMJLambdaMatrix[6*iRB2 + j] += deltaLambda * invMassJacobianMatrix[i][6 + j];
				}
			}
		}
	}


	std::vector<ConstraintManager::vec12> ConstraintManager::calculateInvMassJacobianMatrix() const
	{
		std::vector<vec12> invMassJacobianMatrix(mConstraints.size());

		for (std::size_t i = 0; i < mConstraints.size(); ++i) {
			for (std::size_t j = 0; j < 2; ++j) {
				std::size_t iRB = mConstraintRBMap[i][j];

				for (std::size_t k = 0; k < 2; ++k) {
					const glm::mat3& inverseMass = mInverseMassMatrix[2*iRB + k];
					const glm::vec3 jacobian = glm::vec3(
						mJacobianMatrix[i][6*j + 3*k],
						mJacobianMatrix[i][6*j + 3*k + 1],
						mJacobianMatrix[i][6*j + 3*k + 2]
					);

					const glm::vec3 curInvMassJacobian = inverseMass * jacobian;
					invMassJacobianMatrix[i][6*j + 3*k]		= curInvMassJacobian.x;
					invMassJacobianMatrix[i][6*j + 3*k + 1]	= curInvMassJacobian.y;
					invMassJacobianMatrix[i][6*j + 3*k + 2]	= curInvMassJacobian.z;
				}
			}
		}

		return invMassJacobianMatrix;
	}


	std::vector<float> ConstraintManager::calculateEtaMatrix(float deltaTime) const
	{
		std::vector<float> etaMatrix;
		etaMatrix.reserve(mConstraints.size());

		for (std::size_t i = 0; i < mConstraints.size(); ++i) {
			float bias = mBiasMatrix[i];

			vec12 extAccelerations;
			for (std::size_t j = 0; j < 2; ++j) {
				std::size_t iRB = mConstraintRBMap[i][j];

				for (std::size_t k = 0; k < 2; ++k) {
					const glm::vec3& velocity		= mVelocityMatrix[2*iRB + k];
					const glm::mat3& inverseMass	= mInverseMassMatrix[2*iRB + k];
					const glm::vec3& forceExt		= mForceExtMatrix[2*iRB + k];

					const glm::vec3 extAcceleration = velocity / deltaTime + inverseMass * forceExt;
					extAccelerations[6*j + 3*k]		= extAcceleration.x;
					extAccelerations[6*j + 3*k + 1]	= extAcceleration.y;
					extAccelerations[6*j + 3*k + 2]	= extAcceleration.z;
				}
			}

			float currentEta = bias / deltaTime - std::inner_product(
				mJacobianMatrix[i].begin(), mJacobianMatrix[i].end(),
				extAccelerations.begin(),
				0.0f
			);

			etaMatrix.push_back(currentEta);
		}

		return etaMatrix;
	}


	std::vector<float> ConstraintManager::calculateInvMJLambdaMatrix(
		const std::vector<ConstraintManager::vec12>& invMassJacobianMatrix,
		const std::vector<float>& lambdaMatrix
	) const
	{
		std::vector<float> invMJLambdaMatrix(6 * mRigidBodies.size());

		for (std::size_t i = 0; i < mConstraints.size(); ++i) {
			std::size_t iRB1 = mConstraintRBMap[i][0], iRB2 = mConstraintRBMap[i][1];

			for (std::size_t j = 0; j < 6; ++j) {
				invMJLambdaMatrix[6*iRB1 + j] += invMassJacobianMatrix[i][j] * lambdaMatrix[i];
				invMJLambdaMatrix[6*iRB2 + j] += invMassJacobianMatrix[i][6 + j] * lambdaMatrix[i];
			}
		}

		return invMJLambdaMatrix;
	}


	std::vector<float> ConstraintManager::calculateDiagonalJInvMJMatrix(
		const std::vector<ConstraintManager::vec12>& jacobianMatrix,
		const std::vector<ConstraintManager::vec12>& invMassJacobianMatrix
	) const
	{
		std::vector<float> diagonalJInvMJ;
		diagonalJInvMJ.reserve(mConstraints.size());

		for (std::size_t i = 0; i < mConstraints.size(); ++i) {
			diagonalJInvMJ.push_back(std::inner_product(
				jacobianMatrix[i].begin(), jacobianMatrix[i].end(),
				invMassJacobianMatrix[i].begin(),
				0.0f
			));
		}

		return diagonalJInvMJ;
	}


	void ConstraintManager::updateRigidBodies(float deltaTime)
	{
		std::vector<float> jLambdaMatrix(6 * mRigidBodies.size());
		for (std::size_t i = 0; i < mConstraints.size(); ++i) {
			std::size_t iRB1 = mConstraintRBMap[i][0], iRB2 = mConstraintRBMap[i][1];

			for (std::size_t j = 0; j < 6; ++j) {
				jLambdaMatrix[6*iRB1 + j] += mLambdaMatrix[i] * mJacobianMatrix[i][j];
				jLambdaMatrix[6*iRB2 + j] += mLambdaMatrix[i] * mJacobianMatrix[i][6 + j];
			}
		}

		for (std::size_t i = 0; i < mRigidBodies.size(); ++i) {
			// Check if the RigidBody constraints has been solved or if it has
			// infinite mass
			if (!mShouldSolveMatrix[i] || (mInverseMassMatrix[2*i][0][0] == 0.0f)) { continue; }

			// Update the RigidBody motion data
			for (std::size_t j = 0; j < 2; ++j) {
				const glm::vec3& v1				= mVelocityMatrix[2*i + j];
				const glm::vec3& forceExt		= mForceExtMatrix[2*i + j];
				const glm::mat3& inverseMass	= mInverseMassMatrix[2*i + j];
				const glm::vec3 jLambda(jLambdaMatrix[6*i + 3*j], jLambdaMatrix[6*i + 3*j + 1], jLambdaMatrix[6*i + 3*j + 2]);

				glm::vec3 v2 = v1 + inverseMass * deltaTime * (jLambda + forceExt);
				if (j == 0) {
					mRigidBodies[i]->getData().linearVelocity = v2;
					RigidBodyDynamics::integrateLinearVelocity(*mRigidBodies[i], deltaTime);
				}
				else {
					mRigidBodies[i]->getData().angularVelocity = v2;
					RigidBodyDynamics::integrateAngularVelocity(*mRigidBodies[i], deltaTime);
				}
			}

			// Update the RigidBody internal state
			RigidBodyDynamics::updateTransformsMatrix(*mRigidBodies[i]);
			RigidBodyDynamics::setState(*mRigidBodies[i], RigidBodyState::ConstraintsSolved, true);
			RigidBodyDynamics::setState(*mRigidBodies[i], RigidBodyState::Sleeping, false);
		}
	}


	bool ConstraintManager::tryRemoveRigidBody(std::size_t iRB)
	{
		bool remove = true, first = true;
		for (IndexPair& pair : mConstraintRBMap) {
			if ((pair[0] == iRB) || (pair[1] == iRB)) {
				if (!first) {
					remove = false;
					break;
				}
				first = false;
			}
		}

		if (remove) {
			// Remove the RigidBody
			mRigidBodies.erase(mRigidBodies.begin() + iRB);
			mShouldSolveMatrix.erase(mShouldSolveMatrix.begin() + iRB);
			mInverseMassMatrix.erase(mInverseMassMatrix.begin() + 2*iRB, mInverseMassMatrix.begin() + 2*iRB + 1);
			mVelocityMatrix.erase(mVelocityMatrix.begin() + 2*iRB, mVelocityMatrix.begin() + 2*iRB + 1);
			mForceExtMatrix.erase(mForceExtMatrix.begin() + 2*iRB, mForceExtMatrix.begin() + 2*iRB + 1);

			// Shift the mConstraintRBMap RigidBody indices left
			for (IndexPair& pair : mConstraintRBMap) {
				if (pair[0] > iRB) { --pair[0]; }
				if (pair[1] > iRB) { --pair[1]; }
			}

			return true;
		}

		return false;
	}

}
