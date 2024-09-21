//Source: https://godbolt.org/z/cnrzKr
//Video: https://www.youtube.com/watch?v=INn3xa4pMfg
//More complete version: https://github.com/lefticus/tools/blob/main/include/lefticus/tools/flat_map_adapter.hpp

#include <array>
#include <string_view>
#include <stdexcept>
#include <algorithm>
#include <map>

template <typename Key, typename Value, std::size_t Size>
struct Map {
  std::array<std::pair<Key, Value>, Size> data;

  [[nodiscard]] constexpr Value at(const Key &key) const {
    const auto itr =
        std::find_if(begin(data), end(data),
                     [&key](const auto &v) { return v.first == key; });
    if (itr != end(data)) {
      return itr->second;
    } else {
      throw std::range_error("Not Found");
    }
  }

};

using namespace std::literals::string_view_literals;
static constexpr std::array<std::pair<std::string_view, int>, 8> color_values{
    {{"black"sv, 7},
     {"blue"sv, 3},
     {"cyan"sv, 5},
     {"green"sv, 2},
     {"magenta"sv, 6},
     {"red"sv, 1},
     {"white"sv, 8},
     {"yellow"sv, 4}}};

int lookup_value(const std::string_view sv) {
  //static const auto map = std::map<std::string_view, int>{color_values.begin(), color_values.end()};
  static constexpr auto map =
      Map<std::string_view, int, color_values.size()>{{color_values}};

  return map.at(sv);
}
