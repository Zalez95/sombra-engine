#include <stdexcept>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include "se/utils/Log.h"
#include "se/audio/AudioEngine.h"

namespace se::audio {

	struct AudioEngine::ResourceManager
	{
		ma_resource_manager resourceManager;

		ResourceManager()
		{
			ma_resource_manager_config resourceManagerConfig = ma_resource_manager_config_init();
			resourceManagerConfig.decodedFormat = ma_format_f32;
			resourceManagerConfig.decodedChannels = 0;				// native channel count
			resourceManagerConfig.decodedSampleRate = 48000;

			ma_result result = ma_resource_manager_init(&resourceManagerConfig, &resourceManager);
			if (result != MA_SUCCESS) {
				throw std::runtime_error("Resource Manager creation error");
			}
		};

		~ResourceManager()
		{
			ma_resource_manager_uninit(&resourceManager);
		};
	};


	AudioEngine::AudioEngine(std::size_t deviceId) :
		mContext(std::make_unique<ma_context>()), mDevice(std::make_unique<ma_device>()),
		mEngine(std::make_unique<ma_engine>())
	{
		ma_result result = ma_context_init(nullptr, 0, nullptr, mContext.get());
		if (result != MA_SUCCESS) {
			throw std::runtime_error("Context creation error");
		}

		ma_device_info* deviceInfos;
		ma_uint32 deviceCount;
		result = ma_context_get_devices(mContext.get(), &deviceInfos, &deviceCount, nullptr, nullptr);
		if (result != MA_SUCCESS) {
			throw std::runtime_error("Failed to retrieve the devices");
		}

		ma_device_config deviceConfig;
		deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.pDeviceID = &deviceInfos[deviceId].id;
		deviceConfig.playback.format = getResourceManagerInstance().resourceManager.config.decodedFormat;
		deviceConfig.playback.channels = 0;
		deviceConfig.sampleRate = getResourceManagerInstance().resourceManager.config.decodedSampleRate;
		deviceConfig.dataCallback = deviceDataCallback;
		deviceConfig.pUserData = this;

		result = ma_device_init(mContext.get(), &deviceConfig, mDevice.get());
		if (result != MA_SUCCESS) {
			throw std::runtime_error(std::string("Failed to initialize device ") + deviceInfos[0].name);
		}

		ma_engine_config engineConfig = ma_engine_config_init();
		engineConfig.pDevice = mDevice.get();
		engineConfig.pResourceManager = &getResourceManagerInstance().resourceManager;
		engineConfig.listenerCount = 1;

		result = ma_engine_init(&engineConfig, mEngine.get());
		if (result != MA_SUCCESS) {
			throw std::runtime_error("Engine creation error");
		}
	}


	AudioEngine::~AudioEngine()
	{
		ma_engine_uninit(mEngine.get());
		ma_device_uninit(mDevice.get());
		ma_context_uninit(mContext.get());
	}


	std::vector<AudioEngine::DeviceInfo> AudioEngine::getDevices()
	{
		ma_context context;
		ma_result result = ma_context_init(nullptr, 0, nullptr, &context);
		if (result != MA_SUCCESS) {
			throw std::runtime_error("Context creation error");
		}

		ma_device_info* deviceInfos;
		ma_uint32 deviceCount;
		result = ma_context_get_devices(&context, &deviceInfos, &deviceCount, nullptr, nullptr);
		if (result != MA_SUCCESS) {
			throw std::runtime_error("Failed to retrieve the devices");
		}

		std::vector<DeviceInfo> ret;
		for (ma_uint32 i = 0; i < deviceCount; ++i) {
			ret.push_back({ deviceInfos[i].name, i });
		}

		ma_context_uninit(&context);

		return ret;
	}


	glm::vec3 AudioEngine::getListenerPosition() const
	{
		ma_vec3f pos = ma_engine_listener_get_position(mEngine.get(), kListenerIndex);
		return { pos.x, pos.y, pos.z };
	}


	AudioEngine& AudioEngine::setListenerPosition(const glm::vec3& position)
	{
		ma_engine_listener_set_position(mEngine.get(), kListenerIndex, position.x, position.y, position.z);
		return *this;
	}


	void AudioEngine::getListenerOrientation(glm::vec3& forwardVector, glm::vec3& upVector) const
	{
		ma_vec3f dirFor = ma_engine_listener_get_direction(mEngine.get(), kListenerIndex);
		forwardVector = { dirFor.x, dirFor.y, dirFor.z };

		ma_vec3f dirUp = ma_engine_listener_get_world_up(mEngine.get(), kListenerIndex);
		upVector = { dirUp.x, dirUp.y, dirUp.z };
	}


	AudioEngine& AudioEngine::setListenerOrientation(const glm::vec3& forwardVector, const glm::vec3& upVector)
	{
		ma_engine_listener_set_direction(mEngine.get(), kListenerIndex, forwardVector.x, forwardVector.y, forwardVector.z);
		ma_engine_listener_set_world_up(mEngine.get(), kListenerIndex, upVector.x, upVector.y, upVector.z);
		return *this;
	}


	void AudioEngine::getListenerCone(float& innerAngle, float& outerAngle, float& outerGain) const
	{
		ma_engine_listener_get_cone(mEngine.get(), kListenerIndex, &innerAngle, &outerAngle, &outerGain);
	}


	AudioEngine& AudioEngine::setListenerCone(float innerAngle, float outerAngle, float outerGain)
	{
		ma_engine_listener_set_cone(mEngine.get(), kListenerIndex, innerAngle, outerAngle, outerGain);
		return *this;
	}


	glm::vec3 AudioEngine::getListenerVelocity() const
	{
		ma_vec3f vel = ma_engine_listener_get_velocity(mEngine.get(), kListenerIndex);
		return { vel.x, vel.y, vel.z };
	}


	AudioEngine& AudioEngine::setListenerVelocity(const glm::vec3& velocity)
	{
		ma_engine_listener_set_velocity(mEngine.get(), kListenerIndex, velocity.x, velocity.y, velocity.z);
		return *this;
	}

// Private functions
	AudioEngine::ResourceManager& AudioEngine::getResourceManagerInstance()
	{
		static ResourceManager resourceManager;
		return resourceManager;
	}


	void AudioEngine::deviceDataCallback(
		ma_device* device, void* output, const void*,
		unsigned int frameCount
	) {
		AudioEngine* audioEngine = static_cast<AudioEngine*>(device->pUserData);
		ma_engine_read_pcm_frames(audioEngine->mEngine.get(), output, frameCount, nullptr);
	}

}
