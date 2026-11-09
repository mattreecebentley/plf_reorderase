// Copyright (c) 2026, Matthew Bentley (mattreecebentley@gmail.com) www.plflib.org

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


#ifndef PLF_COMPILER_DEFINES
	#define PLF_REORDERASE_DEFINES // ie. No encapsulating unit/class has previously defined the compiler feature macros in plf_tools.h below, so allow this header to undefine them at it's end.
#endif

#define PLF_INCLUDE_UNINITIALIZED_TOOLS
#define PLF_INCLUDE_TOOLS
#include "plf_tools.h"



#include <cstring>	// memcpy
#include <algorithm> // std::copy, std::find_if
#include <iterator> // std::move_iterator, std::contiguous_iterator_tag, std::random_access_iterator_tag
#include <memory> // std::to_address
#include <deque>


namespace plf
{


// How elements are copied or moved over other elements:
struct copy_assign_tag {};
struct move_assign_tag {};
struct backup_copy_assign_tag {}; // Copy assignment may throw, so the elements about to be overwritten are backed up first and restored if it does



// Picks the tag at compile time. Below C++17 if PLF_CONSTEXPR is a plain if, so every branch of a runtime if would be instantiated, including the copy branches for move-only types:
template <class value_type>
struct assignment_tag
{
	#if defined(PLF_TYPE_TRAITS_SUPPORT) && defined(PLF_MOVE_SEMANTICS_SUPPORT)
		#ifdef PLF_EXCEPTIONS_SUPPORT
			// Move when moving cannot throw, and always for types which cannot be copied. For the latter, if a move throws, the elements already moved are not restored:
			typedef typename plf::conditional<(!std::is_trivially_copyable<value_type>::value && std::is_nothrow_move_assignable<value_type>::value) || !std::is_copy_assignable<value_type>::value, move_assign_tag,
				typename plf::conditional<!std::is_nothrow_copy_assignable<value_type>::value, backup_copy_assign_tag, copy_assign_tag>::type>::type type;
		#else
			typedef typename plf::conditional<(!std::is_trivially_copyable<value_type>::value && std::is_move_assignable<value_type>::value) || !std::is_copy_assignable<value_type>::value, move_assign_tag, copy_assign_tag>::type type;
		#endif
	#elif defined(PLF_TYPE_TRAITS_SUPPORT) && defined(PLF_EXCEPTIONS_SUPPORT)
		typedef typename plf::conditional<!std::is_nothrow_copy_assignable<value_type>::value, backup_copy_assign_tag, copy_assign_tag>::type type;
	#else
		typedef copy_assign_tag type;
	#endif
};



template <class iterator_type>
PLF_CONSTFUNC inline void copy_or_move_implementation(const iterator_type destination, const iterator_type source, const copy_assign_tag)
{
	*destination = *source;
}



#ifdef PLF_MOVE_SEMANTICS_SUPPORT
	template <class iterator_type>
	PLF_CONSTFUNC inline void copy_or_move_implementation(const iterator_type destination, const iterator_type source, const move_assign_tag)
	{
		*destination = std::move(*source);
	}
#endif



#if defined(PLF_TYPE_TRAITS_SUPPORT) && defined(PLF_EXCEPTIONS_SUPPORT)
	template <class iterator_type>
	PLF_CONSTFUNC void copy_or_move_implementation(const iterator_type destination, const iterator_type source, const backup_copy_assign_tag)
	{
		const typename std::iterator_traits<iterator_type>::value_type temp = *destination;

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
#endif



template <class iterator_type>
PLF_CONSTFUNC inline void copy_or_move(const iterator_type destination, const iterator_type source)
{
	plf::copy_or_move_implementation(destination, source, typename plf::assignment_tag<typename std::iterator_traits<iterator_type>::value_type>::type());
}



// Copies or moves count elements from source to destination. The two ranges must not overlap:
template <class container_type, class iterator_type, class size_type>
PLF_CONSTFUNC inline void range_copy_or_move(const iterator_type destination, const iterator_type source, const size_type count, const copy_assign_tag)
{
	std::copy(source, source + count, destination);
}



#ifdef PLF_MOVE_SEMANTICS_SUPPORT
	template <class container_type, class iterator_type, class size_type>
	PLF_CONSTFUNC inline void range_copy_or_move(const iterator_type destination, const iterator_type source, const size_type count, const move_assign_tag)
	{
		std::copy(plf::make_move_iterator(source), plf::make_move_iterator(source + count), destination);
	}
#endif



#if defined(PLF_TYPE_TRAITS_SUPPORT) && defined(PLF_EXCEPTIONS_SUPPORT)
	template <class container_type, class iterator_type, class size_type>
	PLF_CONSTFUNC void range_copy_or_move(const iterator_type destination, const iterator_type source, const size_type count, const backup_copy_assign_tag)
	{
		typedef typename std::iterator_traits<iterator_type>::value_type value_type;
		typedef typename container_type::allocator_type allocator_type; // Only needed here, so containers without an allocator_type (eg. static_vector) can use the other paths
		allocator_type alloc;
		value_type * const temp = PLF_ALLOCATE(allocator_type, alloc, count, &*destination);
		plf::uninitialized_copy(destination, destination + count, temp, alloc); // Back up the elements which are about to be overwritten

		try
		{
			std::copy(source, source + count, destination);
		}
		catch (...)
		{
			std::copy(temp, temp + count, destination);

			if PLF_CONSTEXPR (!std::is_trivially_destructible<value_type>::value)
			{
				for (value_type *current = temp, *temp_end = temp + count; current != temp_end; ++current)
				{
					PLF_DESTROY(allocator_type, alloc, current);
				}
			}

			PLF_DEALLOCATE(allocator_type, alloc, temp, count);
			throw;
		}

		if PLF_CONSTEXPR (!std::is_trivially_destructible<value_type>::value)
		{
			for (value_type *current = temp, *temp_end = temp + count; current != temp_end; ++current)
			{
				PLF_DESTROY(allocator_type, alloc, current);
			}
		}

		PLF_DEALLOCATE(allocator_type, alloc, temp, count);
	}
#endif



template <class container_type, class iterator_type>
PLF_CONSTFUNC iterator_type single_reorderase(container_type &container, const iterator_type location)
{
	iterator_type back = container.end();

	if (location == --back)
	{
		container.pop_back();
		return container.end(); // new end post-pop
	}

	plf::copy_or_move(location, back);
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
		else
	#endif
	{
		plf::range_copy_or_move<container_type>(first, end - copy_distance, copy_distance, typename plf::assignment_tag<typename container_type::value_type>::type());
	}

	const difference_type first_index = first - container.begin();
	container.erase(end - distance, end);
	return container.begin() + first_index; // first's former index now holds the element following the erased range. Re-obtained from begin() as erase() may invalidate first (eg. basic_string)
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

	if (first == begin || last == container.end())
	{
		return container.erase(first, last);
	}


	const size_type distance = last - first;

	if (first >= begin + distance)
	{
		return plf::range_reorderase(container, first, last);
	}
	else
	{
		const size_type copy_distance = static_cast<size_type>(first - begin);
		plf::range_copy_or_move<container_type>(last - copy_distance, begin, copy_distance, typename plf::assignment_tag<value_type>::type());
		container.erase(begin, last - copy_distance); // The front elements have been copied to the end of the range, so erase everything before them
		return container.begin() + static_cast<typename container_type::difference_type>(copy_distance); // The element which followed the erased range is now directly after the front elements
	}
}



// PARTITIONING AND ERASE_IF-equivalents

// Forward-iterator-only variant:
template <class iterator_type, class predicate_function, class work_function>
PLF_CONSTFUNC iterator_type inverse_partition_implementation(iterator_type first, const iterator_type last, predicate_function &predicate, work_function &work, const std::forward_iterator_tag)
{
	first = std::find_if(first, last, predicate);

	if (first == last) // Empty range or no element matches
	{
		return last;
	}

	for (iterator_type it = first; ++it != last;)
	{
		if (!static_cast<bool>(predicate(*it)))
		{
			work(first, it);
			++first;
		}
	}

	return first;
}



// Bidirectional and random-access variant:
template <class iterator_type, class predicate_function, class work_function>
PLF_CONSTFUNC iterator_type inverse_partition_implementation(iterator_type first, iterator_type last, predicate_function &predicate, work_function &work, const std::bidirectional_iterator_tag)
{
	for (; first != last; ++first)
	{
		if (static_cast<bool>(predicate(*first)))
		{
			while (static_cast<bool>(predicate(*--last))) // Check for same condition at back of range
			{
				if (last == first) return last;
			}

			work(first, last);
		}
	}

	return last;
}



// Moves all elements which match the predicate to the back of the range, using work(destination, source) to move non-matching elements forward. Returns the start of the matching elements:
template <class iterator_type, class predicate_function, class work_function>
PLF_CONSTFUNC inline iterator_type inverse_partition(const typename plf::enable_if<true, iterator_type>::type first, const iterator_type last, predicate_function predicate, work_function work) // first is non-deduced so that iterator_type comes from last only
{
	return plf::inverse_partition_implementation(first, last, predicate, work, typename std::iterator_traits<iterator_type>::iterator_category());
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

	 template <typename argument_type>
	 PLF_CONSTFUNC bool operator () (const argument_type &argument) // For iterators which dereference to a temporary (eg. std::vector<bool>)
	{
		return !static_cast<bool>(functor(argument));
	}
};




template <class iterator_type, class predicate_function>
PLF_CONSTFUNC inline iterator_type partition(iterator_type first, iterator_type last, predicate_function predicate)
{
	return plf::inverse_partition(first, last, plf::functor_negator<predicate_function>(predicate), plf::iter_swap_functor<iterator_type>());
}



template <class iterator_type, class predicate_function>
PLF_CONSTFUNC inline iterator_type destructive_partition(iterator_type first, iterator_type last, predicate_function predicate)
{
	return plf::inverse_partition(first, last, plf::functor_negator<predicate_function>(predicate), plf::copy_or_move_functor<iterator_type>());
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
	const iterator_type end = container.end();

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
			iterator_type back = container.end();
			--back; // The back element is outside the range, so it is kept whatever its value
			const bool range_reaches_end = (back == last);

			plf::copy_or_move(first, back);
			container.pop_back();
			++count;

			if (range_reaches_end) // The rest of the range is now at the end of the container
			{
				return count + plf::reorderase_all_if(container, ++first, container.end(), predicate);
			}
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


#ifdef PLF_REORDERASE_DEFINES
	#include "plf_tools_undef.h"
#endif

#endif // PLF_REORDERASE
