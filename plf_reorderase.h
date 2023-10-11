// Copyright (c) 2023, Matthew Bentley (mattreecebentley@gmail.com) www.plflib.org

// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// 	claim that you wrote the original software. If you use this software
// 	in a product, an acknowledgement in the product documentation would be
// 	appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// 	misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.


#ifndef PLF_REORDERASE_H
#define PLF_REORDERASE_H


// Compiler-specific defines:

// Define default cases before possibly redefining:
#define PLF_NOEXCEPT throw()
#define PLF_CONSTEXPR
#define PLF_CONSTFUNC

#if !(((defined(__clang__) || defined(__GNUC__)) && !defined(__EXCEPTIONS)) || (defined(_MSC_VER) && !defined(_CPPUNWIND)))
	#define PLF_EXCEPTIONS_SUPPORT
#endif


#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
	 // Suppress incorrect (unfixed MSVC bug at warning level 4) warnings re: constant expressions in constexpr-if statements
	#pragma warning ( push )
	 #pragma warning ( disable : 4127 )

	#if _MSC_VER >= 1600
		#define PLF_MOVE_SEMANTICS_SUPPORT
	#endif
	#if _MSC_VER >= 1700
		#define PLF_TYPE_TRAITS_SUPPORT
		#define PLF_ALLOCATOR_TRAITS_SUPPORT
	#endif
	#if _MSC_VER >= 1900
		#define PLF_ALIGNMENT_SUPPORT
		#undef PLF_NOEXCEPT
		#define PLF_NOEXCEPT noexcept
	#endif

	#if defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L)
		#undef PLF_CONSTEXPR
		#define PLF_CONSTEXPR constexpr
	#endif

	#if defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L) && _MSC_VER >= 1929
		#define PLF_CPP20_SUPPORT
		#undef PLF_CONSTFUNC
		#define PLF_CONSTFUNC constexpr
	#endif
#elif defined(__cplusplus) && __cplusplus >= 201103L // C++11 support, at least
	#if defined(__GNUC__) && defined(__GNUC_MINOR__) && !defined(__clang__) // If compiler is GCC/G++
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 3) || __GNUC__ > 4
			#define PLF_MOVE_SEMANTICS_SUPPORT
		#endif
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
			#undef PLF_NOEXCEPT
			#define PLF_NOEXCEPT noexcept
		#endif
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 7) || __GNUC__ > 4
			#define PLF_ALLOCATOR_TRAITS_SUPPORT
		#endif
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 8) || __GNUC__ > 4
			#define PLF_ALIGNMENT_SUPPORT
		#endif
		#if __GNUC__ >= 5 // GCC v4.9 and below do not support std::is_trivially_copyable
			#define PLF_TYPE_TRAITS_SUPPORT
		#endif
	#elif defined(__clang__) && !defined(__GLIBCXX__) && !defined(_LIBCPP_CXX03_LANG) && __clang_major__ >= 3
		#define PLF_ALLOCATOR_TRAITS_SUPPORT
		#define PLF_TYPE_TRAITS_SUPPORT

		#if __has_feature(cxx_alignas) && __has_feature(cxx_alignof)
			#define PLF_ALIGNMENT_SUPPORT
		#endif
		#if __has_feature(cxx_noexcept)
			#undef PLF_NOEXCEPT
			#define PLF_NOEXCEPT noexcept
		#endif
		#if __has_feature(cxx_rvalue_references) && !defined(_LIBCPP_HAS_NO_RVALUE_REFERENCES)
			#define PLF_MOVE_SEMANTICS_SUPPORT
		#endif
	#elif defined(__GLIBCXX__) // Using another compiler type with libstdc++ - we are assuming full c++11 compliance for compiler - which may not be true
		#if __GLIBCXX__ >= 20080606
			#define PLF_MOVE_SEMANTICS_SUPPORT
		#endif
		#if __GLIBCXX__ >= 20120322
			#undef PLF_NOEXCEPT
			#define PLF_NOEXCEPT noexcept
		#endif
		#if __GLIBCXX__ >= 20130322
			#define PLF_ALIGNMENT_SUPPORT
		#endif
		#if __GLIBCXX__ >= 20150422 // libstdc++ v4.9 and below do not support std::is_trivially_copyable
			#define PLF_TYPE_TRAITS_SUPPORT
		#endif
	#elif !(defined(_LIBCPP_CXX03_LANG) || defined(_LIBCPP_HAS_NO_RVALUE_REFERENCES)) // Assume type traits and initializer support for other compilers and standard library implementations
		#define PLF_MOVE_SEMANTICS_SUPPORT
		#define PLF_TYPE_TRAITS_SUPPORT
		#define PLF_ALLOCATOR_TRAITS_SUPPORT
		#define PLF_ALIGNMENT_SUPPORT
		#undef PLF_NOEXCEPT
		#define PLF_NOEXCEPT noexcept
	#endif

	#if __cplusplus >= 201703L && ((defined(__clang__) && ((__clang_major__ == 3 && __clang_minor__ == 9) || __clang_major__ > 3)) || (defined(__GNUC__) && __GNUC__ >= 7) || (!defined(__clang__) && !defined(__GNUC__))) // assume correct C++17 implementation for non-gcc/clang compilers
		#undef PLF_CONSTEXPR
		#define PLF_CONSTEXPR constexpr
	#endif

	#if __cplusplus > 201704L && ((((defined(__clang__) && !defined(__APPLE_CC__) && __clang_major__ >= 14) || (defined(__GNUC__) && (__GNUC__ > 11 || (__GNUC__ == 11 && __GNUC_MINOR__ > 0)))) && ((defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 14) || (defined(__GLIBCXX__) && __GLIBCXX__ >= 201806L))) || (!defined(__clang__) && !defined(__GNUC__)))
		#define PLF_CPP20_SUPPORT
		#undef PLF_CONSTFUNC
		#define PLF_CONSTFUNC constexpr
	#endif
