#ifndef COLLISION_EVENT_H
#define COLLISION_EVENT_H
#if 0
#include "Event.h"
#include "../../collision/Manifold.h"
#include "../Entity.h"

namespace se::app {

	/**
	 * Class CollisionEvent, its an event used for notify of collision
	 * detected by the CollisionManager
	 */
	class CollisionEvent : public Event<Topic::Collision>
	{
	private:	// Attributes
		/** The Entities affected by the CollisionEvent */
		Entity mEntities[2];

		/** A pointer to the collision Manifold with the collision data */
		const collision::Manifold* mManifold;

	public:		// Functions
		/** Creates a new CollisionEvent
		 *
		 * @param	entity1 the first Entity of the Collision
		 * @param	entity2 the second Entity of the Collision
		 * @param	manifold a pointer to the collision Manifold */
		CollisionEvent(
			Entity entity1, Entity entity2,
			const collision::Manifold* manifold
		) : mEntities{ entity1, entity2 }, mManifold(manifold) {};

		/** Returns the requested Entity
		 *
		 * @param	second the flag used for selecting the Entity to return
		 * @return	the second Entity if the second flag is true, the first
		 *			one otherwise */
		Entity getEntity(bool second) const
		{ return mEntities[second]; };

		/** @return	a pointer to the collision Manifold */
		const collision::Manifold* getManifold() const { return mManifold; };
	private:
		/** @copydoc Event::printTo() */
		virtual void printTo(std::ostream& os) const override
		{
			os	<< "{ kTopic : " << kTopic << ", mEntities : [ "
				<< mEntities[0] << ", " << mEntities[1] << " ], mManifold : "
				<< mManifold << " }";
		};
	};

}
#endif
#endif		// COLLISION_EVENT_H
