#ifndef ISYSTEM_H
#define ISYSTEM_H

namespace se::app {

	/**
	 * Class System, it's the interface that each System must implement. A
	 * System is used for updating the Entities Components at every clock tick.
	 */
	class ISystem
	{
	protected:	// Attributes
		/** The EntityDatabase that holds all the Entities and their
		 * Components */
		EntityDatabase& mEntityDatabase;

		/** The elapsed time since the last @see update call */
		float mDeltaTime;

	public:		// Functions
		/** Creates a new ISystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities */
		ISystem(EntityDatabase& entityDatabase) :
			mEntityDatabase(entityDatabase), mDeltaTime(0.0f) {};

		/** Class destructor */
		virtual ~ISystem() = default;

		/** Function that the EntityDatabase will call when an Entity Component
		 * is added
		 *
		 * @param	entity the Entity that holds the Component
		 * @param	mask the ComponentMask that is used for knowing which
		 *			Component has been added
		 * @note	this function is called in the middle of a critical section,
		 *			so the entity or the component notified can't be removed
		 *			while we are working with it. This will also lock other
		 *			operations in the EntityDatabase */
		virtual void onNewComponent(
			Entity /*entity*/, const EntityDatabase::ComponentMask& /*mask*/
		) {};

		/** Function that the EntityDatabase will call when an Entity Component
		 * is going to be removed
		 *
		 * @param	entity the Entity that holds the Component
		 * @param	mask the ComponentMask that is used for knowing which
		 *			Component is going to be removed
		 * @note	this function is called in the middle of a critical section,
		 *			so the entity or the component notified can't be removed
		 *			while we are working with it. This will also lock other
		 *			operations in the EntityDatabase */
		virtual void onRemoveComponent(
			Entity /*entity*/, const EntityDatabase::ComponentMask& /*mask*/
		) {};

		/** Sets the delta time of the ISystem
		 *
		 * @param	deltaTime the elapsed time since the last @see update
		 *			call */
		void setDeltaTime(float deltaTime) { mDeltaTime = deltaTime; };

		/** Function called every clock tick */
		virtual void update() {};
	protected:
		/** Tries to call the given component handler function with the correct
		 * Component type
		 *
		 * @param	componentHandler a pointer to the function to call
		 * @param	entity the Entity that holds the Component
		 * @param	mask the ComponentMask that holds the type of the Component
		 *			to check */
		template <typename S, typename C>
		void tryCallC(
			void(S::*componentHandler)(Entity, C*),
			Entity entity, const EntityDatabase::ComponentMask& mask
		);
	};


	template <typename S, typename C>
	void ISystem::tryCallC(
		void(S::*componentHandler)(Entity, C*),
		Entity entity, const EntityDatabase::ComponentMask& mask
	) {
		if (mask.get<C>()) {
			S* thisS = static_cast<S*>(this);

			auto [component] = mEntityDatabase.getComponents<C>(entity);
			(thisS->*componentHandler)(entity, component);
		}
	}

}

#endif		// ISYSTEM_H
