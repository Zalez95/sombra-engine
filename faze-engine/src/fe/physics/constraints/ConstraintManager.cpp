#include "fe/physics/constraints/ConstraintManager.h"
#include <numeric>
#include <algorithm>
#include "fe/physics/RigidBody.h"
#include "fe/physics/constraints/Constraint.h"
#include "fe/physics/constraints/ConstraintBounds.h"

namespace fe { namespace physics {

	void ConstraintManager::addConstraint(Constraint* constraint)
	{
		if (!constraint) { return; }

		// Get the indices of the RigidBodies of the constraint or add them if
		// they aren't yet
		std::array<int, 2> constraintRB;
		for (int i = 0; i < 2; ++i) {
			RigidBody* rb = constraint->getRigidBody(i);

			auto it = std::find(mRigidBodies.begin(), mRigidBodies.end(), rb);
			if (it != mRigidBodies.end()) {
				constraintRB[i] = std::distance(mRigidBodies.begin(), it);
			}
			else {
				mRigidBodies.push_back(rb);
				mInverseMassMatrix.emplace_back(rb->getInvertedMass());
				mInverseMassMatrix.push_back(rb->getInvertedInertiaTensor());

				constraintRB[i] = mRigidBodies.size() - 1;
			}
		}

		// Add the constraint and its data
		mConstraints.push_back(constraint);
		mConstraintRBMap.push_back(constraintRB);
		mLambdaMatrix.push_back(0.0f);
		const ConstraintBounds* cb = constraint->getConstraintBounds();
		mLambdaMinMatrix.push_back(cb->getAlphaMin());
		mLambdaMaxMatrix.push_back(cb->getAlphaMax());
	}


	void ConstraintManager::removeConstraint(Constraint* constraint)
	{
		auto itConstraint	= std::find(mConstraints.begin(), mConstraints.end(), constraint);
		int iConstraint		= std::distance(mConstraints.begin(), itConstraint);
		if (itConstraint == mConstraints.end()) { return; }

		// Remove the constraint and its cached data
		mConstraints.erase(itConstraint);
		mConstraintRBMap.erase(mConstraintRBMap.begin() + iConstraint);
		mLambdaMatrix.erase(mLambdaMatrix.begin() + iConstraint);
		mLambdaMinMatrix.erase(mLambdaMinMatrix.begin() + iConstraint);
		mLambdaMaxMatrix.erase(mLambdaMaxMatrix.begin() + iConstraint);

		// Delete the RigidBodies if the constraint to remove is the only one
		// that uses them
		for (int i = 0; i < 2; ++i) {
			int iRB = mConstraintRBMap[iConstraint][i];

			int count = 0;
			bool shouldRemove = std::any_of(
				mConstraintRBMap.begin(), mConstraintRBMap.end(),
				[&count, iRB](const std::array<int, 2>& item) {
					return (item[0] == iRB || item[1] == iRB) && (count++ > 0);
				}
			);

			if (shouldRemove) {
				// Remove the RigidBody and its cached data
				mRigidBodies.erase(mRigidBodies.begin() + iRB);
				mInverseMassMatrix.erase(
					mInverseMassMatrix.begin() + 2*iRB,
					mInverseMassMatrix.begin() + 2*(iRB+1)
				);

				// Shift the map indexes left
				for (std::array<int, 2>& pair : mConstraintRBMap) {
					if (pair[0] > iRB) { --pair[0]; }
					if (pair[1] > iRB) { --pair[1]; }
				}
			}
		}
	}


	void ConstraintManager::update(float deltaTime)
	{
		// 1. Update the matrices
		updateBiasMatrix();
		updateVelocityMatrix();
		updateForceExtMatrix();
		updateJacobianMatrix();

		// 2. Solve: jacobianMat * bMat * lambdaMat = hMat
		solveConstraints(deltaTime);

		// 3. Update the velocity and position of the RigidBodies
		updateRigidBodies(deltaTime);
	}

// Private functions
	void ConstraintManager::updateBiasMatrix()
	{
		mBiasMatrix = std::vector<float>();
		mBiasMatrix.reserve(mConstraints.size());

		for (const Constraint* c : mConstraints) {
			mBiasMatrix.push_back(c->getBias());
		}
	}


