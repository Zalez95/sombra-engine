#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

namespace se::app {

	/**
	 * Class Camera, is a 3D GraphicsEntity that sets the position and
	 * direction of a Camera
	 */
	class Camera
	{
	private:	// Attributes
		/** The projection matrix of the renderer that transforms from View
		 * Space to Projection Space */
		glm::mat4 mProjectionMatrix;

		/** The position coordinates of the Camera in world space */
		glm::vec3 mPosition;

		/** The point where the Camera is pointing to in world space */
		glm::vec3 mTarget;

		/** The Up vector of the Camera in world space */
		glm::vec3 mUp;

	public:		// Functions
		/** Creates a new Camera */
		Camera() :
			mProjectionMatrix(1.0f),
			mPosition(0.0f), mTarget(0.0f), mUp(0.0f) {};

		/** Sets the projection matrix of the Camera
		 *
		 * @param	xMagnification the horizontal magnification of the view
		 * @param	yMagnification the vertical magnification of the view
		 * @param	zNear the distance to the near clipping plane
		 * @param	zFar the distance to the far clipping plane */
		void setOrthographicProjectionMatrix(
			float xMagnification, float yMagnification, float zNear, float zFar
		);

		/** Sets the projection matrix of the Camera
		 *
		 * @param	fovy the vertical field of view in radians
		 * @param	aspectRatio the aspect ratio of the field of view
		 * @param	zNear the distance to the near clipping plane
		 * @param	zFar the distance to the far clipping plane */
		void setPerspectiveProjectionMatrix(
			float fovy, float aspectRatio, float zNear, float zFar
		);

		/** @return	the projection matrix of the Camera that transforms from
		 *			view space to projection space */
		const glm::mat4& getProjectionMatrix() const
		{ return mProjectionMatrix; };

		/** @return the position of the Camera in world space */
		inline glm::vec3 getPosition() const { return mPosition; };

		/** Sets the position of the Camera
		 *
		 * @param	position the new position of the camera in world space */
		inline void setPosition(const glm::vec3& position)
		{ mPosition = position; };

		/** @return the target point of the Camera in world space */
		inline glm::vec3 getTarget() const { return mTarget; };

		/** Sets the target point of the Camera
		 *
		 * @param	target the new point where the camera is pointing to
		 *			in world space */
		inline void setTarget(const glm::vec3& target)
		{ mTarget = target; };

		/** @return the up vector of the Camera in world space */
		inline glm::vec3 getUp() const { return mUp; };

		/** Sets the up vector of the Camera
		 *
		 * @param	up the new up vector of the camera in world space */
		inline void setUp(const glm::vec3& up)
		{ mUp = up; };

		/** @return	the view matrix of the Camera that transforms from world
		 *			space to view space */
		glm::mat4 getViewMatrix() const;
	};

}

#endif		// CAMERA_H
