#ifndef UNIFORM_VARIABLE_H
#define UNIFORM_VARIABLE_H

#include <functional>
#include "../../utils/FixedVector.h"
#include "Bindable.h"

namespace se::graphics {

	class Program;


	/**
	 * Class IUniformVariable, it's the class used for managing the uniform
	 * variables used in the shaders
	 */
	class IUniformVariable : public Bindable
	{
	private:	// Attributes
		/** The location of the variable in the Shader source code */
		int mUniformLocation;

	public:		// Functions
		/** Creates a new IUniformVariable
		 *
		 * @param	name the name of the IUniformVariable
		 * @param	program the Program used for retrieving the Uniform
		 *			Location of the IUniformVariable */
		IUniformVariable(const char* name, const Program& program);

		/** Class destructor */
		virtual ~IUniformVariable() = default;

		/** @return	true if the IUniformVariable was found inside the Program,
		 *			false otherwise */
		bool found() const;
	protected:
		/** Sets the value of the given uniform variable
		 *
		 * @param	value the new value of the variable */
		template <typename T>
		void setUniform(const T& value) const;

		/** Sets the values of the given array of values uniform variable
		 *
		 * @param	valuePtr a pointer to the vector of values
		 * @param	count the number of elements to set */
		template <typename T>
		void setUniformV(const T* valuePtr, std::size_t count) const;
	};


	/**
	 * Class UniformVariableValue, it's a IUniformVariable that holds the value
	 * to set on bind.
	 */
	template <typename T>
	class UniformVariableValue : public IUniformVariable
	{
	private:	// Attributes
		/** The value to bind of the UniformVariable */
		T mValue;

	public:		// Functions
		/** Creates a new UniformVariableValue
		 *
		 * @param	name the name of the UniformVariableValue
		 * @param	program the Program used for retrieving the Uniform
		 *			Location of the UniformVariableValue
		 * @param	value the value of the UniformVariableValue to bind */
		UniformVariableValue(
			const char* name, const Program& program, const T& value = T()
		) : IUniformVariable(name, program), mValue(value) {};

		/** Sets the value of the UniformVariableValue
		 *
		 * @param	value the new Value */
		void setValue(const T& value) { mValue = value; }

		/** Binds the current UniformVariableValue for using it in the following
		 * operations */
		virtual void bind() const override { setUniform(mValue); };

		/** Unbinds the current UniformVariableValue so it can't be used in the
		 * following operations */
		virtual void unbind() const override {};
	};


	/**
	 * Class UniformVariableValueVector, it's a IUniformVariable that holds a
	 * vector of values to set on bind.
	 * @tparam VectorSize is the maximum number of elements that can be set
	 */
	template <typename T, std::size_t VectorSize>
	class UniformVariableValueVector : public IUniformVariable
	{
	private:	// Nested types
		using Vector = utils::FixedVector<T, VectorSize>;

	private:	// Attributes
		/** The vector with all the values to bind of the UniformVariable */
		Vector mValue;

	public:		// Functions
		/** Creates a new UniformVariableValueVector
		 *
		 * @param	name the name of the UniformVariableValueVector
		 * @param	program the Program used for retrieving the Uniform
		 *			Location of the UniformVariableValueVector
		 * @param	valuePtr a pointer to the first element of the vector of
		 *			values
		 * @param	count the number of elements in valuePtr */
		UniformVariableValueVector(
			const char* name, const Program& program,
			const T* valuePtr = nullptr, std::size_t count = 0
		) : IUniformVariable(name, program)
		{ setValue(valuePtr, count); };

		/** Sets the value of the UniformVariableValueVector
		 *
		 * @param	valuePtr a pointer to the first element of the vector of
		 *			values
		 * @param	count the number of elements in valuePtr */
		void setValue(const T* valuePtr, std::size_t count)
		{ mValue = Vector(valuePtr, valuePtr + count); }

		/** Binds the current UniformVariableValueVector for using it in the
		 * following operations */
		virtual void bind() const override
		{ setUniformV(mValue.data(), mValue.size()); };

		/** Unbinds the current UniformVariableValueVector so it can't be used
		 * in the following operations */
		virtual void unbind() const override {};
	};


	/**
	 * Class UniformVariableCallback, it's a IUniformVariable that holds a
	 * callback function to retrieve the value to set on bind
	 */
	template <typename T, bool isVector = false>
	class UniformVariableCallback : public IUniformVariable
	{
	private:	// Nested types
		using Callback = std::conditional_t<isVector,
			std::function<std::pair<T*, std::size_t>()>,
			std::function<T()>
		>;

	private:	// Attributes
		/** The function that is going to be called for retrieving the value
		 * to bind of the UniformVariableCallback */
		Callback mCallback;

	public:		// Functions
		/** Creates a new UniformVariableCallback
		 *
		 * @param	name the name of the IUniformVariable
		 * @param	program the Program used for retrieving the Uniform
		 *			Location of the IUniformVariable
		 * @param	callback the callback function used for retrieving the
		 *			value of the IUniformVariable to bind */
		UniformVariableCallback(
			const char* name, const Program& program,
			const Callback& callback
		) : IUniformVariable(name, program), mCallback(callback) {};

		/** Binds the current IUniformVariable for using it in the following
		 * operations */
		virtual void bind() const override
		{
			if constexpr (isVector) {
				const auto& [valuePtr, count] = mCallback();
				setUniformV(valuePtr, count);
			}
			else {
				setUniform(mCallback());
			}
		};

		/** Unbinds the current IUniformVariable so it can't be used in the
		 * following operations */
		virtual void unbind() const override {};
	};

}

#endif		// UNIFORM_VARIABLE_H