#endif


#ifdef PLF_ALLOCATOR_TRAITS_SUPPORT
	#define PLF_DESTROY(the_allocator, allocator_instance, location)			std::allocator_traits<the_allocator>::destroy(allocator_instance, location)
	#define PLF_ALLOCATE(the_allocator, allocator_instance, size, hint)			std::allocator_traits<the_allocator>::allocate(allocator_instance, size, hint)
	#define PLF_DEALLOCATE(the_allocator, allocator_instance, location, size)	std::allocator_traits<the_allocator>::deallocate(allocator_instance, location, size)
#else
	#define PLF_DESTROY(the_allocator, allocator_instance, location)			(allocator_instance).destroy(location)
	#define PLF_ALLOCATE(the_allocator, allocator_instance, size, hint)			(allocator_instance).allocate(size, hint)
	#define PLF_DEALLOCATE(the_allocator, allocator_instance, location, size)	(allocator_instance).deallocate(location, size)
#endif


#include <cstring>	// memcpy
#include <algorithm> // std::copy
#include <iterator> // std::move_iterator, std::contiguous_iterator_tag, std::random_access_iterator_tag
#include <memory> // std::uninitialized_copy, std::to_address
#include <deque>


namespace plf
{

#ifndef PLF_TOOLS
	#define PLF_TOOLS

	// std:: tool replacements for C++03/98/11 support:
	template <bool condition, class T = void>
	struct enable_if
	{
		typedef T type;
	};

	template <class T>
	struct enable_if<false, T>
	{};



	template <bool flag, class is_true, class is_false> struct conditional;

	template <class is_true, class is_false> struct conditional<true, is_true, is_false>
	{
		typedef is_true type;
	};

	template <class is_true, class is_false> struct conditional<false, is_true, is_false>
	{
		typedef is_false type;
	};



	template <class element_type>
	struct less
	{
		bool operator() (const element_type &a, const element_type &b) const PLF_NOEXCEPT
		{
			return a < b;
		}
	};



	template<class element_type>
	struct equal_to
	{
		const element_type value;

		explicit equal_to(const element_type store_value): // no noexcept as element may allocate and potentially throw when copied
			value(store_value)
		{}

		bool operator() (const element_type compare_value) const PLF_NOEXCEPT
		{
			return value == compare_value;
		}
	};



