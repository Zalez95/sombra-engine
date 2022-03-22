#ifndef SOUND_COMPONENT_H
#define SOUND_COMPONENT_H

#include "../audio/Sound.h"
#include "Repository.h"

namespace se::app {

	/**
	 * Class SoundComponent, It's a wrapper for the audio::Sound class,
	 * used for mantaining the references to the Repository audio::Buffers
	 */
	class SoundComponent
	{
	public:		// Nested types
		using DataSourceResource = Repository::ResourceRef<audio::DataSource>;

	private:	// Attributes
		/** The audio sound */
		audio::Sound mSound;

		/** A pointer to the DataSource that holds the audio data of the
		 * Sound */
		DataSourceResource mDSource;

	public:		// Functions
		/** Creates a new Source */
		SoundComponent() = default;

		/** @return	the raw audio Sound of the SoundComponent */
		audio::Sound& get() { return mSound; };

		/** @return	the raw audio Sound of the SoundComponent */
		const audio::Sound& get() const { return mSound; };

		/** @return	a pointer to the DataSource of the SoundComponent */
		DataSourceResource& getDataSource() { return mDSource; };

		/** @return	a pointer to the DataSource of the SoundComponent */
		const DataSourceResource& getDataSource() const { return mDSource; };

		/** Sets the DataSource of the audio Sound
		 *
		 * @param	dataSource the new DataSource of the SoundComponent
		 * @return	a reference to the current SoundComponent object */
		SoundComponent& setDataSource(const DataSourceResource& dataSource)
		{
			mDSource = dataSource;
			mSound.bind(*dataSource);
			return *this;
		};
	};

}

#endif		// SOUND_COMPONENT_H
