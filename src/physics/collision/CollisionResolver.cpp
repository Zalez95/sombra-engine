#include "CollisionResolver.h"
#include <glm/gtc/quaternion.hpp>
#include "Contact.h"
#include "../RigidBody.h"

namespace physics {

    void CollisionResolver::addContact(Contact* contact, RigidBody* rb1, RigidBody* rb2)
	{
		if (contact && rb1) {
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
            mContacts.erase(maxIt);

			ContactData* maxContact = &(*maxIt);
	    	//calculatePositionChanges(maxContact);
	    	calculateVelocityChanges(maxContact);
			//updateOtherContacts(maxContact);
        }
	}

// Private functions
	void CollisionResolver::prepareContact(ContactData* contactData)
    {
        // If there is only one body, it must be in the first position
		RigidBody* tmp = contactData->mContactBodies[0];
		contactData->mContactBodies[0] = contactData->mContactBodies[1];
		contactData->mContactBodies[1] = tmp;

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
			RigidBody* body = contactData->mContactBodies[i];
			if (body) {
				contactData->mRelativeVelocities[i] = glm::cross(body->getAngularVelocity(), contactData->mRelativePositions[i]);
				contactData->mRelativeVelocities[i] += body->getVelocity();
			}
		}
    }

	
	void CollisionResolver::calculatePositionChanges(ContactData* contactData)
    {
		glm::vec3 contactNormal = contactData->mContact->getNormal();

        // Calculate linear, angular and total inertia of both RigidBodies in
		// the direction of the Contact normal
		float totalInertia, linearInertia[2], angularInertia[2];
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
		float displacementNeeded[2], rotationNeeded[2];
		
		float penetration = contactData->mContact->getPenetration();
		displacementNeeded[0]	= penetration * linearInertia[0] / totalInertia;
		displacementNeeded[1]	= -penetration * linearInertia[1] / totalInertia;
		rotationNeeded[0]		= penetration * angularInertia[0] / totalInertia;
		rotationNeeded[1]		= -penetration * angularInertia[1] / totalInertia;
		
		for (unsigned int i = 0; i < 2; ++i) {
			if (contactData->mContactBodies[i]) {
				contactData->mPositionChange[i]		= contactNormal * displacementNeeded[i];
		
				glm::mat3 invertedInertiaTensor		= contactData->mContactBodies[i]->getInvertedInertiaTensor();
				glm::vec3 impulseTorque				= glm::cross(contactData->mRelativePositions[i], contactNormal);
				glm::vec3 impulseVelocity			= invertedInertiaTensor * impulseTorque;
				contactData->mOrientationChange[i]	= rotationNeeded[i] * impulseVelocity / angularInertia[i];
			}
		}

		// Apply the changes
		for (unsigned int i = 0; i < 2; ++i) {
			if (contactData->mContactBodies[i]) {
				RigidBody* rb = contactData->mContactBodies[i];
				rb->setPosition(rb->getPosition() + contactData->mPositionChange[i]);
				rb->setOrientation(rb->getOrientation() * glm::angleAxis(0.0f, contactData->mOrientationChange[i]));
			}
		}
    }

	
	void CollisionResolver::calculateVelocityChanges(ContactData* contactData)
    {
		// Calculate the closing velocity at the contact point
		glm::vec3 closingVelocity	= contactData->mContactToWorldMatrix * contactData->mRelativeVelocities[0];
		closingVelocity				+= contactData->mContactToWorldMatrix * contactData->mRelativeVelocities[1];
			
		// Calculate the velocity change needed depending on the material
		// properties
		float deltaVelocity			= -(1 + RESTITUTION) * closingVelocity.x;

		// TODO: Calculate the desired delta velocity
		float desiredDeltaVelocity = 1.0f;

		// Calculate the impulse needed in the direction of the contact normal
		glm::vec3 impulseContact(desiredDeltaVelocity / deltaVelocity, 0.0f, 0.0f);
		glm::vec3 impulseWorld		= contactData->mContactToWorldMatrix * impulseContact;

		// Calculate the change in the velocities of the RigidBodies due to
		// the impulse
		for (unsigned int i = 0; i < 2; ++i) {
			if (i == 1) { impulseWorld *= -1; }
			contactData->mVelocityChange[i]	= impulseWorld * contactData->mContactBodies[i]->getInvertedMass();

   			glm::mat3 invertedInertiaTensor	= contactData->mContactBodies[i]->getInvertedInertiaTensor();
    		glm::vec3 torquePerImpulse		= glm::cross(impulseWorld, contactData->mRelativePositions[i]);
    		contactData->mRotationChange[i]	= invertedInertiaTensor * torquePerImpulse;
		}

		// Apply the changes
		for (unsigned int i = 0; i < 2; ++i) {
			if (contactData->mContactBodies[i]) {
				RigidBody* rb = contactData->mContactBodies[i];
				rb->setVelocity(rb->getVelocity() + contactData->mVelocityChange[i]);
				rb->setAngularVelocity(rb->getAngularVelocity() + contactData->mRotationChange[i]);
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
