#ifndef TAG_COMPONENT_H
#define TAG_COMPONENT_H

#include <array>
#include <string_view>

namespace se::app {

	/**
	 * Class TagComponent, it's a Component that holds the name tag of an
	 * Entity
	 */
	class TagComponent
	{
	private:	// Attributes
		/** The name of the Tag */
		std::array<char, 256> mName;

	public:		// Functions
		/** Creates a new TagComponent
		 *
		 * @param	name the name of the new TagComponent */
		TagComponent(std::string_view name = "")
		{
			std::size_t count = name.size();
			if (count > 255) {
				count = 255;
			}
			name.copy(mName.data(), count);
		};

		/** @return	the name of the Tag */
		const char* getName() const { return mName.data(); };
	};

}

#endif		// TAG_COMPONENT_H
