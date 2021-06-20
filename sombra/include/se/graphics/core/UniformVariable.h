#ifndef UNIFORM_VARIABLE_H
#define UNIFORM_VARIABLE_H

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "Bindable.h"

namespace se::graphics {

	class Program;


	/**
	 * Class IUniformVariable, it's the class used for managing the uniform
	 * variables used in the shaders
	 */
	class IUniformVariable : public Bindable
	{
	protected:	// Attributes
		/** The name of the IUniformVariable */
		std::string mName;

		/** The program of the IUniformVariable */
		std::shared_ptr<Program> mProgram;

		/** The location of the variable in the Shader source code */
		int mUniformLocation;

	public:		// Functions
		/** Creates a new IUniformVariable
		 *
		 * @param	name the name of the IUniformVariable
		 * @param	program the Program used for retrieving the Uniform
		 *			Location of the IUniformVariable */
		IUniformVariable(
			const char* name, const std::shared_ptr<Program>& program
		);

		/** Class destructor */
		virtual ~IUniformVariable() = default;

		/** @return	the name of the IUniformVariable */
		const std::string& getName() const { return mName; };

		/** @return	the Program of the IUniformVariable */
		const std::shared_ptr<Program>& getProgram() const { return mProgram; };

		/** @return	true if the IUniformVariable was found inside the Program,
		 *			false otherwise */
		bool found() const;
	};


	/**
	 * Class UniformVariable, it's the templated class used for managing the
	 * uniform variables of type @tparam T used in the shaders
	 */
	template <typename T>
	class UniformVariable : public IUniformVariable
	{
	public:		// Functions
		/** Creates a new UniformVariable
		 *
		 * @param	name the name of the UniformVariable
		 * @param	program the Program used for retrieving the Uniform
		 *			Location of the UniformVariable */
		UniformVariable(
			const char* name, const std::shared_ptr<Program>& program
		) : IUniformVariable(name, program) {};

		/** Class destructor */
		virtual ~UniformVariable() = default;
	protected:
		/** Sets the value of the given uniform variable
		 *
		 * @param	value the new value of the variable */
		void setUniform(const T& value) const;

		/** Sets the values of the given array of values uniform variable
		 *
		 * @param	valuePtr a pointer to the vector of values
		 * @param	count the number of elements to set */
		void setUniformV(const T* valuePtr, std::size_t count) const;
	};


	/**
	 * Class UniformVariableValue, it's a UniformVariable that holds the value
	 * to set on bind.
	 */
	template <typename T>
	class UniformVariableValue : public UniformVariable<T>
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
			const char* name, const std::shared_ptr<Program>& program,
			const T& value = T()
		) : UniformVariable<T>(name, program), mValue(value) {};

		/** Sets the value of the UniformVariableValue
		 *
		 * @param	value the new Value
		 * @return	the current UniformVariableValue object */
		UniformVariableValue& setValue(const T& value)
		{
			mValue = value;
			return *this;
		};

		/** @return	the value stored in the UniformVariableValue */
		const T& getValue() const { return mValue; };

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override
		{ return std::make_unique<UniformVariableValue>(*this); };

		/** Binds the current UniformVariableValue for using it in the following
		 * operations */
		virtual void bind() const override { this->setUniform(mValue); };

		/** Unbinds the current UniformVariableValue so it can't be used in the
		 * following operations */
		virtual void unbind() const override {};
	};


	/**
	 * Class UniformVariableValueVector, it's a UniformVariable that holds a
	 * vector of values to set on bind.
	 */
	template <typename T>
	class UniformVariableValueVector : public UniformVariable<T>
	{
	private:	// Attributes
		/** The vector with all the values to bind of the UniformVariable */
		std::vector<T> mValue;

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
			const char* name, const std::shared_ptr<Program>& program,
			const T* valuePtr = nullptr, std::size_t count = 0
		) : UniformVariable<T>(name, program) { setValue(valuePtr, count); };

		/** Sets the value of the UniformVariableValueVector
		 *
		 * @param	valuePtr a pointer to the first element of the vector of
		 *			values
		 * @param	count the number of elements in valuePtr
		 * @return	the current UniformVariableValueVector object */
		UniformVariableValueVector& setValue(
			const T* valuePtr, std::size_t count
		) {
			mValue.clear();
			mValue.insert(mValue.end(), valuePtr, valuePtr + count);
			return *this;
		};

		/** Function used for retrieving the values stored in the
		 * UniformVariableValueVector
		 *
		 * @param	retValuePtr a pointer to the first element of the vector of
		 *			values (return parameter)
		 * @param	retCount the number of elements in retValuePtr
		 *			(return parameter) */
		void getValue(const T*& retValuePtr, std::size_t& retCount) const
		{ retValuePtr = mValue.data(); retCount = mValue.size(); };

		/** @return	the number of variables that the UniformVariableValueVector
		 *			holds */
		std::size_t size() const { return mValue.size(); };

		/** @return	the maximum number of variables that the
		 *			UniformVariableValueVector can hold */
		std::size_t capacity() const { return mValue.capacity(); };

		/** Reserves the given number of variables
		 *
		 * @param	capacity the number of variables to reserve */
		void reserve(std::size_t capacity) { mValue.reserve(capacity); };

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override
		{ return std::make_unique<UniformVariableValueVector>(*this); };

		/** Binds the current UniformVariableValueVector for using it in the
		 * following operations */
		virtual void bind() const override
		{ this->setUniformV(mValue.data(), mValue.size()); };

		/** Unbinds the current UniformVariableValueVector so it can't be used
		 * in the following operations */
		virtual void unbind() const override {};
	};


	/**
	 * Class UniformVariableCallback, it's a UniformVariable that holds a
	 * callback function to retrieve the value to set on bind
	 */
	template <typename T, bool isVector = false>
	class UniformVariableCallback : public UniformVariable<T>
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
		 * @param	name the name of the UniformVariableCallback
		 * @param	program the Program used for retrieving the Uniform
		 *			Location of the UniformVariableCallback
		 * @param	callback the callback function used for retrieving the
		 *			value of the UniformVariableCallback to bind */
		UniformVariableCallback(
			const char* name, const std::shared_ptr<Program>& program,
			const Callback& callback
		) : UniformVariable<T>(name, program), mCallback(callback) {};

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override
		{ return std::make_unique<UniformVariableCallback>(*this); };

		/** Binds the current UniformVariableCallback for using it in the
		 * following operations */
		virtual void bind() const override
		{
			if constexpr (isVector) {
				const auto& [valuePtr, count] = mCallback();
				this->setUniformV(valuePtr, count);
			}
			else {
				this->setUniform(mCallback());
			}
		};

		/** Unbinds the current IUniformUniformVariableCallbackVariable so it
		 * can't be used in the following operations */
		virtual void unbind() const override {};
	};

}

#endif		// UNIFORM_VARIABLE_H
