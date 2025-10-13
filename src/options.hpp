#include <cstdint>
#include <type_traits>

namespace simconfig
{

// Define concept for an OptionEnum: An enum of underlying type std::uint8_t with a SIZE enumerator
template <typename Enum>
concept OptionEnum = std::is_enum_v<Enum> && 
std::is_same_v<std::underlying_type_t<Enum>, std::uint8_t> &&
requires { Enum::SIZE; };


template<OptionEnum Enum>
constexpr Enum GetEnumerator(std::uint8_t i)
{
if (i >= GetSize<Enum>())
   throw std::logic_error("Error: Index i is not less than ENUM::SIZE");

return static_cast<Enum>(i);
};

// Define OptionArray -- Create and index std::array using OptionEnums

// This idea was adapted from https://stackoverflow.com/a/69302415, using concepts (C++20).
// Allows usage of scoped-enums for indexing arrays based on their underlying type uint8_t at compile time.
// This is faster than if std::map were used.
template<typename T, OptionEnum Enum> 
struct OptionArray : public std::array<T, Enum::Size>
{
   using std::array<T, Enum::Size>::operator[];

   constexpr T& operator[](const Enum& e) 
   { return std::array<T, Enum::Size>::operator[](static_cast<std::uint8_t>(e)); };

   constexpr const T& operator[](const Enum& e) const
   { return std::array<T, Enum::Size>::operator[](static_cast<std::uint8_t>(e)); };
};

} // namespace simconfig