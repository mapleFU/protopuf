//   Copyright 2020 PragmaTwice
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef PROTOPUF_INT_H
#define PROTOPUF_INT_H

#include <cstdint>
#include <cstddef>
#include "coder.h"
#include "byte.h"

namespace pp {
    template <std::size_t N>
    struct sint_impl;

    template <>
    struct sint_impl<1> {
        using type = std::int8_t;
    };

    template <>
    struct sint_impl<2> {
        using type = std::int16_t;
    };

    template <>
    struct sint_impl<4> {
        using type = std::int32_t;
    };

    template <>
    struct sint_impl<8> {
        using type = std::int64_t;
    };

    // type for signed integer
    template <std::size_t N>
    using sint = typename sint_impl<N>::type;

    template <std::size_t N>
    struct uint_impl;

    template <>
    struct uint_impl<1> {
        using type = std::uint8_t;
    };

    template <>
    struct uint_impl<2> {
        using type = std::uint16_t;
    };

    template <>
    struct uint_impl<4> {
        using type = std::uint32_t;
    };

    template <>
    struct uint_impl<8> {
        using type = std::uint64_t;
    };

    // type for unsigned integer
    template <std::size_t N>
    using uint = typename uint_impl<N>::type;

    // specializing std::is_integral is not allowed
    template <typename T>
    struct is_integral : std::is_integral<T> {};

    template <typename T>
    constexpr bool is_integral_v = is_integral<T>::value;

    template <typename T>
    concept integral = is_integral_v<T>;

    template <typename T, std::size_t N>
    concept sized_integral = integral<T> && sizeof(T) == N;

    template <typename T>
    concept integral32 = sized_integral<T, 4>;

    template <typename T>
    concept integral64 = sized_integral<T, 8>;

    // direct way to implement `bytes_to_int` is:
    // 1. *(uint<N> *)bytes.data() // but it generates a UB while N > 1
    // 2. std::bit_cast(std::to_array(bytes)) // but libstdc++ has not implemented bit_cast yet
    template <std::size_t N>
    constexpr uint<N> bytes_to_int(sized_bytes<N> bytes) {
        return bytes_to_int(bytes.template subspan<0, N/2>()) | bytes_to_int(bytes.template subspan<N/2>()) << N*4;
    }
    template <>
    constexpr uint<1> bytes_to_int(sized_bytes<1> bytes) {
        return static_cast<uint<1>>(bytes.front());
    }

    // same as above
    template <std::size_t N>
    constexpr void int_to_bytes(uint<N> i, sized_bytes<N> bytes) {
        int_to_bytes<N/2>(i, bytes.template subspan<0, N/2>());
        int_to_bytes<N/2>(i >> N*4, bytes.template subspan<N/2>());
    }
    template <>
    constexpr void int_to_bytes(uint<1> i, sized_bytes<1> bytes) {
        bytes.front() = static_cast<std::byte>(i);
    }

    // NRVO is expected
    template <std::size_t N>
    constexpr auto int_to_bytes(uint<N> i) {
        std::array<std::byte, N> a;

        int_to_bytes(i, std::span(a));

        return a;
    }

    // fixed integer encoder/decoder
    template <typename>
    class integer_coder;

    template <std::unsigned_integral T>
    class integer_coder<T> {
    public:
        using value_type = T;

        integer_coder() = delete;

        static constexpr std::size_t N = sizeof(T);

        static constexpr bytes encode(T i, bytes b) {
            int_to_bytes<N>(i, b.subspan<0, N>());
            return b.subspan<N>();
        }

        static constexpr decode_result<T> decode(bytes b) {
            return {bytes_to_int<N>(b.subspan<0, N>()), b.subspan<N>()};
        }
    };

    template <std::signed_integral T>
    class integer_coder<T> {
    public:
        using value_type = T;

        integer_coder() = delete;

        static constexpr bytes encode(T i, bytes b) {
            return integer_coder<std::make_unsigned_t<T>>::encode(i, b);
        }

        static constexpr decode_result<T> decode(bytes b) {
            return integer_coder<std::make_unsigned_t<T>>::decode(b);
        }
    };

}

#endif //PROTOPUF_INT_H
