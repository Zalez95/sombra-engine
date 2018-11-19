#include <glm/gtc/matrix_transform.hpp>
#include "se/graphics/3D/Camera.h"

namespace se::graphics {

	glm::mat4 Camera::getViewMatrix() const
	{
		return glm::lookAt(mPosition, mTarget, mUp);
	}

}
