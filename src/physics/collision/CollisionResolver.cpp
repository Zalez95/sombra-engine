#include "CollisionResolver.h"
#include <glm/gtc/quaternion.hpp>
#include "Contact.h"
#include "../RigidBody.h"

namespace physics {

    void CollisionResolver::addContact(Contact* contact, RigidBody* rb1, RigidBody* rb2)
	{
		if ( contact && (rb1 || rb2) ) {
			ContactData contactData;
			contactData.mContact = contact;
			contactData.mContactBodies[0] = rb1;
			contactData.mContactBodies[1] = rb2;

			mContacts.push_back(contactData);
		}
	}


    void CollisionResolver::resolve(float delta)
	{
		for (ContactData& curContact : mContacts) {
	        prepareContact(&curContact);
		}

        while (!mContacts.empty()) {
			// Update the contact with the biggest penetration
			auto maxIt = mContacts.begin();
			for (auto it = maxIt; it != mContacts.end(); ++it) {
				if (it->mContact->getPenetration() > maxIt->mContact->getPenetration()) {
					maxIt = it;
				}
			}

			ContactData* maxContact = &(*maxIt);
			prepareContact(maxContact);
	    	calculatePositionChanges(maxContact);
	    	calculateVelocityChanges(maxContact, delta);
			updateOtherContacts(maxContact);

            mContacts.erase(maxIt);
        }
	}

// Private functions
	void CollisionResolver::prepareContact(ContactData* contactData)
    {
        // If there is only one body, it must be in the first position
		if (!contactData->mContactBodies[0]) {
			RigidBody* tmp = contactData->mContactBodies[0];
			contactData->mContactBodies[0] = contactData->mContactBodies[1];
			contactData->mContactBodies[1] = tmp;
		}

		// Calculate the contact space to world space matrix
		contactData->mContactToWorldMatrix = contactData->mContact->getContactToWorldMatrix();

        // Calculate the positions of the contact relative to each of the
		// RigidBodies
		glm::vec3 contactPosition = contactData->mContact->getPosition();
		for (unsigned int i = 0; i < 2; ++i) {
			if (contactData->mContactBodies[i]) {
	 			contactData->mRelativePositions[i] = contactPosition - contactData->mContactBodies[i]->getPosition();
			}
		}

        // Calculate the closing velocity of the contact relative to each
		// RigidBody
		for (unsigned int i = 0; i < 2; ++i) {
			if (contactData->mContactBodies[i]) {
				RigidBody* rb = contactData->mContactBodies[i];
				contactData->mRelativeVelocities[i] = glm::cross(rb->getAngularVelocity(), contactData->mRelativePositions[i]);
				contactData->mRelativeVelocities[i] += rb->getLinearVelocity();
			}
		}
    }

	
	void CollisionResolver::calculatePositionChanges(ContactData* contactData)
    {
		glm::vec3 contactNormal	= contactData->mContact->getNormal();
		float penetration		= contactData->mContact->getPenetration();

        // Calculate linear, angular and total inertia of both RigidBodies in
		// the direction of the Contact normal
		float totalInertia = 0.0f, linearInertia[2], angularInertia[2];
		for (unsigned int i = 0; i < 2; ++i) {
			if (contactData->mContactBodies[i]) {
				linearInertia[i] = contactData->mContactBodies[i]->getInvertedMass();
				
				glm::mat3 invertedInertiaTensor	= contactData->mContactBodies[i]->getInvertedInertiaTensor();
				glm::vec3 angularInertiaWorld	= glm::cross(contactData->mRelativePositions[i], contactNormal);
				angularInertiaWorld				= invertedInertiaTensor * angularInertiaWorld;
				angularInertiaWorld				= glm::cross(angularInertiaWorld, contactData->mRelativePositions[i]);
				angularInertia[i]				= glm::dot(angularInertiaWorld, contactNormal);

				totalInertia += linearInertia[i] + angularInertia[i];
			}
		}
		
		// Calculate the change in position and orientation of the RigidBodies
		for (unsigned int i = 0; i < 2; ++i) {
			if (contactData->mContactBodies[i]) {
				int sign							= (i == 0)? 1 : -1;
				float displacementNeeded			= sign * penetration * linearInertia[i] / totalInertia;
				float rotationNeeded				= sign * penetration * angularInertia[i] / totalInertia;

				// Check that the rotationNeeded doesn't exced the limit
				float limit							= ANGULAR_LIMIT * glm::length(contactData->mRelativePositions[i]);
				if (abs(rotationNeeded) > limit) {
					float total						= displacementNeeded + rotationNeeded;
					rotationNeeded					= (rotationNeeded >= 0)? limit : -limit;
					displacementNeeded				= total - rotationNeeded;
				}

				contactData->mPositionChange[i]		= contactNormal * displacementNeeded;
		
				if (rotationNeeded != 0) {			// Check if we need to rotate the RigidBodies
					glm::mat3 invertedInertiaTensor		= contactData->mContactBodies[i]->getInvertedInertiaTensor();
					glm::vec3 impulseTorque				= glm::cross(contactData->mRelativePositions[i], contactNormal);
					glm::vec3 impulsePerMove			= invertedInertiaTensor * impulseTorque;
					contactData->mOrientationChange[i]	= glm::quat(rotationNeeded * impulsePerMove / angularInertia[i]);
				}
			}
		}

		// Apply the changes
		for (unsigned int i = 0; i < 2; ++i) {
			if (contactData->mContactBodies[i]) {
				RigidBody* rb = contactData->mContactBodies[i];
				rb->setPosition(rb->getPosition() + contactData->mPositionChange[i]);
				rb->setOrientation(rb->getOrientation() * contactData->mOrientationChange[i]);
				rb->updateTransformsMatrix();
				rb->updateInertiaTensorWorld();
			}
		}
    }

	
	void CollisionResolver::calculateVelocityChanges(ContactData* contactData, float delta)
    {
		// Calculate the closing velocity at the contact point
		glm::vec3 closingVelocity	= contactData->mContactToWorldMatrix * contactData->mRelativeVelocities[0];
		closingVelocity				+= contactData->mContactToWorldMatrix * contactData->mRelativeVelocities[1];
			
		// Calculate the velocity change needed depending on the material
		// properties
		float deltaVelocity			= -(1 + RESTITUTION) * closingVelocity.x;

		// Calculate the desired delta velocity
		RigidBody* rb1				= contactData->mContactBodies[0];
		float desiredDeltaVelocity	= glm::dot(rb1->getLinearAcceleration(), contactData->mContact->getNormal()) * delta;
		if (contactData->mContactBodies[1]) {
			RigidBody* rb2			= contactData->mContactBodies[0];
			desiredDeltaVelocity	-= glm::dot(rb2->getLinearAcceleration(), contactData->mContact->getNormal()) * delta;
		}
		desiredDeltaVelocity		= -closingVelocity.x - RESTITUTION * (closingVelocity.x - desiredDeltaVelocity);

		// Calculate the impulse needed in the direction of the contact normal
		glm::vec3 impulseContact	= glm::vec3(desiredDeltaVelocity / deltaVelocity, 0.0f, 0.0f);
		glm::vec3 impulseWorld		= contactData->mContactToWorldMatrix * impulseContact;

		// Calculate the change in the velocities of the RigidBodies due to
		// the impulse
		for (unsigned int i = 0; i < 2; ++i) {
			if (contactData->mContactBodies[i]) {
				if (i == 1) { impulseWorld *= -1; }
				contactData->mVelocityChange[i]	= impulseWorld * contactData->mContactBodies[i]->getInvertedMass();
	
   				glm::mat3 invertedInertiaTensor	= contactData->mContactBodies[i]->getInvertedInertiaTensor();
    			glm::vec3 torquePerImpulse		= glm::cross(impulseWorld, contactData->mRelativePositions[i]);
    			contactData->mRotationChange[i]	= invertedInertiaTensor * torquePerImpulse;
			}
		}

		// Apply the changes
		for (unsigned int i = 0; i < 2; ++i) {
			if (contactData->mContactBodies[i]) {
				RigidBody* rb = contactData->mContactBodies[i];
				rb->addLinearVelocity(contactData->mVelocityChange[i]);
				rb->addAngularVelocity(contactData->mRotationChange[i]);
			}
		}
	}


