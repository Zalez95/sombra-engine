#include <glm/gtc/matrix_transform.hpp>
#include "se/app/TransformsComponent.h"

namespace se::app {

	glm::mat4 getModelMatrix(const TransformsComponent& transforms)
	{
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms.position);
		glm::mat4 rotation		= glm::mat4_cast(transforms.orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), transforms.scale);
		return translation * rotation * scale;
	}

}
