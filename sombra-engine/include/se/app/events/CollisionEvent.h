#ifndef COLLISION__EVENT_H
#define COLLISION__EVENT_H

#include "../IEvent.h"
#include "../../collision/Manifold.h"

namespace se::app {

	struct Entity;


	/**
	 * Class CollisionEvent, its an event used for notify of collision
	 * detected by the CollisionManager
	 */
	class CollisionEvent : public IEvent
	{
	public:		// Attributes
		static constexpr Topic kTopic = Topic::Collision;
	private:
		/** The Entities affected by the CollisionEvent */
		Entity* mEntities[2];

		/** A pointer to the collision Manifold with the collision data */
		const collision::Manifold* mManifold;

	public:		// Functions
		/** Creates a new CollisionEvent
		 *
		 * @param	entity1 a pointer to the first of the Collision
		 * @param	entity2 a pointer to the second of the Collision
		 * @param	manifold a pointer to the collision Manifold */
		CollisionEvent(
			Entity* entity1, Entity* entity2,
			const collision::Manifold* manifold
		) : mEntities{ entity1, entity2 }, mManifold(manifold) {};

		/** Class destructor */
		virtual ~CollisionEvent() {};

		/** @return	the Topic of the CollisionEvent */
		virtual Topic getTopic() const override { return kTopic; };

		/** Returns a pointer to the requested Entity
		 *
		 * @param	second the flag used to select the Entity to return
		 * @return	a pointer to the second Entity if the second flag is
		 *			true, the first one otherwise */
		Entity* getEntity(bool second) const
		{ return mEntities[second]; };

		/** @return	a pointer to the collision Manifold */
		const collision::Manifold* getManifold() const { return mManifold; };
	protected:
		/** Appends the current CollisionEvent formated as text to the given
		 * ostream
		 *
		 * @param	os a reference to the ostream where we want to print the
		 *			current CollisionEvent */
		virtual void printTo(std::ostream& os) const
		{
			os	<< "{ kTopic : " << Topic::Collision << ", mEntities : [ "
				<< mEntities[0] << ", " << mEntities[1] << " ], mManifold : "
				<< mManifold << " }";
		};
	};

}

#endif		// COLLISION__EVENT_H