	void CollisionResolver::updateOtherContacts(ContactData* contactData)
	{
        for (auto it = mContacts.begin(); it != mContacts.end(); ++it) {
			if (it->mContactBodies[0]) {
				if (it->mContactBodies[0] == contactData->mContactBodies[0]) {
					glm::vec3 contactPoint = glm::cross(contactData->mRotationChange[0], contactData->mRelativePositions[0]);
					contactPoint += contactData->mVelocityChange[0];
					
					float penetrationChange = glm::dot(contactPoint, contactData->mContact->getNormal());
					contactData->mContact->setPenetration(contactData->mContact->getPenetration() + penetrationChange);
				}
				else if (it->mContactBodies[0] == contactData->mContactBodies[1]) {
					glm::vec3 contactPoint = glm::cross(contactData->mRotationChange[1], contactData->mRelativePositions[1]);
					contactPoint += contactData->mVelocityChange[1];
	
					float penetrationChange = glm::dot(contactPoint, contactData->mContact->getNormal());
					contactData->mContact->setPenetration(contactData->mContact->getPenetration() + penetrationChange);
				}
			}
			else if (it->mContactBodies[1]) {
			   	if (it->mContactBodies[1] == contactData->mContactBodies[1]) {
					glm::vec3 contactPoint = glm::cross(contactData->mRotationChange[1], contactData->mRelativePositions[1]);
					contactPoint += contactData->mVelocityChange[1];

					float penetrationChange = glm::dot(contactPoint, contactData->mContact->getNormal());
					contactData->mContact->setPenetration(contactData->mContact->getPenetration() + penetrationChange);
				}
				else if (it->mContactBodies[1] == contactData->mContactBodies[0]) {
					glm::vec3 contactPoint = glm::cross(contactData->mRotationChange[0], contactData->mRelativePositions[0]);
					contactPoint += contactData->mVelocityChange[0];

					float penetrationChange = glm::dot(contactPoint, contactData->mContact->getNormal());
					contactData->mContact->setPenetration(contactData->mContact->getPenetration() + penetrationChange);
				}
			}
		}
	}

}
