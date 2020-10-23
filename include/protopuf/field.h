#ifndef PROTOPUF_FIELD_H
#define PROTOPUF_FIELD_H

#include "int.h"
#include "float.h"
#include "array.h"
#include "zigzag.h"

namespace pp {

    template <coder T>
    constexpr uint<1> wire_type = -1;

    template <typename T>  requires integral32<T> || integral64<T>
    constexpr uint<1> wire_type<varint_coder<T>> = 0;

    template <integral64 T>
    constexpr uint<1> wire_type<integer_coder<T>> = 1;

    template <floating_point64 T>
    constexpr uint<1> wire_type<float_coder<T>> = 1;

    template <typename T, typename C>
    constexpr uint<1> wire_type<array_coder<T, C>> = 2;

    template <integral32 T>
    constexpr uint<1> wire_type<integer_coder<T>> = 5;

    template <floating_point32 T>
    constexpr uint<1> wire_type<float_coder<T>> = 5;

    enum attribute {
        singular,
        repeated
    };

    template <attribute, typename T, typename>
    struct field_container_impl {
        using type = std::optional<T>;
    };

    template <typename T, typename C>
    struct field_container_impl<repeated, T, C> {
        using type = C;
    };

    template <attribute A, typename T, std::ranges::sized_range Container = std::vector<T>>
    using field_container = typename field_container_impl<A, T, Container>::type;

    template <uint<4> N, coder C, attribute A = singular, std::ranges::sized_range Container = std::vector<typename C::value_type>>
    struct field : field_container<A, typename C::value_type, Container>{
        static constexpr uint<4> number = N;

        static constexpr uint<4> key = (N << 3u) | wire_type<C>;

        using coder = C;

        static constexpr attribute attr = A;

        using base_type = field_container<A, typename C::value_type, Container>;

        using base_type::base_type;
    };

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using integer_field = field<N, integer_coder<T>, A, Container>;

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using varint_field = field<N, varint_coder<T>, A, Container>;

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using float_field = field<N, float_coder<T>, A, Container>;

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using array_field = field<N, array_coder<T>, A, Container>;

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<std::basic_string<T>>>
    using basic_string_field = field<N, basic_string_coder<T>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<std::string>>
    using string_field = field<N, string_coder, A, Container>;

    template <uint<4> N, attribute A = singular>
    using bytes_field = field<N, bytes_coder, A>;

    template <typename>
    constexpr bool is_field = false;

    template <uint<4> N, coder C, attribute A, typename Container>
    constexpr bool is_field <field<N, C, A, Container>> = true;

    template <typename T>
    concept field_c = is_field<T>;

    struct field_not_found;

    template <uint<4>, field_c...>
    struct field_selector_impl;

    template <uint<4> I, field_c C, field_c... D>
    struct field_selector_impl<I, C, D...> {
        using type = std::conditional_t<I == C::number, C, typename field_selector_impl<I, D...>::type>;
    };

    template <uint<4> I>
    struct field_selector_impl<I> {
        using type = field_not_found;
    };

    template <uint<4> I, field_c... C>
    using field_selector = typename field_selector_impl<I, C...>::type;

    template <field_c T>
    constexpr bool empty_field(const T& v) {
        if constexpr (T::attr == singular) {
            return !v.has_value();
        } else {
            return v.empty();
        }
    };
}

#endif //PROTOPUF_FIELD_H