#pragma once

#include <tuple>

namespace PicoDriver {
    namespace TypeUtils {

        namespace Detail {
            template<size_t ... Coordinates> 
            struct Coordinate {
                std::array<size_t, sizeof...(Coordinates)> values { Coordinates ...};
            };

            template<typename CoordinateType, typename ... Arguments>
            struct CountTypes;

            template<size_t Index, size_t SecondaryIndex, typename ... Arguments>
            struct CountTypes<Coordinate<Index, SecondaryIndex>, Arguments ...> {
                using PrimaryType = std::tuple_element_t<Index, std::tuple<Arguments ...>>;
                using SecondaryType = std::tuple_element_t<SecondaryIndex, std::tuple<Arguments ...>>;

                static constexpr size_t FoundType = std::is_same_v<PrimaryType, SecondaryType> 
                                                    + CountTypes<Coordinate<Index, SecondaryIndex - 1>, Arguments ...>::FoundType;
            };

            template<size_t PrimaryIndex, typename ... Arguments>
            struct CountTypes<Coordinate<PrimaryIndex, 0>, Arguments ...> {
                using PrimaryType = std::tuple_element_t<PrimaryIndex, std::tuple<Arguments ...>>;
                using SecondaryType = std::tuple_element_t<0, std::tuple<Arguments ...>>;
                static constexpr size_t FoundType = std::is_same_v<PrimaryType, SecondaryType> 
                                                    + CountTypes<Coordinate<PrimaryIndex - 1, sizeof...(Arguments) - 1>, Arguments ...>::FoundType;
            };

            template<size_t SecondaryIndex, typename ... Arguments>
            struct CountTypes<Coordinate<0, SecondaryIndex>, Arguments ...> {
                using PrimaryType = std::tuple_element_t<0, std::tuple<Arguments ...>>;
                using SecondaryType = std::tuple_element_t<SecondaryIndex, std::tuple<Arguments ...>>;
                static constexpr size_t FoundType = std::is_same_v<PrimaryType, SecondaryType> 
                                                    + CountTypes<Coordinate<0, SecondaryIndex - 1>, Arguments ...>::FoundType;
            };

            template<typename ... Arguments>
            struct CountTypes<Coordinate<0, 0>, Arguments ...> {
                static constexpr size_t FoundType = 1;
            };

        }

        template<typename ... Arguments>
        static inline constexpr bool CountTypes = Detail::CountTypes<Detail::Coordinate<sizeof...(Arguments) - 1, sizeof...(Arguments) - 1>, Arguments ...>::FoundType == sizeof...(Arguments);

        template<typename ... Arguments>
        static inline constexpr bool IsTypeSet = ::PicoDriver::TypeUtils::CountTypes<Arguments ...>;

        template<size_t Index, size_t Len, typename ... Args>
        struct ConstexprFor {
            template<typename Callable>
            static auto call(Callable callable, std::tuple<Args ...> &args) {
                using ReturnType = decltype(ConstexprFor<Index - 1, Len, Args ...>::call(callable, args));
                using IndexType = std::integral_constant<uint16_t, Index>;
                if constexpr (std::is_same_v<ReturnType, void>) {
                    return callable(IndexType{}, std::get<Index>(args)) & ConstexprFor<Index - 1, Len, Args ...>::call(callable, args);
                } else {
                    return callable(IndexType{}, std::get<Index>(args));
                }
            }
        };

        template<size_t Len, typename ... Args>
        struct ConstexprFor<0, Len, Args ...> {
            template<typename Callable>
            static auto call(Callable callable, std::tuple<Args ...> &args) {
                using IndexType = std::integral_constant<uint16_t, 0>;
                return callable(IndexType{}, std::get<0>(args));
            }
        };

    }
}