#include "options.hpp"

namespace simconfig
{

// Define alias for an OptionArray of compile-time strings
template<OptionEnum Enum>
using KeyStrings_t = OptionArray<std::string_view, Enum>;

// Define global variable template KeyStrings
template<OptionEnum Enum>
constexpr KeyStrings_t<Enum> KeyStrings;

// Define concept for KeyEnum: any OptionEnum with an explicitly specialized KeyStrings
template<typename T>
concept KeyEnum = OptionEnum<T> && (KeyStrings<T>.size() == T::SIZE());

// Define KeyVector: a vector of KeyEnum enumerators
template<KeyEnum Enum>
using KeyVector = std::vector<Enum>;

// Define type traits to check if a type is a KeyVector
template<typename T>
struct is_key_vector : std::false_type {};
template<KeyEnum E>
struct is_key_vector<KeyVector<E>> : std::true_type {};
template<typename T>
inline constexpr bool is_key_vector_v = is_key_vector<T>::value;

} // namespace config