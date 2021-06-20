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

		/** The program that the UniformBlock belongs to */
		std::shared_ptr<Program> mProgram;

		/** The location of the UniformBlock in the Shader source code */
		int mUniformLocation;

		/** The binding point where a Uniform Buffer Object is bound */
		unsigned int mSlot;

	public:		// Functions
		/** Creates a new UniformBlock
		 *
		 * @param	name the name of the UniformBlock
		 * @param	program the Program used for retrieving the Uniform
		 *			Location of the UniformBlock */
		UniformBlock(const char* name, std::shared_ptr<Program> program);

		/** @return	the name of the IUniformVariable */
		const std::string& getName() const { return mName; };

		/** @return	the Program of the IUniformVariable */
		const std::shared_ptr<Program>& getProgram() const { return mProgram; };

		/** @return	the binding point of the UniformBlock */
		unsigned int getBindingPoint() const { return mSlot; };

		/** Sets the binding point of the UniformBlock
		 *
		 * @param	slot the binding point where a Uniform Buffer Object will
		 *			be bound */
		void setBindingPoint(unsigned int slot) { mSlot = slot; };

		/** @return	true if the IUniformVariable was found inside the Program,
		 *			false otherwise */
		bool found() const;

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override;

		/** Binds the UniformBlock */
		virtual void bind() const override;

		/** Unbinds the UniformBlock */
		virtual void unbind() const override;
	};

}

#endif		// UNIFORM_BLOCK_H
