#pragma once

#include <array>
#include <string>
#include <iosfwd>
#include <cstring>

template <std::size_t Size, class T = char>
class ShortString
{
public:		// Nested types
	using size_type					= std::size_t;
	using difference_type			= std::ptrdiff_t;
	using value_type				= T;
	using pointer					= T*;
	using reference					= T&;
	using iterator					= T*;
	using const_iterator			= const T*;
	using reverse_iterator			= std::reverse_iterator<iterator>;
	using const_reverse_iterator	= std::reverse_iterator<const_iterator>;

private:	// Attributes
	std::array<T, Size> mBuffer;

public:		// Functions
	template <size_type N>
	ShortString(const T(&str)[N]) : mBuffer(str) {};

	template <class Traits, class Allocator>
	ShortString(const std::basic_string<T, Traits, Allocator>& str) noexcept
	{
		auto size = str.size() >= Size ? Size : str.size();
		std::memcpy(_buffer, str.c_str(), size);
		set_size(size);
	}

	template <size_type N>
	constexpr ShortString(const ShortString<N>& rhs) noexcept
		: _buffer{}
	{
		auto size = N >= Size ? Size : N;
		std::memcpy(_buffer, rhs.c_str(), size);
		set_size(size);
	}


	template <size_type N>
	constexpr ShortString& operator = (const T(&str)[N]) noexcept {
		auto size = N >= Size ? Size - 1 : N - 1;
		std::memcpy(_buffer, str, size);
		set_size(size);
		return *this;
	}


	template <class Traits, class Allocator>
	ShortString& operator = (const std::basic_string<T, Traits, Allocator>& str) noexcept {
		auto size = str.size() >= Size ? Size : str.size();
		std::memcpy(_buffer, str.c_str(), size);
		set_size(size);
		return *this;
	}


	template <size_type N>
	constexpr ShortString& operator = (const ShortString<N>& rhs) noexcept {
		if ((void*)this != (void*)&rhs) {
			auto size = N >= Size ? Size : N;
			std::memcpy(_buffer, rhs.c_str(), size);
			set_size(size);
		}
		return *this;
	}


	template <size_type N>
	ShortString(ShortString<N>&& rhs) {
		auto size = N >= Size ? Size: N;
		std::memcpy(_buffer, rhs.c_str(), size);
		set_size(size);
	}


	template <size_type N>
	ShortString& operator = (ShortString<N>&& rhs) {
		if ((void*)this != (void*)&rhs) {
			auto size = N >= Size ? Size : N;
			std::memcpy(_buffer, rhs.c_str(), size);
			set_size(size);
		}
		return *this;
	}


	inline void append(value_type value) noexcept {
		if (size() < capacity()) {
			_buffer[size()] = value;
			set_size(size() + 1);
		}
	}


	const value_type* c_str() const { return mBuffer.data(); }

	std::string to_string() const { return {reinterpret_cast<const char*>(_buffer), size()}; }


	size_type size() const { return capacity() - _buffer[capacity()]; }
	size_type capacity() const { return Size - 1; }


	template <size_type N>
	constexpr bool compare(const ShortString<N>& rhs) const noexcept {
		return compare(_buffer, rhs.c_str());
	}


	const_iterator begin() const { return mBuffer.data(); }

	const_iterator end() const { return mBuffer.data() + size(); }


	template <std::size_t SizeB>
	ShortString<Size, T>& operator += (const ShortString<SizeB, T>& rhs) noexcept {
		for (const auto c : rhs) {
			append(c);
		}
		return *this;
	}


	template <std::size_t N>
	ShortString<Size, T>& operator += (const T(&str)[N]) noexcept {
		for (const value_type* p = str; *p; ++p) {
			append(*p);
		}
		return *this;
	}


	template <class Traits, class Allocator>
	ShortString<Size, T>& operator += (const std::basic_string<T, Traits, Allocator>& rhs) noexcept {
		for (const auto c : rhs) {
			append(c);
		}
		return *this;
	}


private:
	inline void set_size(size_type size) noexcept { _buffer[capacity()] = Size - 1 - size; }


	constexpr bool compare(const value_type* lhs, const value_type* rhs) const noexcept {
		return (*lhs && *rhs) ? (*lhs == *rhs && compare(lhs + 1, rhs + 1)) : (!*lhs && !*rhs);
	}


	value_type _buffer[Size];
};


template <std::size_t SizeA, std::size_t SizeB, class T>
ShortString<SizeA, T> operator + (ShortString<SizeA, T> a, const ShortString<SizeB, T>& b) noexcept {
	a += b;
	return a;
}


template <std::size_t Size, class CharT, class Traits, class Allocator>
ShortString<Size, CharT> operator + (ShortString<Size, CharT> ss, const std::basic_string<CharT, Traits, Allocator>& str) noexcept {
	ss += str;
	return ss;
}


template <std::size_t Size, class T, std::size_t N>
ShortString<Size, T> operator + (ShortString<Size, T> ss, const T(&str)[N]) noexcept {
	ss += str;
	return ss;
}


template <std::size_t Size, class T, std::size_t N>
ShortString<Size, T> operator + (const T(&str)[N], ShortString<Size, T> ss) noexcept {
	ss += str;
	return ss;
}


template <std::size_t SizeA, std::size_t SizeB, class T>
constexpr bool operator == (const ShortString<SizeA, T>& a, const ShortString<SizeB, T>& b) noexcept {
	return a.compare(b);
}


template <std::size_t Size, class CharT, class Traits, class Allocator>
bool operator == (const ShortString<Size, CharT>& lhs, const std::basic_string<CharT, Traits, Allocator>& rhs) noexcept {
	return std::strncmp(lhs.c_str(), rhs.c_str(), Size) == 0;
}


template <class CharT, class Traits, class Allocator, std::size_t Size>
bool operator == (const std::basic_string<CharT, Traits, Allocator>& lhs, const ShortString<Size, CharT>& rhs) noexcept {
	return rhs == lhs;
}


template <std::size_t Size, class T>
std::ostream& operator << (std::ostream& os, const ShortString<Size, T>& s) {
	return os << s.c_str();
}