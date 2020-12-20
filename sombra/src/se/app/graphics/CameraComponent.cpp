#include <glm/gtc/matrix_transform.hpp>
#include "se/app/CameraComponent.h"

namespace se::app {

	bool CameraComponent::getOrthographicParams(
		float& left, float& right, float& bottom, float& top,
		float& zNear, float& zFar
	) const
	{
		if (mIsOrthographic) {
			left	= mProjectionParams[0];
			right	= mProjectionParams[1];
			bottom	= mProjectionParams[2];
			top		= mProjectionParams[3];
			zNear	= mProjectionParams[4];
			zFar	= mProjectionParams[5];
			return true;
		}

		return false;
	}


	void CameraComponent::setOrthographicProjection(
		float left, float right, float bottom, float top,
		float zNear, float zFar
	) {
		mIsOrthographic = true;
		mProjectionParams = { left, right, bottom, top, zNear, zFar };
		mProjectionMatrix = glm::ortho(left, right, bottom, top, zNear, zFar);
	}


	bool CameraComponent::getPerspectiveParams(float& fovy, float& aspectRatio, float& zNear, float& zFar) const
	{
		if (!mIsOrthographic) {
			fovy		= mProjectionParams[0];
			aspectRatio	= mProjectionParams[1];
			zNear		= mProjectionParams[2];
			zFar		= mProjectionParams[3];
			return true;
		}

		return false;
	}


	void CameraComponent::setPerspectiveProjection(float fovy, float aspectRatio, float zNear, float zFar)
	{
		mIsOrthographic = false;
		mProjectionParams = { fovy, aspectRatio, zNear, zFar, 0.0f, 0.0f };
		mProjectionMatrix = glm::perspective(fovy, aspectRatio, zNear, zFar);
	}


	glm::mat4 CameraComponent::getViewMatrix() const
	{
		return glm::lookAt(mPosition, mTarget, mUp);
	}

}
