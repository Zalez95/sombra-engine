#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct ma_context;
struct ma_device;
struct ma_engine;

namespace se::audio {

	/**
	 * Class AudioEngine, It's the class used to prepare the audio devices for
	 * playing sounds, and to set the properties of the Listener of the Sounds.
	 * The properties of this Listener can be used to control from where we are
	 * going to be listening the Sounds in this 3D audio scene.
	 */
	class AudioEngine
	{
	public:		// Nested Types
		friend class Sound;
		friend class DataSource;

		/** Holds the info about an audio device */
		struct DeviceInfo
		{
			std::string name;	///< The name of the device
			std::size_t id;		///< The id of the device
		};
	private:
		struct ResourceManager;

	private:	// Attributes
		/** The id of the single listener in @see mEngine */
		static constexpr unsigned int kListenerIndex = 0;

		/** The context used by the Engine */
		std::unique_ptr<ma_context> mContext;

		/** A pointer to the Device used for playing the sound */
		std::unique_ptr<ma_device> mDevice;

		/** A pointer to the Engine */
		std::unique_ptr<ma_engine> mEngine;

	public:		// Functions
		/** Creates a new AudioEngine and initializes the devices and context.
		 * The audio device will be the default one.
		 *
		 * @param	deviceId the sound device to use @see getDevices
		 * @throw	runtime_error in case of not being able to initialize the
		 *			devices */
		AudioEngine(std::size_t deviceId);

		/** Class destructor. It releases the audio devices. */
		~AudioEngine();

		/** @return	all the sound devices that can be used */
		static std::vector<DeviceInfo> getDevices();

		/** @return	the 3D position of the current Listener */
		glm::vec3 getListenerPosition() const;

		/** Sets the 3D position of the current Listener
		 *
		 * @param	position the new position of the Listener
		 * @return	a reference to the current AudioEngine object */
		AudioEngine& setListenerPosition(const glm::vec3& position);

		/** Returns the 3D orientation of the current Listener
		 *
		 * @param	forwardVector a reference to the vector that points to
		 *			the new forward direction of the current Listener
		 * @param	upVector a reference to the vector that points to the new
		 *			up direction of the current Listener */
		void getListenerOrientation(
			glm::vec3& forwardVector, glm::vec3& upVector
		) const;

		/** Sets the 3D orientation of the current Listener
		 *
		 * @param	forwardVector the vector that points to the new forward
		 *			direction of the current Listener
		 * @param	upVector the vector that points to the new up direction of
		 *			the current Listener
		 * @return	a reference to the current AudioEngine object */
		AudioEngine& setListenerOrientation(
			const glm::vec3& forwardVector, const glm::vec3& upVector
		);

		/** Returns the directional attenuation of the Listener
		 *
		 * @param	innerAngle a reference to the float where the angle in
		 *			radians where a sound inside that cone will have no
		 *			attenuation will be stored
		 * @param	outerAngle a reference to the float where the angle in
		 *			radians where a sound outside that cone will have an
		 *			attenuation set to @see outerGain will be stored
		 * @param	outerGain a reference to the float where the gain used
		 *			for the @see outerAngle cone will be stored */
		void getListenerCone(
			float& innerAngle, float& outerAngle, float& outerGain
		) const;

		/** Sets the directional attenuation of the Listener
		 *
		 * @param	innerAngle the angle in radians where a sound inside that
		 *			cone will have no attenuation
		 * @param	outerAngle the angle in radians where a sound outside that
		 *			cone will have an attenuation set to @see outerGain. The
		 *			sounds inside the outerAngle cone and outside
		 *			@see innerAngle cone will be interpolated between 1 and
		 *			@see outerGain
		 * @param	outerGain the gain used for the @see outerAngle cone
		 * @return	a reference to the current AudioEngine object */
		AudioEngine& setListenerCone(
			float innerAngle, float outerAngle, float outerGain
		);

		/** @return	the 3D velocity of the current Listener */
		glm::vec3 getListenerVelocity() const;

		/** Sets the 3D velocity of the current Listener
		 *
		 * @param	velocity the new velocity of the Listener
		 * @return	a reference to the current AudioEngine object */
		AudioEngine& setListenerVelocity(const glm::vec3& velocity);
	private:
		/** @return	the ResourceManager that stores the data */
		static ResourceManager& getResourceManagerInstance();

		/** The data callback used by the device
		 *
		 * @param	device a pointer to the device using the callback
		 * @param	output a pointer to the output data
		 * @param	input a pointer to the input data
		 * @param	frameCount the limit of frames to process */
		static void deviceDataCallback(
			ma_device* device, void* output, const void* input,
			unsigned int frameCount
		);
	};

}

#endif		// AUDIO_ENGINE_H
