#ifndef DATA_SOURCE_H
#define DATA_SOURCE_H

#include <memory>

struct ma_sound;

namespace se::audio {

	class AudioEngine;


	/**
	 * Class DataSource, a data source is an object used to create, delete and
	 * access to the buffered audio data. A DataSource can be shared between
	 * multiple Sounds to reduce memory consumption.
	 */
	class DataSource
	{
	private:	// Attributes
		friend class Sound;

		/** The data source owner (sound) */
		std::unique_ptr<ma_sound> mDataSourceOwner;

	public:		// Functions
		/** Creates a new DataSource */
		DataSource();
		DataSource(const DataSource& other);
		DataSource(DataSource&& other);

		/** Class destructor */
		~DataSource();

		/** Assignment operator */
		DataSource& operator=(const DataSource& other);
		DataSource& operator=(DataSource&& other);

		/** @return	true if the DataSource was initialized successfully,
		 *			false otherwise */
		bool good() const { return (mDataSourceOwner != nullptr); };

		/** Creates a new DataSource from the given file
		 *
		 * @param	engine the engine that will hold the DataSource
		 * @param	path the where the file is located
		 * @return	the new DataSource */
		static DataSource createFromFile(
			AudioEngine& engine, const char* path
		);
	};

}

#endif		// DATA_SOURCE_H
