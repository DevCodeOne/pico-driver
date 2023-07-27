#pragma once

#include <tuple>

namespace PicoDriver {
    namespace TypeUtils {

        template<size_t Index, typename Head, typename ... Rest>
        struct TypeAt {
            using type = typename TypeAt<Index - 1, Rest ...>::type;
        };

        template<typename Head, typename ... Rest>
        struct TypeAt<0, Head, Rest ...> {
            using type = Head;
        };

        template<size_t Index, typename ... TypeList>
        using TypeAt_t = typename TypeAt<Index, TypeList ...>::type;

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