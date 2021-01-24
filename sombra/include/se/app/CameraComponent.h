#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

#include <array>
#include <glm/glm.hpp>

namespace se::app {

	/**
	 * Class CameraComponent, is a 3D GraphicsEntity that sets the position and
	 * direction of a Camera
	 */
	class CameraComponent
	{
	private:	// Attributes
		/** If the Camera uses Orthographic or Perspective projection */
		bool mIsOrthographic = true;

		/** Holds all the parameters used for calculating the projection matrix
		 * @see setOrthographicProjection(float,float,float,float,float,float)
		 * and @see setPerspectiveProjection(float, float, float, float) */
		std::array<float, 6> mProjectionParams = {};

		/** The projection matrix of the renderer that transforms from View
		 * Space to Projection Space */
		glm::mat4 mProjectionMatrix = glm::mat4(1.0f);

		/** The position coordinates of the CameraComponent in world space */
		glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };

		/** The point where the CameraComponent is pointing to in world space */
		glm::vec3 mTarget = { 0.0f, 0.0f, 1.0f };

		/** The Up vector of the CameraComponent in world space */
		glm::vec3 mUp = { 0.0f, 1.0f, 0.0f };

	public:		// Functions
		/** @return	true if the Camera has orthographic projection, false if
		 *			has perspective projection */
		bool hasOrthographicProjection() const { return mIsOrthographic; };

		/** Returns the parameters used for calculating the orthographic
		 * projection
		 *
		 * @param	left the left coordinate of the screen
		 * @param	right the right coordinate of the screen
		 * @param	bottom the bottom coordinate of the screen
		 * @param	top the top coordinate of the screen
		 * @param	zNear the distance to the near clipping plane
		 * @param	zFar the distance to the far clipping plane
		 * @return	true if the params were returned successfully (the camera
		 *			has orthographic projection), otherwise it will return
		 *			false and the return params won't be updated */
		bool getOrthographicParams(
			float& left, float& right, float& bottom, float& top,
			float& zNear, float& zFar
		) const;

		/** Sets the projection matrix of the CameraComponent
		 *
		 * @param	left the left coordinate of the screen
		 * @param	right the right coordinate of the screen
		 * @param	bottom the bottom coordinate of the screen
		 * @param	top the top coordinate of the screen
		 * @param	zNear the distance to the near clipping plane
		 * @param	zFar the distance to the far clipping plane */
		void setOrthographicProjection(
			float left, float right, float bottom, float top,
			float zNear, float zFar
		);

		/** Returns the parameters used for calculating the perspective
		 * projection
		 *
		 * @param	fovy the vertical field of view in radians
		 * @param	aspectRatio the aspect ratio of the field of view
		 * @param	zNear the distance to the near clipping plane
		 * @param	zFar the distance to the far clipping plane
		 * @return	true if the params were returned successfully (the camera
		 *			has perspective projection), otherwise it will return
		 *			false and the return params won't be updated */
		bool getPerspectiveParams(
			float& fovy, float& aspectRatio, float& zNear, float& zFar
		) const;

		/** Sets the projection matrix of the CameraComponent
		 *
		 * @param	fovy the vertical field of view in radians
		 * @param	aspectRatio the aspect ratio of the field of view
		 * @param	zNear the distance to the near clipping plane
		 * @param	zFar the distance to the far clipping plane */
		void setPerspectiveProjection(
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
