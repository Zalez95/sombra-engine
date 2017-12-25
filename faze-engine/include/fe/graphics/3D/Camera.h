#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

namespace fe { namespace graphics {

	/**
	 * Class Camera, is a 3D GraphicsEntity that sets the position and
	 * direction of a Camera
	 */
	class Camera
	{
	private:	// Attributes
		/** The position coordinates of the Camera in world space */
		glm::vec3 mPosition;

		/** The point where the Camera is pointing to in world space */
		glm::vec3 mTarget;

		/** The Up vector of the Camera in world space */
		glm::vec3 mUp;

	public:		// Functions
		/** Creates a new Camera */
		Camera() {};

		/** Creates a new Camera
		 *
		 * @param	position the position of the new camera in world space
		 * @param	target the point where the Camera is pointing towards in
		 *			world space
		 * @param	up the up vector of the camera */
		Camera(
			const glm::vec3& position,
			const glm::vec3& target,
			const glm::vec3& up
		) : mPosition(position), mTarget(target), mUp(up) {}

		/** Class destructor */
		~Camera() {};

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

}}

#endif		// CAMERA_H
