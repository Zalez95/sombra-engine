#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace graphics {
	
	glm::mat4 Camera::getViewMatrix() const
	{
		return glm::lookAt(mPosition, mTarget, mUp);
	};

}
