//
// Created by daniele on 22/03/26.
//

#pragma once

#include <string>
#include <tuple>
#include <sstream>
#include <limits>

namespace dferone {
    namespace detail {
        template<class Value>
        inline void print_value(std::ostream &out, const Value &value) {
            out << value;
        }

        template<class Key, class Value>
        inline void print_value(std::ostream &out, const std::pair<Key, Value> &kv) {
            out << kv.first << ": " << kv.second;
        }
    } // namespace detail

    /** @brief  Joins the elements of a container into a string, with an optional
     *          separator and an optional formatting function.
     *
     *  If no formatter is provided, defaults to operator<< (with special handling
     *  for std::pair, printing "key: value").
     *  If a formatter is provided, it must be callable as: formatter(element) -> string-like,
     *  or accept (ostream&, element).
     *
     *  @tparam Container   Range type.
     *  @tparam Formatter   Callable type (optional).
     *  @param  container   The container to join.
     *  @param  separator   String interposed between adjacent elements (default ", ").
     *  @param  formatter   Optional function to convert each element to string.
     *  @return             The joined string.
     */
    template<std::ranges::range Container, class Formatter = std::nullptr_t>
    std::string join(Container &&container, const std::string_view separator = ", ", Formatter formatter = nullptr) {
        std::ostringstream ss;
        bool first = true;

        for (auto &&elem : container) {
            if (!first)
                ss << separator;
            first = false;

            if constexpr (!std::is_null_pointer_v<Formatter>) {
                ss << formatter(elem);
            } else {
                detail::print_value(ss, elem);
            }
        }

        return ss.str();
    }

    template<std::ranges::range Container, class Formatter>
    std::string join(Container &&container, Formatter formatter) {
        return join(std::forward<Container>(container), ", ", formatter);
    }

#if !defined(__cpp_lib_ranges_enumerate) || __cpp_lib_ranges_enumerate < 202302L
    /** @brief  Iterates over an iterable container and yields both the index and
  *          the element.
  *
  *          This method works analogously to Python's enumerate().
  *          The code below is not mine, but by Nathan Reed and it was
  *          origianlly available at http://reedbeta.com/blog/python-like-enumerate-in-cpp17/
  *
  *  @tparam Container   The container type.
  *  @param  iterable    An instance of the iterable container.
  *  @return             An anonymous struct implementing begin() and end(). When passed in
  *                      a range-based for loop, each element gives a tuple whose second element
  *                      is an iterable element, and whose first element is the corresponding
  *                      index.
  */
    template<typename Container, typename Iter = decltype(std::begin(std::declval<Container>())), typename = decltype(std::end(std::declval<Container>()))>
    constexpr auto enumerate(Container &&iterable) {
        struct iterator {
            std::size_t i;
            Iter iter;

            bool operator!=(const iterator &other) const { return iter != other.iter; }
            void operator++() {
                ++i;
                ++iter;
            }
            auto operator*() const { return std::tie(i, *iter); }
        };

        struct iterable_wrapper {
            Container iterable;

            auto begin() { return iterator{0u, std::begin(iterable)}; }
            auto end() { return iterator{0u, std::end(iterable)}; }
        };

        return iterable_wrapper{std::forward<Container>(iterable)};
    }
#endif

#if !defined(__cpp_lib_ranges_contains) || __cpp_lib_ranges_contains < 202207L
    /** @brief  Tells whether a container contains a certain element.
  *
  *  The standard library's functions to find elements (e.g. std::find)
  *  always return an iterator. Sometimes, though, we just want to know
  *  whether an element is in a container or not. This helper function
  *  lets us do this in a concise way. This function also has specialisation
  *  for when the container implements a .count() method, i.e. a more
  *  efficient way of searching elements than simple linear search.
  *
  *  @tparam Container    Container type.
  *  @tparam T            Containee type.
  *  @param  container    The container.
  *  @param  element      The element we are searching in \p container.
  *  @return              True iff \p element was found in \p container.
  */
    template<std::ranges::range Container, class T>
    inline bool contains(const Container &container, const T &element) {
        return std::find(container.begin(), container.end(), element) != container.end();
    };

    template<std::ranges::range Container, class T>
    inline bool contains(const Container &container, const T &element)
        requires requires {
        { container.count(element) } -> std::convertible_to<typename Container::size_type>;
    }
{
    return container.count(element) > 0u;
};
#endif

    /** @brief  Skips a certain number of lines from an input file stream.
 *
 *  @param stream    The file stream.
 *  @param how_many  Number of lines to skip.
 */
    inline void skip_lines(std::istream &stream, const std::size_t how_many = 1u) {
        for (auto i = 0u; i < how_many; ++i) {
            stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

}