	// To enable conversion to void * when allocator supplies non-raw pointers:
	template <class source_pointer_type>
	static PLF_CONSTFUNC void * void_cast(const source_pointer_type source_pointer) PLF_NOEXCEPT
	{
		#ifdef PLF_CPP20_SUPPORT
			return static_cast<void *>(std::to_address(source_pointer));
		#else
			return static_cast<void *>(&*source_pointer);
		#endif
	}



	#ifdef PLF_MOVE_SEMANTICS_SUPPORT
		template <class iterator_type>
		PLF_CONSTFUNC std::move_iterator<iterator_type> make_move_iterator(iterator_type it)
		{
			return std::move_iterator<iterator_type>(std::move(it));
		}
	#endif


	enum priority { performance = 1, memory_use = 4};

#endif



template <class iterator_type>
#ifdef PLF_TYPE_TRAITS_SUPPORT
	PLF_CONSTFUNC inline void copy_or_move(const typename plf::enable_if<std::is_same<typename std::iterator_traits<iterator_type>::iterator_category, std::random_access_iterator_tag>::value, iterator_type>::type destination, const iterator_type source)
#else
	PLF_CONSTFUNC inline void copy_or_move(const iterator_type destination, const iterator_type source)
#endif
{
	#ifdef PLF_TYPE_TRAITS_SUPPORT
		typedef typename iterator_type::value_type value_type;

		#ifdef PLF_MOVE_SEMANTICS_SUPPORT
			#ifdef PLF_EXCEPTIONS_SUPPORT
				if PLF_CONSTEXPR (!std::is_trivially_copy_assignable<value_type>::value && std::is_nothrow_move_assignable<value_type>::value)
			#else
				if PLF_CONSTEXPR (!std::is_trivially_copy_assignable<value_type>::value && std::is_move_assignable<value_type>::value)
			#endif
			{
				*destination = std::move(*source);
			}
			else
		#endif
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if PLF_CONSTEXPR (!std::is_nothrow_copy_assignable<value_type>::value)
			{
				const value_type temp = *destination;

				try
				{
					*destination = *source;
				}
				catch (...)
				{
					*destination = temp;
					throw;
				}
			}
			else
		#endif
	#endif
	{
		*destination = *source;
	}
}



template <class container_type, class iterator_type>
PLF_CONSTFUNC iterator_type single_reorderase(container_type &container, const iterator_type location)
{
	const iterator_type back = container.end() - 1;

	if (location != back)
	{
		plf::copy_or_move(location, back);
	}

	container.pop_back();
	return location;
}



template <class container_type>
#ifdef PLF_TYPE_TRAITS_SUPPORT
	PLF_CONSTFUNC inline typename container_type::iterator reorderase(container_type &container, const typename plf::enable_if<std::is_same<typename std::iterator_traits<typename container_type::iterator>::iterator_category, std::random_access_iterator_tag>::value, typename container_type::iterator>::type location)
#else
	PLF_CONSTFUNC inline typename container_type::iterator reorderase(container_type &container, const typename container_type::iterator location)
#endif
{
	return plf::single_reorderase(container, location);
}



template <class value_type, class allocator_type>
PLF_CONSTFUNC inline typename std::deque<value_type, allocator_type>::iterator reorderase(typename std::deque<value_type, allocator_type> &container, typename std::deque<value_type, allocator_type>::iterator location)
{
	if (location == container.begin())
	{
		container.pop_front();
		return container.begin();
	}

	return plf::single_reorderase(container, location);
}



#ifndef PLF_CPP20_SUPPORT
	template <typename value_type>
	struct is_deque
	{
		static const bool value = false;
	};


