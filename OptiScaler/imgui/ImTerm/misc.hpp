#ifndef IMTERM_MISC_HPP
#define IMTERM_MISC_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2019, Lucas Lazare                                                                                                     ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,         ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  is furnished to do so, subject to the following conditions:                                                                        ///
///                                                                                                                                     ///
///  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.     ///
///                                                                                                                                     ///
///  The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the               ///
///  warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or              ///
///  copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,        ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#include <utility>
#include <iterator>
#include <functional>
#include <type_traits>

namespace misc {

	// std::identity is c++20
	struct identity {
		template<typename T>
		constexpr T&& operator()(T&& t) const {
			return std::forward<T>(t);
		}
	};

	namespace details {
		struct structured_void {};
		inline structured_void no_value{};

		template <typename T>
		struct non_void {
			using type = T;
		};
		template <>
		struct non_void<void> {
			using type = structured_void;
		};
	}
	// type T if T is non void, details::structured void otherwise
	template <typename T>
	using non_void_t = typename details::non_void<T>::type;


	// Returns the length of the given byte string, at most buffer_size
	constexpr unsigned int strnlen(const char* beg, unsigned int buffer_size) {
		unsigned int len = 0;
		while (len < buffer_size && beg[len] != '\0') {
			++len;
		}
		return len;
	}

	// std::is_sorted is not constexpr pre C++20
	template <typename ForwardIt, typename EndIt, typename Comparator = std::less<std::remove_reference_t<decltype(*std::declval<ForwardIt>())>>>
	constexpr bool is_sorted(ForwardIt it, EndIt last, Comparator&& comp = {}) {
		if (it == last) {
			return true;
		}

		for (ForwardIt next = std::next(it) ; next != last ; ++next, ++it) {
			if (comp(*next, *it)) {
				return false;
			}
		}
		return true;
	}

	// returns the length of the longest element of the list, minimum 0
	template <typename ForwardIt, typename EndIt, typename SizeExtractor>
	constexpr auto max_size(ForwardIt it, EndIt last, SizeExtractor&& size_of) -> decltype(size_of(*it)){
		using size_type = decltype(size_of(*it));
		size_type max_size = 0u;
		while (it != last) {
			size_type current_size = size_of(*it++);
			if (current_size > max_size) {
				max_size = current_size;
			}
		}
		return max_size;
	}

	// copies until it reaches the end of any of the two collections
	template <typename ForwardIt, typename OutputIt>
	constexpr void copy(ForwardIt src_beg, ForwardIt src_end, OutputIt dest_beg, OutputIt dest_end) {
		while(src_beg != src_end && dest_beg != dest_end) {
			*dest_beg++ = *src_beg++;
		}
	}

	// same as misc::copy, but in reversed order
	template <typename BidirIt1, typename BidirIt2>
	constexpr void copy_backward(BidirIt1 src_beg, BidirIt1 src_end, BidirIt2 dest_beg, BidirIt2 dest_end) {
		auto copy_length = std::distance(src_beg, src_end);
		auto avail_length = std::distance(dest_beg, dest_end);
		if (avail_length < copy_length) {
			std::advance(src_end, avail_length - copy_length);
		} else {
			std::advance(dest_end, copy_length - avail_length);
		}
		while(src_beg != src_end && dest_beg != dest_end) {
			*--dest_end = *--src_end;
		}
	}

	// Returns new end of dest collection
	// ignores the n first values of the destination
	template <typename ForwardIt, typename RandomAccessIt>
	constexpr RandomAccessIt erase_insert(ForwardIt src_beg, ForwardIt src_end, RandomAccessIt dest_beg, RandomAccessIt dest_end, RandomAccessIt dest_max, unsigned int n) {
		n = std::min(static_cast<unsigned>(std::distance(dest_beg, dest_end)), n);
		auto copy_length = static_cast<unsigned>(std::distance(src_beg, src_end));
		auto avail_length = static_cast<unsigned>(std::distance(dest_end, dest_max)) + n;

		if (copy_length <= avail_length) {
			misc::copy_backward(dest_beg + n, dest_end, dest_beg + copy_length, dest_end + copy_length);
			misc::copy(src_beg, src_end, dest_beg, dest_beg + copy_length);
			return dest_end + copy_length - n;

		} else {
			std::advance(src_beg, copy_length - avail_length);
			copy_length = avail_length;
			misc::copy_backward(dest_beg + n, dest_end, dest_beg + copy_length, dest_end + copy_length);
			misc::copy(src_beg, src_end, dest_beg, dest_beg + copy_length);
			return dest_end + copy_length - n;
		}
	}

