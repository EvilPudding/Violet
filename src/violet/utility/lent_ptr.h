#ifndef VIOLET_LentPtr_H
#define VIOLET_LentPtr_H

#include <memory>

namespace vlt
{
	template <typename T>
	struct Lent_Ptr
	{
		Lent_Ptr(T * ptr);
		Lent_Ptr(const std::unique_ptr<T> & ptr);

		T & operator*() const;
		T * operator->() const;

		bool operator==(const T * rhs) const;
		bool operator!=(const T * rhs) const;

	private:

		T * m_ptr;
	};
}

#include "violet/utility/lent_ptr.inl"

#endif
