#include "se/graphics/IProgram.h"
#include "se/graphics/core/Program.h"
#include "se/utils/Log.h"

namespace se::graphics {

	bool IProgram::init()
	{
		if (createProgram()) {
			if (!addUniforms()) {
				SOMBRA_WARN_LOG << "Some uniforms are missing";
			}

			return true;
		}

		return false;
	}


	void IProgram::end()
	{
		if (mProgram) {
			delete mProgram;
		}
	}


	void IProgram::enable() const
	{
		mProgram->enable();
	}


	void IProgram::disable() const
	{
		mProgram->disable();
	}

}
