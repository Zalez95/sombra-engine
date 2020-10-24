#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

#include <glm/glm.hpp>

namespace se::app {

	/**
	 * Class CameraComponent, is a 3D GraphicsEntity that sets the position and
	 * direction of a Camera
	 */
	class CameraComponent
	{
	private:	// Attributes
		/** The projection matrix of the renderer that transforms from View
		 * Space to Projection Space */
		glm::mat4 mProjectionMatrix = glm::mat4(1.0f);

		/** The position coordinates of the CameraComponent in world space */
		glm::vec3 mPosition = {};

		/** The point where the CameraComponent is pointing to in world space */
		glm::vec3 mTarget = {};

		/** The Up vector of the CameraComponent in world space */
		glm::vec3 mUp = {};

	public:		// Functions
		/** Sets the projection matrix of the CameraComponent
		 *
		 * @param	left the left coordinate of the screen
		 * @param	right the right coordinate of the screen
		 * @param	bottom the bottom coordinate of the screen
		 * @param	top the top coordinate of the screen
		 * @param	zNear the distance to the near clipping plane
		 * @param	zFar the distance to the far clipping plane */
		void setOrthographicProjectionMatrix(
			float left, float right, float bottom, float top,
			float zNear, float zFar
		);

		/** Sets the projection matrix of the CameraComponent
		 *
		 * @param	fovy the vertical field of view in radians
		 * @param	aspectRatio the aspect ratio of the field of view
		 * @param	zNear the distance to the near clipping plane
		 * @param	zFar the distance to the far clipping plane */
		void setPerspectiveProjectionMatrix(
			float fovy, float aspectRatio, float zNear, float zFar
		);

		/** @return	the projection matrix of the CameraComponent that transforms
		 *			from view space to projection space */
		const glm::mat4& getProjectionMatrix() const
		{ return mProjectionMatrix; };

		/** @return the position of the CameraComponent in world space */
		inline glm::vec3 getPosition() const { return mPosition; };

		/** Sets the position of the CameraComponent
		 *
		 * @param	position the new position of the CameraComponent in world
		 *			space */
		inline void setPosition(const glm::vec3& position)
		{ mPosition = position; };

		/** @return the target point of the CameraComponent in world space */
		inline glm::vec3 getTarget() const { return mTarget; };

		/** Sets the target point of the CameraComponent
		 *
		 * @param	target the new point where the CameraComponent is pointing
		 *			to in world space */
		inline void setTarget(const glm::vec3& target)
		{ mTarget = target; };

		/** @return	the up vector of the CameraComponent in world space */
		inline glm::vec3 getUp() const { return mUp; };

		/** Sets the up vector of the CameraComponent
		 *
		 * @param	up the new up vector of the CameraComponent in world
		 *			space */
		inline void setUp(const glm::vec3& up)
		{ mUp = up; };

		/** @return	the view matrix of the CameraComponent that transforms from
		 *			world space to view space */
		glm::mat4 getViewMatrix() const;
	};

}

#endif		// CAMERA_COMPONENT_H
