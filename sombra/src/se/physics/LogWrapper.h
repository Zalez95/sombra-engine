#ifndef LOG_WRAPPER_H
#define LOG_WRAPPER_H

#include <ostream>
#include "se/utils/StringUtils.h"
#include "se/physics/RigidBodyWorld.h"

namespace se::physics {

	/** The different log levels */
	enum class LogLevel : int { Error = 0, Warning, Info, Debug };


	/**
	 * Class LogStream, it's used to write logs with the LogHandler in a stream
	 * like fashion
	 */
	template <typename CharT, std::streamsize Size, LogLevel Level>
	class LogStream : public std::basic_ostream<CharT>
	{
	private:	// Attributes
		/** The buffer where the traces will be stored */
		utils::ArrayStreambuf<CharT, Size> mASBuf;

		/** The RigidBodyWorld used for tracing */
		const RigidBodyWorld& mWorld;

	public:		// Functions
		/** Creates a new LogStream
		 *
		 * @param	world the RigidBodyWorld to use for tracing
		 * @param	level the level of the traces */
		LogStream(const RigidBodyWorld& world) :
			std::basic_ostream<CharT>(&mASBuf), mWorld(world) {};

		/** Class destructor */
		~LogStream()
		{
			if constexpr (Level == LogLevel::Error) {
				mWorld.getProperties().logHandler->error(mASBuf.data());
			}
			else if constexpr (Level == LogLevel::Warning) {
				mWorld.getProperties().logHandler->warning(mASBuf.data());
			}
			else if constexpr (Level == LogLevel::Info) {
				mWorld.getProperties().logHandler->info(mASBuf.data());
			}
			else {
				mWorld.getProperties().logHandler->debug(mASBuf.data());
			}
		};
	};

}

#define FORMAT_LOCATION(function, line) function << "(" << line << "): "
#define LOCATION FORMAT_LOCATION(__func__, __LINE__)

#define SPHYS_ERROR_LOG(world)	\
	LogStream<char, 256, LogLevel::Error>(world) << LOCATION
#define SPHYS_WARN_LOG(world)	\
	LogStream<char, 256, LogLevel::Warning>(world) << LOCATION
#define SPHYS_INFO_LOG(world)	\
	LogStream<char, 256, LogLevel::Info>(world) << LOCATION
#define SPHYS_DEBUG_LOG(world)	\
	LogStream<char, 256, LogLevel::Debug>(world) << LOCATION

#endif		// LOG_WRAPPER_H
