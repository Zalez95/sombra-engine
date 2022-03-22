#include <miniaudio.h>
#include "se/utils/Log.h"
#include "se/audio/DataSource.h"
#include "se/audio/AudioEngine.h"

namespace se::audio {

	DataSource::DataSource() {}


	DataSource::DataSource(const DataSource& other)
	{
		operator=(other);
	}


	DataSource::DataSource(DataSource&& other) :
		mDataSourceOwner(std::move(other.mDataSourceOwner)) {}


	DataSource::~DataSource()
	{
		if (mDataSourceOwner) {
			ma_sound_uninit(mDataSourceOwner.get());
			SOMBRA_TRACE_LOG << "Deleted DataSourceOwner " << mDataSourceOwner.get();
			mDataSourceOwner = nullptr;
		}
	}


	DataSource& DataSource::operator=(const DataSource& other)
	{
		if (mDataSourceOwner) {
			ma_sound_uninit(mDataSourceOwner.get());
			SOMBRA_TRACE_LOG << "Deleted DataSourceOwner " << mDataSourceOwner.get();
			mDataSourceOwner = nullptr;
		}

		mDataSourceOwner = std::make_unique<ma_sound>();
		ma_engine* engine = ma_sound_get_engine(other.mDataSourceOwner.get());
		ma_result res = ma_sound_init_copy(engine, other.mDataSourceOwner.get(), 0, nullptr, mDataSourceOwner.get());
		if (res != MA_SUCCESS) {
			SOMBRA_ERROR_LOG << "Failed to create the DataSourceOwner";
			mDataSourceOwner = nullptr;
		}

		SOMBRA_TRACE_LOG << "Created DataSourceOwner " << mDataSourceOwner.get();

		return *this;
	}


	DataSource& DataSource::operator=(DataSource&& other)
	{
		if (mDataSourceOwner) {
			ma_sound_uninit(mDataSourceOwner.get());
			SOMBRA_TRACE_LOG << "Deleted DataSourceOwner " << mDataSourceOwner.get();
			mDataSourceOwner = nullptr;
		}

		mDataSourceOwner = std::move(other.mDataSourceOwner);

		return *this;
	}


	DataSource DataSource::createFromFile(AudioEngine& engine, const char* path)
	{
		DataSource ret;
		ret.mDataSourceOwner = std::make_unique<ma_sound>();
		ma_result res = ma_sound_init_from_file(engine.mEngine.get(), path, MA_SOUND_FLAG_DECODE, nullptr, nullptr, ret.mDataSourceOwner.get());
		if (res != MA_SUCCESS) {
			SOMBRA_ERROR_LOG << "Failed to create the DataSourceOwner";
			ret.mDataSourceOwner = nullptr;
			return ret;
		}

		SOMBRA_TRACE_LOG << "Created DataSourceOwner " << ret.mDataSourceOwner.get();

		return ret;
	}

}