	template <typename value_type, typename allocator_type>
	struct is_deque<std::deque<value_type, allocator_type> >
	{
		static const bool value = true;
	};
#endif



template <class container_type, class iterator_type>
PLF_CONSTFUNC iterator_type range_reorderase(container_type &container, const iterator_type first, const iterator_type last)
{
	typedef typename container_type::size_type size_type;
	typedef typename container_type::difference_type difference_type;
	const difference_type distance = static_cast<difference_type>(last - first);

	if (distance == 0)
	{
		return first;
	}
	else if (distance == 1)
	{
		return plf::reorderase(container, first);
	}

	const iterator_type end = container.end();

	if (last == end)
	{
		return container.erase(first, last);
	}

	const iterator_type first_replacement = end - distance;
	const size_type copy_distance = static_cast<size_type>((last > first_replacement) ? distance - (last - first_replacement) : distance);

	#ifdef PLF_TYPE_TRAITS_SUPPORT
		typedef typename container_type::value_type value_type;

		#ifdef PLF_CPP20_SUPPORT
			if PLF_CONSTEXPR (std::is_same<typename std::iterator_traits<iterator_type>::iterator_category, std::contiguous_iterator_tag>::value && std::is_trivially_copyable<value_type>::value)
		#else
			if PLF_CONSTEXPR (!plf::is_deque<container_type>::value && std::is_trivially_copyable<value_type>::value)
		#endif
		{
			#ifdef PLF_ALIGNMENT_SUPPORT
				PLF_CONSTEXPR const size_type aligned_size = (alignof(value_type) > sizeof(value_type)) ? alignof(value_type) : sizeof(value_type); // To compensate for possible overalignment
			#else
				PLF_CONSTEXPR const size_type aligned_size = sizeof(value_type);
			#endif

			std::memcpy(plf::void_cast(first), plf::void_cast(end - copy_distance), copy_distance * aligned_size);
		}
		#ifdef PLF_MOVE_SEMANTICS_SUPPORT
			#ifdef PLF_EXCEPTIONS_SUPPORT
				else if PLF_CONSTEXPR (!std::is_trivially_copyable<value_type>::value && std::is_nothrow_move_assignable<value_type>::value)
			#else
				else if PLF_CONSTEXPR (!std::is_trivially_copyable<value_type>::value && std::is_move_assignable<value_type>::value)
			#endif
			{
				std::copy(plf::make_move_iterator(end - copy_distance), plf::make_move_iterator(end), first);
			}
		#endif
		#ifdef PLF_EXCEPTIONS_SUPPORT
			else if PLF_CONSTEXPR (!std::is_nothrow_copy_assignable<value_type>::value)
			{
				typedef typename container_type::allocator_type allocator_type;
				allocator_type alloc;
				value_type * const temp = PLF_ALLOCATE(allocator_type, alloc, copy_distance, &*end);
				std::uninitialized_copy(first, first + copy_distance, temp);

				try
				{
					std::copy(end - copy_distance, end, first);
				}
				catch (...)
				{
					std::copy(temp, temp + copy_distance, first);

					if PLF_CONSTEXPR (!std::is_trivially_destructible<value_type>::value)
					{
						for (value_type *current = temp, *temp_end = temp + copy_distance; current != temp_end; ++current)
						{
							PLF_DESTROY(allocator_type, alloc, current);
						}
					}

					PLF_DEALLOCATE(allocator_type, alloc, temp, copy_distance);
					throw;
				}

				if PLF_CONSTEXPR (!std::is_trivially_destructible<value_type>::value)
				{
					for (value_type *current = temp, *temp_end = temp + copy_distance; current != temp_end; ++current)
					{
						PLF_DESTROY(allocator_type, alloc, current);
					}
				}

				PLF_DEALLOCATE(allocator_type, alloc, temp, copy_distance);
			}
		#endif
		else
	#endif
	{
		std::copy(end - copy_distance, end, first);
	}

	return container.erase(end - distance, end);
}



template <class container_type>
#ifdef PLF_TYPE_TRAITS_SUPPORT
	PLF_CONSTFUNC inline typename container_type::iterator reorderase(container_type &container, const typename plf::enable_if<std::is_same<typename std::iterator_traits<typename container_type::iterator>::iterator_category, std::random_access_iterator_tag>::value, typename container_type::iterator>::type first, const typename container_type::iterator last)
#else
	PLF_CONSTFUNC inline typename container_type::iterator reorderase(container_type &container, const typename container_type::iterator first, const typename container_type::iterator last)
#endif
{
	return plf::range_reorderase(container, first, last);
}



template <class value_type, class allocator_type>
PLF_CONSTFUNC typename std::deque<value_type, allocator_type>::iterator reorderase(std::deque<value_type, allocator_type> &container, const typename std::deque<value_type, allocator_type>::iterator first, const typename std::deque<value_type, allocator_type>::iterator last)
{
	typedef typename std::deque<value_type, allocator_type> container_type;
	typedef typename container_type::size_type size_type;
	const typename container_type::iterator begin = container.begin();

	if (first == begin)
	{
		container.erase(first, last);
		return last;
	}

	const size_type distance = last - first;

	if (first >= begin + distance)
	{
		return plf::range_reorderase(container, first, last);
	}
	else
	{
		const size_type copy_distance = static_cast<size_type>(first - begin);

		#ifdef PLF_TYPE_TRAITS_SUPPORT
			#ifdef PLF_MOVE_SEMANTICS_SUPPORT
				#ifdef PLF_EXCEPTIONS_SUPPORT
					if PLF_CONSTEXPR (!std::is_trivially_copyable<value_type>::value && std::is_nothrow_move_assignable<value_type>::value)
				#else
					if PLF_CONSTEXPR (!std::is_trivially_copyable<value_type>::value && std::is_move_assignable<value_type>::value)
				#endif
				{
					std::copy(plf::make_move_iterator(begin), plf::make_move_iterator(begin + copy_distance), last - copy_distance);
				}
			#endif
			#ifdef PLF_EXCEPTIONS_SUPPORT
				else if PLF_CONSTEXPR (!std::is_nothrow_copy_assignable<value_type>::value)
				{
					allocator_type alloc;
					value_type * const temp = PLF_ALLOCATE(allocator_type, alloc, copy_distance, &*begin);
					std::uninitialized_copy(first, last, temp);

					try
					{
						std::copy(begin, begin + copy_distance, last - copy_distance);
					}
					catch (...)
					{
						std::copy(temp, temp + copy_distance, first);

						if PLF_CONSTEXPR (!std::is_trivially_destructible<value_type>::value)
						{
							for (value_type *current = temp, *temp_end = temp + copy_distance; current != temp_end; ++current)
							{
								PLF_DESTROY(allocator_type, alloc, current);
							}
						}

						PLF_DEALLOCATE(allocator_type, alloc, temp, copy_distance);
						throw;
					}

					if PLF_CONSTEXPR (!std::is_trivially_destructible<value_type>::value)
					{
						for (value_type *current = temp, *temp_end = temp + copy_distance; current != temp_end; ++current)
						{
							PLF_DESTROY(allocator_type, alloc, current);
						}
					}

					PLF_DEALLOCATE(allocator_type, alloc, temp, copy_distance);
				}
			#endif
			else
		#endif
		{
			std::copy(begin, begin + copy_distance, last - copy_distance);
		}

		return container.erase(begin, begin + copy_distance);
	}
}



// PARTITIONING AND ERASE_IF-equivalents

// Forward-iterator-only variant:
template <class iterator_type, class predicate_function, class work_function>
PLF_CONSTFUNC iterator_type inverse_partition(typename plf::enable_if<std::is_same<typename std::iterator_traits<iterator_type>::iterator_category, std::forward_iterator_tag>::value, iterator_type>::type first, iterator_type last, predicate_function predicate, work_function work)
{
	for (; !static_cast<bool>(predicate(*first)); ++first)
	{
		if (first == last) return first;
	}

	for (iterator_type it = ++iterator_type(first); it != last; ++it)
	{
		if (!static_cast<bool>(predicate(*it)))
		{
			work(first, it);
			++first;
		}
	}

	return first;
}



template <class iterator_type, class predicate_function, class work_function>
PLF_CONSTFUNC iterator_type inverse_partition(typename plf::enable_if<!std::is_same<typename std::iterator_traits<iterator_type>::iterator_category, std::forward_iterator_tag>::value, iterator_type>::type first, iterator_type last, predicate_function predicate, work_function work)
{
	for (; first != last; ++first)
	{
		if (predicate(*first))
		{
			while (predicate(*--last)) // Check for same condition at back of range
			{
				if (last == first) return last;
			}

			work(first, last);
		}
	}

	return last;
}



template <class iterator_type>
struct iter_swap_functor
{
	PLF_CONSTFUNC void operator() (const iterator_type a, const iterator_type b)
	{
		std::iter_swap(a, b);
	}
};



template <class iterator_type>
struct copy_or_move_functor
{
	PLF_CONSTFUNC void operator() (const iterator_type a, const iterator_type b)
	{
		plf::copy_or_move(a, b);
	}
};



template <class functor_type>
struct functor_negator
{
	 functor_type &functor;

