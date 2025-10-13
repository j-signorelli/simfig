#include "options.hpp"


namespace simfig
{

// --------------------------------------------------------------------------------
// Define useful aliases

template<OptionEnum Enum>
using KeyStrings_t = OptionArray<std::string_view, Enum>;

// Each flag has a string name + number of EXTRA fields it requires
using FlagSetting_t = std::pair<std::string_view, std::size_t>;

template<std::size_t NumFlags>
using FlagsMeta_t = std::array<FlagSetting_t, NumFlags>;

template<std::size_t NumFields>
using FieldNames_t = std::array<std::string_view, NumFields>;

// --------------------------------------------------------------------------------
// Define type traits for FlagsMeta_t + FieldNames_t

template<typename T>
struct is_flagmeta_arr : std::false_type {};
template<std::size_t NumFlags>
struct is_flagmeta_arr<const FlagsMeta_t<NumFlags>> : std::true_type {};
template<typename T>
inline constexpr bool is_flagmeta_arr_v  = is_flagmeta_arr<T>::value;

template<typename T>
struct is_fieldname_arr : std::false_type {};
template<std::size_t NumFields>
struct is_fieldname_arr<const FieldNames_t<NumFields>> : std::true_type {};
template<typename T>
inline constexpr bool is_fieldname_arr_v = is_fieldname_arr<T>::value;

// --------------------------------------------------------------------------------
// Define primary template for EndpointData, ValidEnpoindData concept, and EndpointDataVariant

template<auto e>
struct EndpointData; // forward declaration

// Define ValidEndpointData concept
/* Each EndpointDataImpl must have:
   
   static constexpr std::string_view Type = "Name of type";
   static constexpr FlagsMeta_t<# flags> FlagsMeta {FlagSetting{},...};
   static constexpr FieldNames_t<# fields> FieldNames {"FieldName",...};

   (constinit) std::bitset<# flags> flags;
   (constinit) std::tuple<(# fields) field types here> fields;

*/

template<auto V>
concept ValidEndpointData = requires
{
   EndpointData<V>::Type; // Type must be defined
   EndpointData<V>::FlagsMeta; // FlagsMeta must be defined
   EndpointData<V>::FieldNames; // FieldNames must be defined
   EndpointData<V>::flags; // flags must be defined
   EndpointData<V>::fields; // fields must be defined
   requires std::same_as<decltype(EndpointData<V>::Type), const std::string_view>; // Type must be a const string_view
   requires std::is_member_object_pointer_v<decltype(&EndpointData<V>::Type)> == false; // Type must be static

   requires is_flagmeta_arr_v<decltype(EndpointData<V>::FlagsMeta)>; // FlagsMeta must be a FlagsMeta_t
   requires std::is_member_object_pointer_v<decltype(&EndpointData<V>::FlagsMeta)> == false; // FlagsMeta must be static
   
   requires is_fieldname_arr_v<decltype(EndpointData<V>::FieldNames)>;// FieldNames must be a FieldNames_t
   requires std::is_member_object_pointer_v<decltype(&EndpointData<V>::FieldNames)> == false;// FieldNames must be static
   
   requires std::same_as<std::bitset<std::tuple_size_v<decltype(EndpointData<V>::FlagsMeta)>>, decltype(EndpointData<V>::flags)>; // flags is a bitset type of the size of FlagsMeta
   requires std::is_member_object_pointer_v<decltype(&EndpointData<V>::flags)> == true; // flags is non-static
   requires is_std_tuple_v<decltype(EndpointData<V>::fields)>; // fields is a tuple type
   requires std::tuple_size_v<decltype(EndpointData<V>::fields)> 
               == std::tuple_size_v<decltype(EndpointData<V>::FieldNames)>;// fields size must be the same size as the FieldNames array
   requires std::is_member_object_pointer_v<decltype(&EndpointData<V>::fields)> == true; // fields is non-static
};


// Extremely helpful variant definition, for holding structs of the same enum together
template<OptionEnum E, std::size_t... I> // Large help from ChatGPT here -- unimplemented function but with return type specified....
constexpr auto make_endpoint_variant(std::index_sequence<I...>) -> std::variant<EndpointData<GetEnumerator<E>(I)>...>;


// EndpointVariant is just a std::variant of all EndpointData<enumerator>'s  for a given enum
// Scroll down for explicit specializations of EndpointData
template<OptionEnum E>
using EndpointDataVariant = decltype(make_endpoint_variant<E>(std::make_index_sequence<GetSize<E>()>{})); // Get the return type from the unimplemented function


// --------------------------------------------------------------------------------
constexpr std::string_view NOTYPE{""}; // For typeless endpoints (Main, Timestepping, preCICE, ...)

// --------------------------------------------------------------------------------
// Returns array of indices, associated fields with flags
// Index = -1 if field is always required / not associated with any flag
template<auto V>
      requires ValidEndpointData<V>
consteval std::array<int, EndpointData<V>::FieldNames.size()> GetFlagFieldMap()
{
   std::array<int, EndpointData<V>::FieldNames.size()> arr;

   std::vector<std::size_t> iFlagFields;
   for (std::size_t i = 0; i < EndpointData<V>::FlagsMeta.size(); i++)
   {
      iFlagFields.insert(iFlagFields.end(), EndpointData<V>::FlagsMeta[i].second, i);
   }

   std::size_t numAlwaysRequiredFields = arr.size() - iFlagFields.size();
   for (std::size_t i = 0; i < arr.size(); i++)
   {
      if (i < numAlwaysRequiredFields)
         arr[i] = -1;
      else
         arr[i] = iFlagFields[i-numAlwaysRequiredFields];
   }
   
   return arr;
}



} // namespace simfig