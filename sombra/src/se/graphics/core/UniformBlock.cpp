#include "se/graphics/core/UniformBlock.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/GLWrapper.h"

namespace se::graphics {

	UniformBlock::UniformBlock(const char* name, std::shared_ptr<Program> program) :
		mName(name), mProgram(program), mUniformLocation(-1), mSlot(0)
	{
		GL_WRAP( mUniformLocation = glGetUniformBlockIndex(mProgram->mProgramId, name) );

		if (mUniformLocation == -1) {
			SOMBRA_WARN_LOG << "Uniform block \"" << name << "\" wasn't found";
		}
	}


	bool UniformBlock::found() const
	{
		return mUniformLocation >= 0;
	}


	std::unique_ptr<Bindable> UniformBlock::clone() const
	{
		auto ret = std::make_unique<UniformBlock>(mName.c_str(), mProgram);
		ret->mSlot = mSlot;
		return ret;
	}


	void UniformBlock::bind() const
	{
		GL_WRAP( glUniformBlockBinding(mProgram->mProgramId, mSlot, mUniformLocation) );
	}


	void UniformBlock::unbind() const
	{
		GL_WRAP( glUniformBlockBinding(mProgram->mProgramId, 0, mUniformLocation) );
	}

}
