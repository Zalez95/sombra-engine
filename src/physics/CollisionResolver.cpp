#include "CollisionResolver.h"
#include <algorithm>
#include "RigidBody.h"

namespace physics {

    void CollisionResolver::addContact(Contact& contact, RigidBody* rb1, RigidBody* rb2)
	{
		if (rb1 || rb2) {
			mContacts.emplace_back(contact, rb1, rb2);
		}
	}


    void CollisionResolver::resolve(float delta)
	{
		for (ContactData contactData : mContacts) {
			prepareContact(contactData);
		}

        while (!mContacts.empty()) {
			// Update the contact with the biggest penetration
			auto maxIt = mContacts.begin();
			for (auto it = maxIt; it != mContacts.end(); ++it) {
				if (it->mContact.mPenetration > maxIt->mContact.mPenetration) {
					maxIt = it;
				}
			}

			ContactData& maxContact = (*maxIt);
			prepareContact(maxContact);
			calculatePositionChanges(maxContact);
			calculateVelocityChanges(maxContact, delta);
			updateOtherContacts(maxContact);

			mContacts.erase(maxIt);
        }
	}

// Private functions
	void CollisionResolver::prepareContact(ContactData& contactData)
    {
        // If there is only one body, it must be in the first position
		if (!contactData.mContactBodies[0]) {
			std::swap(contactData.mContactBodies[0], contactData.mContactBodies[1]);
		}

		// Calculate the contact space to world space matrix
		contactData.mContactToWorldMatrix = getContactToWorldMatrix(contactData.mContact);

        // Calculate the positions of the RigidBodies relative to the Contact
		// point
		glm::vec3 contactPosition = contactData.mContact.mPosition;
		for (size_t i = 0; i < 2; ++i) {
			if (contactData.mContactBodies[i]) {
	 			contactData.mRelativePositions[i] = contactPosition - contactData.mContactBodies[i]->mPosition;
			}
		}

        // Calculate the closing velocity of RigidBodies at the Contact point
		for (size_t i = 0; i < 2; ++i) {
			if (contactData.mContactBodies[i]) {
				contactData.mRelativeVelocities[i] = glm::cross(contactData.mContactBodies[i]->mAngularVelocity, contactData.mRelativePositions[i]);
				contactData.mRelativeVelocities[i] += contactData.mContactBodies[i]->mLinearVelocity;
			}
		}
    }