	void ConstraintManager::updateVelocityMatrix()
	{
		mVelocityMatrix = std::vector<glm::vec3>();
		mVelocityMatrix.reserve(2 * mRigidBodies.size());

		for (const RigidBody* rb : mRigidBodies) {
			mVelocityMatrix.push_back(rb->getLinearVelocity());
			mVelocityMatrix.push_back(rb->getAngularVelocity());
		}
	}


	void ConstraintManager::updateForceExtMatrix()
	{
		mForceExtMatrix = std::vector<glm::vec3>();
		mForceExtMatrix.reserve(2 * mRigidBodies.size());

		for (const RigidBody* rb : mRigidBodies) {
			mForceExtMatrix.push_back(rb->getForceSum());
			mForceExtMatrix.push_back(rb->getTorqueSum());
		}
	}


	void ConstraintManager::updateJacobianMatrix()
	{
		mJacobianMatrix = std::vector<std::array<float, 12>>();
		mJacobianMatrix.reserve(mConstraints.size());

		for (const Constraint* c : mConstraints) {
			mJacobianMatrix.push_back(c->getJacobianMatrix());
		}
	}


	void ConstraintManager::solveConstraints(float deltaTime)
	{
		auto bMat = getBMatrix();
		auto hMat = getHMatrix(deltaTime);

		auto aMat = getAMatrix(bMat, mLambdaMatrix);
		auto dMat = getDMatrix(bMat, mJacobianMatrix);

		for (int iteration = 0; iteration < sMaxIterations; ++iteration) {
			for (size_t i = 0; i < mConstraints.size(); ++i) {
				int iRB1 = mConstraintRBMap[i][0], iRB2 = mConstraintRBMap[i][1];

				float ja1 = std::inner_product(mJacobianMatrix[i].begin(), mJacobianMatrix[i].begin() + 6, aMat.begin() + 6*iRB1, 0.0f);
				float ja2 = std::inner_product(mJacobianMatrix[i].begin() + 6, mJacobianMatrix[i].end(), aMat.begin() + 6*iRB2, 0.0f);
				float deltaLambda = (hMat[i] - ja1 - ja2) / dMat[i];

				float oldLambda = mLambdaMatrix[i];
				mLambdaMatrix[i] = std::max(mLambdaMinMatrix[i], std::min(mLambdaMatrix[i] + deltaLambda, mLambdaMaxMatrix[i]));
				deltaLambda = mLambdaMatrix[i] - oldLambda;

				for (int j = 0; j < 6; ++j) {
					aMat[6*iRB1 + j] += deltaLambda * bMat[i][j];
					aMat[6*iRB2 + j] += deltaLambda * bMat[i][6 + j];
				}
			}
		}
	}


	std::vector<std::array<float, 12>> ConstraintManager::getBMatrix() const
	{
		std::vector<std::array<float, 12>> bMatrix(mConstraints.size());

		for (size_t i = 0; i < mConstraints.size(); ++i) {
			for (int j = 0; j < 2; ++j) {
				int iRB = mConstraintRBMap[i][j];

				for (int k = 0; k < 2; ++k) {
					const glm::mat3& inverseMass	= mInverseMassMatrix[2*iRB + k];
					const glm::vec3 jacobian		= glm::vec3(
						mJacobianMatrix[i][6*j + 3*k],
						mJacobianMatrix[i][6*j + 3*k + 1],
						mJacobianMatrix[i][6*j + 3*k + 2]
					);

					const glm::vec3 curB = inverseMass * jacobian;
					bMatrix[i][6*j + 3*k]		= curB.x;
					bMatrix[i][6*j + 3*k + 1]	= curB.y;
					bMatrix[i][6*j + 3*k + 2]	= curB.z;
				}
			}
		}

		return bMatrix;
	}


