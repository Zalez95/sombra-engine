#include "ConstraintManager.h"
#include <numeric>
#include <algorithm>
#include "../RigidBody.h"
#include "Constraint.h"
#include "ConstraintBounds.h"

namespace physics {
	
	void ConstraintManager::addConstraint(Constraint* constraint)
	{
		if (!constraint) { return; }

		const ConstraintBounds* cb = constraint->getConstraintBounds();

		std::array<size_t, 2> constraintRB;
		for (size_t i = 0; i < 2; ++i) {
			RigidBody* rb = constraint->getRigidBody(i);

			bool shouldAdd = true;
			for (size_t j = 0; j < mRigidBodies.size(); ++j) {
				if (mRigidBodies[j] == rb) {
					constraintRB[i] = j;
					shouldAdd = false;
					break;
				}
			}

			if (shouldAdd) {
				mRigidBodies.push_back(rb);
				mInverseMassMatrix.emplace_back(rb->getInvertedMass());
				mInverseMassMatrix.push_back(rb->getInvertedInertiaTensor());

				constraintRB[i] = mRigidBodies.size() - 1;
			}
		}

		mConstraints.push_back(constraint);
		mConstraintRBMap.push_back(constraintRB);
		mLambdaMatrix.push_back(0.0f);
		mLambdaMinMatrix.push_back(cb->getAlphaMin());
		mLambdaMaxMatrix.push_back(cb->getAlphaMax());
	}

	
	void ConstraintManager::removeConstraint(Constraint* constraint)
	{
		auto itConstraint	= std::find(mConstraints.begin(), mConstraints.end(), constraint);
		size_t iConstraint	= std::distance(mConstraints.begin(), itConstraint);
		if (itConstraint == mConstraints.end()) { return; }

		// Delete the RigidBodies if the constraint to remove is the only one
		// that uses them
		for (size_t i = 0; i < 2; ++i) {
			size_t iRB = mConstraintRBMap[iConstraint][i];

			bool shouldRemove = true;
			size_t count = 0;
			for (auto it = mConstraintRBMap.begin(); it != mConstraintRBMap.end(); ++it) {
				if ((*it)[0] == iRB || (*it)[1] == iRB) {
					++count;
					if (count > 1) {
						shouldRemove = false;
						break;
					}
				}
			}

			if (shouldRemove) {
				// Remove the RigidBody and shift the map indexes left
				mRigidBodies.erase(mRigidBodies.begin() + iRB);
				mInverseMassMatrix.erase(
					mInverseMassMatrix.begin() + 2*iRB,
					mInverseMassMatrix.begin() + 2*(iRB+1)
				);

				for (std::array<size_t, 2>& pair : mConstraintRBMap) {
					if (pair[0] > iRB) { --pair[0]; }
					if (pair[1] > iRB) { --pair[1]; }
				}
			}
		}

		mConstraints.erase(itConstraint);
		mConstraintRBMap.erase(mConstraintRBMap.begin() + iConstraint);
		mLambdaMatrix.erase(mLambdaMatrix.begin() + iConstraint);
		mLambdaMinMatrix.erase(mLambdaMinMatrix.begin() + iConstraint);
		mLambdaMaxMatrix.erase(mLambdaMaxMatrix.begin() + iConstraint);
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
		mBiasMatrix.clear();
		mBiasMatrix.reserve(mConstraints.size());

		for (const Constraint* c : mConstraints) {
			mBiasMatrix.push_back(c->getBias());
		}
	}


	void ConstraintManager::updateVelocityMatrix()
	{
		mVelocityMatrix.clear();
		mVelocityMatrix.reserve(2 * mRigidBodies.size());

		for (const RigidBody* rb : mRigidBodies) {
			mVelocityMatrix.push_back(rb->mLinearVelocity);
			mVelocityMatrix.push_back(rb->mAngularVelocity);
		}
	}


	void ConstraintManager::updateForceExtMatrix()
	{
		mForceExtMatrix.clear();
		mForceExtMatrix.reserve(2 * mRigidBodies.size());

		for (const RigidBody* rb : mRigidBodies) {
			mForceExtMatrix.push_back(rb->getForceSum());
			mForceExtMatrix.push_back(rb->getTorqueSum());
		}
	}


	void ConstraintManager::updateJacobianMatrix()
	{
		mJacobianMatrix.clear();
		mJacobianMatrix.reserve(mConstraints.size());

		for (const Constraint* c : mConstraints) {
			mJacobianMatrix.push_back( c->getJacobianMatrix() );
		}
	}


