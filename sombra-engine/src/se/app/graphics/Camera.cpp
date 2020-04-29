#include <glm/gtc/matrix_transform.hpp>
#include "se/app/graphics/Camera.h"

namespace se::app {

	void Camera::setOrthographicProjectionMatrix(float xMagnification, float yMagnification, float zNear, float zFar)
	{
		mProjectionMatrix = glm::ortho(0.0f, xMagnification, 0.0f, yMagnification, zNear, zFar);
	}


	void Camera::setPerspectiveProjectionMatrix(float fovy, float aspectRatio, float zNear, float zFar)
	{
		mProjectionMatrix = glm::perspective(fovy, aspectRatio , zNear, zFar);
	}


	glm::mat4 Camera::getViewMatrix() const
	{
		return glm::lookAt(mPosition, mTarget, mUp);
	}

}
