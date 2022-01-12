#include "se/graphics/core/UniformBlock.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/GLWrapper.h"

namespace se::graphics {

	bool UniformBlock::load(const Program& program)
	{
		mProgramId = program.mProgramId;
		GL_WRAP( mUniformLocation = glGetUniformBlockIndex(mProgramId, mName.c_str()) );

		if (mUniformLocation == -1) {
			SOMBRA_WARN_LOG << "Uniform block \"" << mName << "\" wasn't found in Program " << mProgramId;
			return false;
		}

		return true;
	}


	void UniformBlock::bind() const
	{
		GL_WRAP( glUniformBlockBinding(mProgramId, mSlot, mUniformLocation) );
	}


	void UniformBlock::unbind() const
	{
		GL_WRAP( glUniformBlockBinding(mProgramId, 0, mUniformLocation) );
	}

}
