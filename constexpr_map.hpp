#ifndef CONSTEXPR_MAP_HPP
#define CONSTEXPR_MAP_HPP

#include <array>
#include <functional>
#include <optional>
#include <stdexcept>
#include <utility>

/**
 * @brief A C++17-compatible constexpr map with a fixed size.
 *
 * This class provides an immutable, map-like interface over a fixed-size
 * std::array of key-value pairs.
 *
 * @tparam Key The key type. Must support `operator==`.
 * @tparam Value The value type.
 * @tparam Size The number of key-value pairs in the map.
 */
template <typename Key, typename Value, std::size_t Size>
class ConstexprMap {
   private:
    /**
     * @brief C++17 helper to construct the internal array at compile-time.
     *
     * This workaround is necessary because std::pair::operator= was not
     * constexpr until C++20. This function uses list initialization to
     * bypass the non-constexpr assignment.
     */
    template <std::size_t... I>
    static constexpr std::array<std::pair<Key, Value>, Size> makeArray(
        const std::array<std::pair<Key, Value>, Size>& values,
        std::index_sequence<I...> /*unused*/) {
        return {{{values[I].first, values[I].second}...}};
    }

   public:
    /**
     * @brief Constructs the map from an array of key-value pairs.
     */
    constexpr explicit ConstexprMap(
        const std::array<std::pair<Key, Value>, Size>& values)
        : m_data(makeArray(values, std::make_index_sequence<Size>())) {}

    /**
     * @brief Finds a value by its key. (constexpr)
     *
     * @param key The key to search for.
     * @return An std::optional containing a reference_wrapper to the value if
     * found; otherwise, std::nullopt.
     */
    [[nodiscard]] constexpr std::optional<std::reference_wrapper<const Value>>
    find(const Key& key) const noexcept {
        for (const auto& pair : m_data) {
            if (pair.first == key) {
                return pair.second;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Accesses a value by its key. Throws on failure. (RUNTIME ONLY)
     * @note This function is NOT constexpr to remain C++17 compatible.
     *
     * @param key The key to look up.
     * @return A const reference to the value associated with the key.
     * @throws std::out_of_range if the key is not found.
     */
    [[nodiscard]] const Value& at(const Key& key) const {
        const auto result = find(key);
        if (result) {
            return result->get();
        }
        throw std::out_of_range("Key not found in ConstexprMap");
    }

    /**
     * @brief Returns the number of elements in the map.
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept { return Size; }

    /**
     * @brief Checks if the map is empty.
     */
    [[nodiscard]] constexpr bool empty() const noexcept { return Size == 0; }

   private:
    const std::array<std::pair<Key, Value>, Size> m_data;
};

#endif  // CONSTEXPR_MAP_HPP
