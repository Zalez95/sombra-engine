#include <glm/gtc/matrix_transform.hpp>
#include "se/app/CameraComponent.h"

namespace se::app {

	void CameraComponent::setOrthographicProjectionMatrix(
		float left, float right, float bottom, float top,
		float zNear, float zFar
	) {
		mProjectionMatrix = glm::ortho(left, right, bottom, top, zNear, zFar);
	}


	void CameraComponent::setPerspectiveProjectionMatrix(float fovy, float aspectRatio, float zNear, float zFar)
	{
		mProjectionMatrix = glm::perspective(fovy, aspectRatio, zNear, zFar);
	}


	glm::mat4 CameraComponent::getViewMatrix() const
	{
		return glm::lookAt(mPosition, mTarget, mUp);
	}

}
