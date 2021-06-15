#include "se/app/Repository.h"

namespace se::app {

	std::size_t Repository::sRepoTableTypeCount = 0;


	Repository::~Repository()
	{
		for (auto it = mRepoTables.rbegin(); it != mRepoTables.rend(); ++it) {
			if (*it) {
				it->reset();
			}
		}
	}

}
