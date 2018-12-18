#ifndef MATERIAL_H
#define MATERIAL_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace se::graphics {

	/**
	 * Struct Material, it holds the data of the surface material properties
	 * of a Renderable3D, used for the Phong Shading in the shaders
	 */
	struct Material
	{
		/** The name of the Material */
		std::string name;

		/** The ambient color of the Material */
		glm::vec3 ambientColor;

		/** The color of the diffuse reflected light the Material */
		glm::vec3 diffuseColor;

		/** The color of the specular reflected light the Material */
		glm::vec3 specularColor;

		/** The specular shininess of the Material */
		float shininess;
	};

}

#endif		// MATERIAL_H