	std::vector<float> ConstraintManager::getHMatrix(float deltaTime) const
	{
		std::vector<float> hMatrix;
		hMatrix.reserve(mConstraints.size());

		for (size_t i = 0; i < mConstraints.size(); ++i) {
			float bias = mBiasMatrix[i];

			std::array<float, 12> tmp;
			for (int j = 0; j < 2; ++j) {
				int iRB = mConstraintRBMap[i][j];

				for (int k = 0; k < 2; ++k) {
					const glm::vec3& velocity		= mVelocityMatrix[2*iRB + k];
					const glm::vec3& forceExt		= mForceExtMatrix[2*iRB + k];
					const glm::mat3& inverseMass	= mInverseMassMatrix[2*iRB + k];

					const glm::vec3 toInsert = velocity / deltaTime + inverseMass * forceExt;
					tmp[6*j + 3*k]		= toInsert.x;
					tmp[6*j + 3*k + 1]	= toInsert.y;
					tmp[6*j + 3*k + 2]	= toInsert.z;
				}
			}

			float currentH = bias / deltaTime - std::inner_product(
				mJacobianMatrix[i].begin(), mJacobianMatrix[i].end(),
				tmp.begin(),
				0.0f
			);

			hMatrix.push_back(currentH);
		}

		return hMatrix;
	}


	std::vector<float> ConstraintManager::getAMatrix(
		const std::vector<std::array<float, 12>>& bMatrix,
		const std::vector<float>& lambdaMatrix
	) const
	{
		std::vector<float> aMatrix(6 * mRigidBodies.size());

		for (size_t i = 0; i < mConstraints.size(); ++i) {
			int iRB1 = mConstraintRBMap[i][0], iRB2 = mConstraintRBMap[i][1];

			for (int j = 0; j < 6; ++j) {
				aMatrix[6*iRB1 + j] += lambdaMatrix[i] * bMatrix[i][j];
				aMatrix[6*iRB2 + j] += lambdaMatrix[i] * bMatrix[i][6 + j];
			}
		}

		return aMatrix;
	}


	std::vector<float> ConstraintManager::getDMatrix(
		const std::vector<std::array<float, 12>>& bMatrix,
		const std::vector<std::array<float, 12>>& jacobianMatrix
	) const
	{
		std::vector<float> dMatrix;
		dMatrix.reserve(mConstraints.size());

		for (size_t i = 0; i < mConstraints.size(); ++i) {
			dMatrix.push_back(std::inner_product(
				jacobianMatrix[i].begin(), jacobianMatrix[i].end(),
				bMatrix[i].begin(),
				0.0f
			));
		}

		return dMatrix;
	}


	void ConstraintManager::updateRigidBodies(float deltaTime)
	{
		std::vector<float> jLambdaMat(6 * mRigidBodies.size());
		for (size_t i = 0; i < mConstraints.size(); ++i) {
			int iRB1 = mConstraintRBMap[i][0], iRB2 = mConstraintRBMap[i][1];

			for (int j = 0; j < 6; ++j) {
				jLambdaMat[6*iRB1 + j] += mLambdaMatrix[i] * mJacobianMatrix[i][j];
				jLambdaMat[6*iRB2 + j] += mLambdaMatrix[i] * mJacobianMatrix[i][6 + j];
			}
		}

		for (size_t i = 0; i < mRigidBodies.size(); ++i) {
			for (int j = 0; j < 2; ++j) {
				const glm::vec3& v1				= mVelocityMatrix[2*i + j];
				const glm::vec3& forceExt		= mForceExtMatrix[2*i + j];
				const glm::mat3& inverseMass	= mInverseMassMatrix[2*i + j];
				const glm::vec3 jLambda(jLambdaMat[6*i + 3*j], jLambdaMat[6*i + 3*j + 1], jLambdaMat[6*i + 3*j + 2]);

				glm::vec3 v2 = v1 + inverseMass * deltaTime * (jLambda + forceExt);
				if (j == 0) {
					mRigidBodies[i]->setLinearVelocity(v2);
					mRigidBodies[i]->setPosition(mRigidBodies[i]->getPosition() + v2 * deltaTime);
				}
				else {
					mRigidBodies[i]->setAngularVelocity(v2);
					mRigidBodies[i]->setOrientation(glm::normalize(glm::quat(deltaTime / 2.0f * v2) * mRigidBodies[i]->getOrientation()));
				}
			}

			mRigidBodies[i]->updateData();
		}
	}

}}
