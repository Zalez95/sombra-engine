#ifndef AUDIO_SOURCE_COMPONENT_H
#define AUDIO_SOURCE_COMPONENT_H

#include "../audio/Source.h"
#include "Repository.h"

namespace se::app {

	/**
	 * Class AudioSourceComponent, It's a wrapper for the audio::Source class,
	 * used for mantaining the references to the Repository audio::Buffers
	 */
	class AudioSourceComponent
	{
	private:	// Nested types
		using BufferRef = Repository::ResourceRef<audio::Buffer>;

	private:	// Attributes
		/** The audio source */
		audio::Source mSource;

		/** A pointer to the buffer that holds the audio data of the Source */
		BufferRef mBuffer;

	public:		// Functions
		/** @return	the raw audio source of the AudioSourceComponent */
		audio::Source& get() { return mSource; };

		/** @return	the raw audio source of the AudioSourceComponent */
		const audio::Source& get() const { return mSource; };

		/** @return	a pointer to the Buffer of the AudioSourceComponent */
		const BufferRef& getBuffer() const { return mBuffer; };

		/** Sets the Buffer of the audio Source
		 *
		 * @param	buffer the new buffer of the AudioSourceComponent
		 * @return	a reference to the current AudioSourceComponent object */
		AudioSourceComponent& setBuffer(const BufferRef& buffer)
		{
			mBuffer = buffer;
			mSource.bind(*mBuffer);
			return *this;
		};
	};

}

#endif		// AUDIO_SOURCE_COMPONENT_H