	// returns the last element in the range [begin, end) that is equal to val, returns end if there is no such element
	template <typename ForwardIterator, typename ValueType>
	constexpr ForwardIterator find_last(ForwardIterator begin, ForwardIterator end, const ValueType& val) {
		auto rend = std::reverse_iterator(begin);
		auto rbegin = std::reverse_iterator(end);
		auto search = std::find(rbegin, rend, val);
		if (search == rend) {
			return end;
		}
		return std::prev(search.base());
	}

	// Returns an iterator to the first character that has no a space after him
	template <typename ForwardIterator, typename SpaceDetector>
	constexpr ForwardIterator find_terminating_word(ForwardIterator begin, ForwardIterator end, SpaceDetector&& is_space_pred) {
		auto rend = std::reverse_iterator(begin);
		auto rbegin = std::reverse_iterator(end);

		int sp_size = 0;
		auto is_space = [&sp_size, &is_space_pred, &end] (char c) {
			sp_size = is_space_pred(std::string_view{&c, static_cast<unsigned>(&*std::prev(end) - &c)});
			return sp_size > 0;
		};

		auto search = std::find_if(rbegin, rend, is_space);
		if (search == rend) {
			return begin;
		}
		ForwardIterator it = std::prev(search.base());
		it += sp_size;
		return it;
	}

	constexpr bool success(std::errc ec) {
		return ec == std::errc{};
	}

	// Search any element starting by "prefix" in the sorted collection formed by [c_beg, c_end)
	// str_ext must map dectype(*c_beg) to std::string_view
	// transform is whatever transformation you want to do to the matching elements.
	template <typename ForwardIt, typename StrExtractor = identity, typename Transform = identity>
	auto prefix_search(std::string_view prefix, ForwardIt c_beg, ForwardIt c_end,
			StrExtractor&& str_ext = {}, Transform&& transform = {}) -> std::vector<std::decay_t<decltype(transform(*c_beg))>> {

		auto lower = std::lower_bound(c_beg, c_end, prefix);
		auto higher = std::upper_bound(lower, c_end, prefix, [&str_ext](std::string_view pre, const auto& element) {
			std::string_view val = str_ext(element);
			return pre.substr(0, std::min(val.size(), pre.size())) < val.substr(0, std::min(val.size(), pre.size()));
		});

		std::vector<std::decay_t<decltype(transform(*c_beg))>> ans;
		std::transform(lower, higher, std::back_inserter(ans), std::forward<Transform>(transform));
		return ans;
	}

	// returns an iterator to the first element starting by "prefix"
	// is_space is a predicate returning a value greater than 0 if a given string_view starts by a space and 0 otherwise
	// str_ext is an unary functor maping decltype(*beg) to std::string_view
	template <typename ForwardIt, typename IsSpace, typename StrExtractor = identity>
	ForwardIt find_first_prefixed(std::string_view prefix, ForwardIt beg, ForwardIt end, IsSpace&& is_space, StrExtractor&& str_ext = {}) {
		// std::string_view::start_with is C++20
		auto start_with = [&](std::string_view str, std::string_view pr) {
			std::string_view::size_type idx = 0;
			int space_count{};
			while (idx < str.size() && (space_count = is_space(str.substr(idx))) > 0) {
				idx += space_count;
			}
			return (str.size() - idx) >= pr.size() ? str.substr(idx, pr.size()) == pr : false;
		};
		while (beg != end) {
			if (start_with(str_ext(*beg), prefix)) {
				return beg;
			}
			++beg;
		}
		return beg;
	}


	namespace details {
		template <typename AlwaysVoid, template<typename...> typename Op, typename... Args>
		struct detector : std::false_type {
			using type = std::is_same<void,void>;
		};

		template <template<typename...> typename Op, typename... Args>
		struct detector<std::void_t<Op<Args...>>, Op, Args...> : std::true_type {
			using type = Op<Args...>;
		};
	}

	template <template<typename...> typename Op, typename... Args>
	using is_detected = typename details::detector<void, Op, Args...>;

	template <template<typename...> typename Op, typename... Args>
	using is_detected_t = typename is_detected <Op, Args...>::type;

	// compile time function detection, return type agnostic
	template <template<typename...> typename Op, typename... Args>
	constexpr bool is_detected_v = is_detected<Op, Args...>::value;

	// compile time function detection
	template <template<typename...> typename Op, typename ReturnType, typename... Args>
	constexpr bool is_detected_with_return_type_v = std::is_same_v<is_detected_t<Op, Args...>, ReturnType>;

	// dummy mutex
	struct no_mutex {
		constexpr no_mutex() noexcept = default;
		constexpr void lock() {}
		constexpr void unlock() {}
		constexpr bool try_lock() { return true; }
	};
}

#endif //IMTERM_MISC_HPP