	void ConstraintManager::solveConstraints(float deltaTime)
	{
		auto bMat = getBMatrix();
		auto hMat = getHMatrix(deltaTime);

		auto aMat = getAMatrix(bMat, mLambdaMatrix);
		auto dMat = getDMatrix(bMat, mJacobianMatrix);

		for (unsigned int i = 0; i < MAX_IT; ++i) {
			for (size_t j = 0; j < mConstraints.size(); ++j) {
				size_t iRB1 = mConstraintRBMap[j][0];
				size_t iRB2 = mConstraintRBMap[j][1];

				float ja1 = std::inner_product(mJacobianMatrix[j].begin(), mJacobianMatrix[j].begin() + 6, aMat.begin() + 6*iRB1, 0.0f);
				float ja2 = std::inner_product(mJacobianMatrix[j].begin() + 6, mJacobianMatrix[j].end(), aMat.begin() + 6*iRB2, 0.0f);
				float deltaLambda = (hMat[j] - ja1 - ja2) / dMat[j];

				float oldLambda = mLambdaMatrix[j];
				mLambdaMatrix[j] = std::max(mLambdaMinMatrix[j], std::min(mLambdaMatrix[j] + deltaLambda, mLambdaMaxMatrix[j]));
				deltaLambda = mLambdaMatrix[j] - oldLambda;
				
				for (size_t k = 0; k < 6; ++k) {
					aMat[6*iRB1 + k] += deltaLambda * bMat[j][k];
					aMat[6*iRB2 + k] += deltaLambda * bMat[j][6 + k];
				}
			}
		}
	}


	std::vector<std::array<float, 12>> ConstraintManager::getBMatrix() const
	{
		std::vector<std::array<float, 12>> bMatrix(mConstraints.size());

		for (size_t i = 0; i < mConstraints.size(); ++i) {
			for (size_t j = 0; j < 2; ++j) {
				size_t iRB = mConstraintRBMap[i][j];

				for (size_t k = 0; k < 2; ++k) {
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
			for (size_t j = 0; j < 2; ++j) {
				size_t iRB = mConstraintRBMap[i][j];

				for (size_t k = 0; k < 2; ++k) {
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
				mJacobianMatrix[i].begin(),
				mJacobianMatrix[i].end(),
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
			size_t iRB1 = mConstraintRBMap[i][0];
			size_t iRB2 = mConstraintRBMap[i][1];

			for (size_t k = 0; k < 6; ++k) {
				aMatrix[6*iRB1 + k] += lambdaMatrix[i] * bMatrix[i][k];
				aMatrix[6*iRB2 + k] += lambdaMatrix[i] * bMatrix[i][6 + k];
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
				jacobianMatrix[i].begin(),
				jacobianMatrix[i].end(),
				bMatrix[i].begin(), 0.0f
			));
		}

		return dMatrix;
	}

	
	void ConstraintManager::updateRigidBodies(float deltaTime)
	{
		std::vector<float> jLambdaMat(6 * mRigidBodies.size());
		for (size_t i = 0; i < mConstraints.size(); ++i) {
			size_t iRB1 = mConstraintRBMap[i][0];
			size_t iRB2 = mConstraintRBMap[i][1];

			for (size_t k = 0; k < 6; ++k) {
				jLambdaMat[6*iRB1 + k] += mLambdaMatrix[i] * mJacobianMatrix[i][k];
				jLambdaMat[6*iRB2 + k] += mLambdaMatrix[i] * mJacobianMatrix[i][6 + k];
			}
		}

		for (size_t i = 0; i < mRigidBodies.size(); ++i) {
			for (size_t j = 0; j < 2; ++j) {
				const glm::vec3& v1				= mVelocityMatrix[2*i + j];
				const glm::vec3& forceExt		= mForceExtMatrix[2*i + j];
				const glm::mat3& inverseMass	= mInverseMassMatrix[2*i + j];
				const glm::vec3& jLambda		= glm::vec3(
					jLambdaMat[6*i + 3*j],
					jLambdaMat[6*i + 3*j + 1],
					jLambdaMat[6*i + 3*j + 2]
				);

				glm::vec3 v2 = v1 + inverseMass * deltaTime * (jLambda + forceExt);
				if (j == 0) {
					mRigidBodies[i]->mLinearVelocity = v2;
					mRigidBodies[i]->mPosition += deltaTime * v2;
				}
				else {
					mRigidBodies[i]->mAngularVelocity = v2;
					mRigidBodies[i]->mOrientation = glm::normalize(glm::quat(deltaTime / 2 * v2) * mRigidBodies[i]->mOrientation);
				}
			}
		}
	}

}