	 PLF_CONSTFUNC functor_negator(functor_type &func) PLF_NOEXCEPT :
		functor(func)
	{}

	 template <typename argument_type>
	 PLF_CONSTFUNC bool operator () (argument_type &argument)
	{
		return !static_cast<bool>(functor(argument));
	}
};




template <class iterator_type, class predicate_function>
PLF_CONSTFUNC inline iterator_type partition(iterator_type first, iterator_type last, predicate_function predicate)
{
	return plf::inverse_partition(first, last, plf::functor_negator(predicate), plf::iter_swap_functor<iterator_type>());
}



template <class iterator_type, class predicate_function>
PLF_CONSTFUNC inline iterator_type destructive_partition(iterator_type first, iterator_type last, predicate_function predicate)
{
	return plf::inverse_partition(first, last, plf::functor_negator(predicate), plf::copy_or_move_functor<iterator_type>());
}



template <class container_type, class predicate_function>
PLF_CONSTFUNC inline typename container_type::size_type reorderase_all_if(container_type &container, predicate_function predicate)
{
	typedef typename container_type::iterator iterator_type;
	const iterator_type end = container.end(), begin = plf::inverse_partition(container.begin(), end, predicate, plf::copy_or_move_functor<iterator_type>());
	const typename container_type::size_type range_size = std::distance(begin, end);
	container.erase(begin, end);
	return range_size;
}



template <class container_type, class value_type>
PLF_CONSTFUNC inline typename container_type::size_type reorderase_all(container_type &container, const value_type &value)
{
	typedef typename container_type::value_type element_type;
	return plf::reorderase_all_if(container, plf::equal_to<element_type>(static_cast<element_type>(value)));
}



template <class container_type, class iterator_type, class predicate_function>
PLF_CONSTFUNC typename container_type::size_type reorderase_all_if(container_type &container, iterator_type first, iterator_type last, predicate_function predicate)
{
	typedef typename container_type::size_type size_type;
	iterator_type end = container.end();

	if (last == end)
	{
		const iterator_type begin = plf::inverse_partition(first, end, predicate, plf::copy_or_move_functor<iterator_type>());
		const size_type range_size = std::distance(begin, end);
		container.erase(begin, end);
		return range_size;
	}

	size_type count = 0;

	for (; first != last; ++first)
	{
		if (predicate(*first))
		{
			while (predicate(*--end)) // Check for same condition at back of range
			{
				if (end == first) return count;
			}

			if (end < last) last = end;

			plf::copy_or_move(first, end);
			plf::single_reorderase(container, end);
			++count;
		}
	}

	return count;
}



template <class container_type, class iterator_type, class value_type>
PLF_CONSTFUNC typename container_type::size_type reorderase_all(container_type &container, iterator_type first, iterator_type last, const value_type &value)
{
	typedef typename container_type::value_type element_type;
	return plf::reorderase_all_if(container, first, last, plf::equal_to<element_type>(static_cast<element_type>(value)));
}


} // plf namespace


#undef PLF_ALIGNMENT_SUPPORT
#undef PLF_TYPE_TRAITS_SUPPORT
#undef PLF_ALLOCATOR_TRAITS_SUPPORT
#undef PLF_MOVE_SEMANTICS_SUPPORT
#undef PLF_NOEXCEPT
#undef PLF_CONSTEXPR
#undef PLF_CONSTFUNC
#undef PLF_CPP20_SUPPORT

#undef PLF_DESTROY
#undef PLF_ALLOCATE
#undef PLF_DEALLOCATE

#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
	#pragma warning ( pop )
#endif

#endif // PLF_REORDERASE
