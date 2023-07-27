#pragma once

namespace PicoDriver {
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

}