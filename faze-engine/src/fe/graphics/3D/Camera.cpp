#include <glm/gtc/matrix_transform.hpp>
#include "fe/graphics/3D/Camera.h"

namespace fe { namespace graphics {

	glm::mat4 Camera::getViewMatrix() const
	{
		return glm::lookAt(mPosition, mTarget, mUp);
	}

}}
