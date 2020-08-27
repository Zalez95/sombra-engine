#include <glm/gtc/matrix_transform.hpp>
#include "se/app/graphics/CameraComponent.h"

namespace se::app {

	void CameraComponent::setOrthographicProjectionMatrix(float xMagnification, float yMagnification, float zNear, float zFar)
	{
		mProjectionMatrix = glm::ortho(0.0f, xMagnification, 0.0f, yMagnification, zNear, zFar);
	}


	void CameraComponent::setPerspectiveProjectionMatrix(float fovy, float aspectRatio, float zNear, float zFar)
	{
		mProjectionMatrix = glm::perspective(fovy, aspectRatio , zNear, zFar);
	}


	glm::mat4 CameraComponent::getViewMatrix() const
	{
		return glm::lookAt(mPosition, mTarget, mUp);
	}

}