	glm::mat3 CollisionResolver::getContactToWorldMatrix(Contact& contact) const
	{
		// Two of the tangent vectors of the Contact that
		// with the normal vector create an orthonormal basis
		glm::vec3 tangents[2];

		// We create the orthonormal with the normal and one generated vector
		// (we split the code so we don't end with problems with the normal being
		// parallel to the generated vector)
		if (abs(contact.mNormal.x) > abs(contact.mNormal.y)) {
			// The normal is nearer to the X axis so we use the Y axis as the generated vector
			tangents[0] = glm::cross(contact.mNormal, glm::vec3(0, 1, 0));
			tangents[1] = glm::cross(tangents[0], contact.mNormal);
		}
		else {
			// The normal is nearer to the Y axis so we use the X axis as the generated vector
			tangents[0] = glm::cross(contact.mNormal, glm::vec3(1, 0, 0));
			tangents[1] = glm::cross(tangents[0], contact.mNormal);
		}

		tangents[0] = glm::normalize(tangents[0]);
		tangents[1] = glm::normalize(tangents[1]);

		// Create the matrix from the basis vectors and return it's inverse
		// Note that the inverse of a rotation matrix is the same than it's transpose
		return glm::transpose( glm::mat3(contact.mNormal, tangents[0], tangents[1]) );
	}

	
	void CollisionResolver::calculatePositionChanges(ContactData& contactData)
    {
		glm::vec3 contactNormal	= contactData.mContact.mNormal;
		float penetration		= contactData.mContact.mPenetration;

        // Calculate linear, angular and total inertia of both RigidBodies in
		// the direction of the Contact normal
		float totalInertia = 0.0f, linearInertia[2], angularInertia[2];
		for (size_t i = 0; i < 2; ++i) {
			if (contactData.mContactBodies[i]) {
				linearInertia[i] = contactData.mContactBodies[i]->getInvertedMass();
				
				glm::mat3 invertedInertiaTensor	= contactData.mContactBodies[i]->getInvertedInertiaTensor();
				glm::vec3 angularInertiaWorld	= glm::cross(contactData.mRelativePositions[i], contactNormal);
				angularInertiaWorld				= invertedInertiaTensor * angularInertiaWorld;
				angularInertiaWorld				= glm::cross(angularInertiaWorld, contactData.mRelativePositions[i]);
				angularInertia[i]				= glm::dot(angularInertiaWorld, contactNormal);

				totalInertia += linearInertia[i] + angularInertia[i];
			}
		}
		
		// Calculate the change in position and orientation of the RigidBodies
		for (size_t i = 0; i < 2; ++i) {
			if (contactData.mContactBodies[i]) {
				float sign						= (i == 0)? 1.0f : -1.0f;
				float displacementNeeded		= sign * penetration * linearInertia[i] / totalInertia;
				float rotationNeeded			= sign * penetration * angularInertia[i] / totalInertia;

				// Check that the rotationNeeded doesn't exceed the limit
				float limit						= ANGULAR_LIMIT * glm::length(contactData.mRelativePositions[i]);
				if (abs(rotationNeeded) > limit) {
					float total					= displacementNeeded + rotationNeeded;
					rotationNeeded				= (rotationNeeded >= 0)? limit : -limit;
					displacementNeeded			= total - rotationNeeded;
				}

				contactData.mPositionChange[i]	= contactNormal * displacementNeeded;
		
				if (rotationNeeded != 0) {			// Check if we need to rotate the RigidBodies
					glm::mat3 invertedInertiaTensor		= contactData.mContactBodies[i]->getInvertedInertiaTensor();
					glm::vec3 impulseTorque				= glm::cross(contactData.mRelativePositions[i], contactNormal);
					glm::vec3 impulsePerMove			= invertedInertiaTensor * impulseTorque;
					contactData.mOrientationChange[i]	= glm::quat(rotationNeeded * impulsePerMove / angularInertia[i]);
				}
			}
		}

		// Apply the changes
		for (size_t i = 0; i < 2; ++i) {
			if (contactData.mContactBodies[i]) {
				contactData.mContactBodies[i]->mPosition += contactData.mPositionChange[i];
				contactData.mContactBodies[i]->mOrientation = contactData.mOrientationChange[i] * contactData.mContactBodies[i]->mOrientation;
				contactData.mContactBodies[i]->updateTransformsMatrix();
				contactData.mContactBodies[i]->updateInertiaTensorWorld();
			}
		}
    }

	
	void CollisionResolver::calculateVelocityChanges(ContactData& contactData, float delta)
    {
		// Calculate the closing velocity at the contact point
		glm::vec3 closingVelocity	= contactData.mContactToWorldMatrix * contactData.mRelativeVelocities[0];
		closingVelocity				+= contactData.mContactToWorldMatrix * contactData.mRelativeVelocities[1];
			
		// Calculate the velocity change needed depending on the material
		// properties
		float deltaVelocity			= -(1 + RESTITUTION) * closingVelocity.x;

		// Calculate the desired delta velocity
		glm::vec3 contactNormal = contactData.mContact.mNormal;
		float desiredDeltaVelocity	= glm::dot(contactData.mContactBodies[0]->mLinearAcceleration, contactNormal) * delta;
		if (contactData.mContactBodies[1]) {
			desiredDeltaVelocity	-= glm::dot(contactData.mContactBodies[1]->mLinearAcceleration, contactNormal) * delta;
		}
		desiredDeltaVelocity		= -closingVelocity.x - RESTITUTION * (closingVelocity.x - desiredDeltaVelocity);

		// Calculate the impulse needed in the direction of the contact normal
		glm::vec3 impulseContact	= glm::vec3(desiredDeltaVelocity / deltaVelocity, 0.0f, 0.0f);
		glm::vec3 impulseWorld		= contactData.mContactToWorldMatrix * impulseContact;

		// Calculate the change in the velocities of the RigidBodies due to
		// the impulse
		for (size_t i = 0; i < 2; ++i) {
			if (contactData.mContactBodies[i]) {
				float sign = (i == 0) ? 1.0f : -1.0f;
				contactData.mVelocityChange[i]	= sign * impulseWorld * contactData.mContactBodies[i]->getInvertedMass();
	
   				glm::mat3 invertedInertiaTensor	= contactData.mContactBodies[i]->getInvertedInertiaTensor();
    			glm::vec3 torquePerImpulse		= sign * glm::cross(impulseWorld, contactData.mRelativePositions[i]);
    			contactData.mRotationChange[i]	= invertedInertiaTensor * torquePerImpulse;
			}
		}

		// Apply the changes
		for (size_t i = 0; i < 2; ++i) {
			if (contactData.mContactBodies[i]) {
				contactData.mContactBodies[i]->mLinearVelocity += contactData.mVelocityChange[i];
				contactData.mContactBodies[i]->mAngularVelocity += contactData.mRotationChange[i];
			}
		}
	}


	void CollisionResolver::updateOtherContacts(ContactData& contactData)
	{
        for (auto it = mContacts.begin(); it != mContacts.end(); ++it) {
			if (it->mContactBodies[0]) {
				if (it->mContactBodies[0] == contactData.mContactBodies[0]) {
					glm::vec3 contactPoint = glm::cross(contactData.mRotationChange[0], contactData.mRelativePositions[0]);
					contactPoint += contactData.mVelocityChange[0];
					
					float penetrationChange = glm::dot(contactPoint, contactData.mContact.mNormal);
					contactData.mContact.mPenetration += penetrationChange;
				}
				else if (it->mContactBodies[0] == contactData.mContactBodies[1]) {
					glm::vec3 contactPoint = glm::cross(contactData.mRotationChange[1], contactData.mRelativePositions[1]);
					contactPoint += contactData.mVelocityChange[1];
	
					float penetrationChange = glm::dot(contactPoint, contactData.mContact.mNormal);
					contactData.mContact.mPenetration += penetrationChange;
				}
			}
			else if (it->mContactBodies[1]) {
			   	if (it->mContactBodies[1] == contactData.mContactBodies[1]) {
					glm::vec3 contactPoint = glm::cross(contactData.mRotationChange[1], contactData.mRelativePositions[1]);
					contactPoint += contactData.mVelocityChange[1];

					float penetrationChange = glm::dot(contactPoint, contactData.mContact.mNormal);
					contactData.mContact.mPenetration += penetrationChange;
				}
				else if (it->mContactBodies[1] == contactData.mContactBodies[0]) {
					glm::vec3 contactPoint = glm::cross(contactData.mRotationChange[0], contactData.mRelativePositions[0]);
					contactPoint += contactData.mVelocityChange[0];

					float penetrationChange = glm::dot(contactPoint, contactData.mContact.mNormal);
					contactData.mContact.mPenetration += penetrationChange;
				}
			}
		}
	}

}
