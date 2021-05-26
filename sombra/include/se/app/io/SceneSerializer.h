#ifndef SCENE_SERIALIZER_H
#define SCENE_SERIALIZER_H

#include <memory>
#include "../Scene.h"
#include "Result.h"

namespace se::app {

	/**
	 * Class SceneSerializer, it's used for loading and storing full scenes
	 * from/in the given files
	 */
	class SceneSerializer
	{
	public:		// Functions
		/** Stores the given scene in the given file
		 *
		 * @param	path the path of the file where the Scene is going to be
		 *			stored
		 * @param	scene the Scene to store
		 * @return	a Result object with the result of the load operation */
		static Result serialize(const std::string& path, const Scene& scene);

		/** Reads the given file and loads the result in the given Scene
		 * object
		 *
		 * @param	path the path to the file to parse
		 * @param	output the Scene where the file data will be stored
		 * @return	a Result object with the result of the load operation */
		static Result deserialize(const std::string& path, Scene& output);
	};

}

#endif		// SCENE_SERIALIZER_H
