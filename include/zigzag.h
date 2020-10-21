#ifndef PROTOPUF_ZIGZAG_H
#define PROTOPUF_ZIGZAG_H

#include <cstddef>
#include <int.h>

namespace pp {

    // ZigZag encoding type of signed integers
    template <std::size_t N>
    class sint_zigzag {
    public:
        using underlying_type = uint<N>;

    private:
        underlying_type v;

        constexpr static uint<N> from_sint(sint<N> in) {
            return (in << 1) ^ (in >> (N * 8 - 1));
        }

        constexpr static sint<N> to_sint(uint<N> in) {
            return (in >> 1) ^ -(in & 1);
        }

    public:
        constexpr sint_zigzag() : v(0) {}

        constexpr explicit sint_zigzag(sint<N> in) : v(from_sint(in)) {}
        constexpr explicit sint_zigzag(std::span<std::byte, N> in) : v(bytes_to_int(in)) {}

        template <std::size_t M> requires (M <= N)
        constexpr sint_zigzag(const sint_zigzag<M>& i) : v(i.v) {}

        constexpr sint<N> get() const {
            return to_sint(v);
        }

        static constexpr sint_zigzag from_uint(underlying_type in) {
            sint_zigzag s;
            s.v = in;
            return s;
        }

        constexpr underlying_type get_underlying() const {
            return v;
        }

        constexpr void dump_to(std::span<std::byte, N> out) const {
            int_to_bytes(v, out);
        }

        constexpr std::array<std::byte, N> dump() const {
            return int_to_bytes(v);
        }

        template <std::size_t M> requires (M <= N)
        constexpr sint_zigzag& operator=(const sint_zigzag<M>& i) {
            v = i.v;
            return *this;
        }
    };

    template <std::size_t N>
    struct is_integral<sint_zigzag<N>> : std::true_type {};
}

#endif //PROTOPUF_ZIGZAG_H
