#ifndef LIGHT_SOURCE_EVENT_H
#define LIGHT_SOURCE_EVENT_H

#include <memory>
#include "../Entity.h"
#include "Event.h"

namespace se::app {

	class LightSource;


	/**
	 * Class LightSourceEvent, its an event used for notify of a LightSource or
	 * LightComponent updates
	 */
	class LightSourceEvent : public Event<Topic::LightSource>
	{
	public:		// Nested types
		using LightSourceSPtr = std::shared_ptr<LightSource>;

	private:	// Attributes
		/** A pointer to the LightSource affected by the update */
		LightSourceSPtr mLightSource;

		/** The Entity affected by the update */
		Entity mEntity;

	public:		// Functions
		/** Creates a new LightSourceEvent used for notifying of a LightSource
		 * or a LightComponent update
		 *
		 * @param	source a pointer to the LightSource affected
		 * @param	entity the Entity affected by the update */
		LightSourceEvent(
			const LightSourceSPtr& source, Entity entity = kNullEntity
		) : mLightSource(source), mEntity(entity) {};

		/** @return	the LightSource of the Event */
		const LightSourceSPtr& getLightSource() const { return mLightSource; };

		/** @return	the Entity affected by the Event */
		Entity getEntity() const { return mEntity; };
	private:
		/** @copydoc Event::printTo() */
		virtual void printTo(std::ostream& os) const override
		{
			os	<< "{ kTopic : " << kTopic
				<< ", mLightSource : " << mLightSource
				<< ", mEntity : " << mEntity << " }";
		};
	};

}

#endif		// LIGHT_SOURCE_EVENT_H
