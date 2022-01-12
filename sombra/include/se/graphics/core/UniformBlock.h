#ifndef UNIFORM_BLOCK_H
#define UNIFORM_BLOCK_H

#include <string>
#include "Bindable.h"

namespace se::graphics {

	class Program;


	/**
	 * Class UniformBlock, it's used for accessing to the data of a Uniform
	 * Buffer Object from the Shader source code
	 */
	class UniformBlock : public Bindable
	{
	private:	// Attributes
		/** The name of the UniformBlock */
		std::string mName;

		/** The location of the UniformBlock in the Shader source code */
		int mUniformLocation;

		/** The program id of @see mProgram */
		unsigned int mProgramId;

		/** The binding point where a Uniform Buffer Object is bound */
		unsigned int mSlot;

	public:		// Functions
		/** Creates a new UniformBlock
		 *
		 * @param	name the name of the UniformBlock */
		UniformBlock(const std::string& name) :
			mName(name), mUniformLocation(-1), mProgramId(0), mSlot(0) {};

		/** @return	the name of the IUniformVariable */
		const std::string& getName() const { return mName; };

		/** @return	the binding point of the UniformBlock */
		unsigned int getBindingPoint() const { return mSlot; };

		/** Sets the binding point of the UniformBlock
		 *
		 * @param	slot the binding point where a Uniform Buffer Object will
		 *			be bound */
		void setBindingPoint(unsigned int slot) { mSlot = slot; };

		/** Loads the UniformBlock
		 *
		 * @param	program a reference to the Program used for loading
		 * @return	true if the UniformBlock was found inside the Program,
		 *			false otherwise */
		bool load(const Program& program);

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override
		{ return std::make_unique<UniformBlock>(*this); };

		/** Binds the UniformBlock */
		virtual void bind() const override;

		/** Unbinds the UniformBlock */
		virtual void unbind() const override;
	};

}

#endif		// UNIFORM_BLOCK_H
