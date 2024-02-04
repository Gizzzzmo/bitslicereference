#include <iostream>
#include <cassert>
#include "bitsliceref.hpp"

template <uint8_t n, typename T>
struct Regs {
    T memory[n] = {};
};

template <typename T>
T hash64shift(T keyy) {
    T key = static_cast<uint64_t>(keyy);
    key = (~key) + (key << 21); // key = (key << 21) - key - 1;
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return static_cast<T>(key);
}

template<uint8_t start_bit, uint8_t end_bit, typename T>
void test_single() {
    using BSlice = BitSliceRef<start_bit, end_bit, 1, T>;
    using alias_t = BSlice::alias_t;
    static constexpr alias_t a1 = BSlice::Helper::a1;
    static constexpr alias_t a0 = BSlice::Helper::a0;
    static constexpr T u1 = BSlice::Helper::u1;
    static constexpr T u0 = BSlice::Helper::u0;
    T x = 0;
    //static_assert(std::equal<T, BSlice::underlying_t>::value);
    BSlice ref(x);

    alias_t mask = ((BSlice::Helper::bitsize == BSlice::Helper::bitsize_a) ? a0 : a1<<BSlice::Helper::bitsize) - a1;
    T lefted = (end_bit == BSlice::Helper::bitsize_u) ? u0 : u1<<end_bit;
    alias_t value = hash64shift(a0);
    for(int i = 0; i < 10000; i++) {
        ref = value;
        //std::cout << "   " << std::bitset<BSlice::Helper::bitsize>(value) << std::endl;
        assert((value & mask) == static_cast<alias_t>(ref));
        assert((x & ~(lefted - u1<<start_bit)) == 0);
        value = hash64shift(value);
    }
}

template<uint8_t start_bit, uint8_t end_bit, typename T>
void testDouble() {
    using BSlice = BitSliceRef<start_bit, end_bit, 2, T>;
    using alias_t = BSlice::alias_t;
    static constexpr alias_t a1 = BSlice::Helper::a1;
    static constexpr alias_t a0 = BSlice::Helper::a0;
    static constexpr T u1 = BSlice::Helper::u1;
    static constexpr T u0 = BSlice::Helper::u0;
    T x1 = 0;
    T x2 = 0;
    BSlice ref(x1, x2);

}

template<std::size_t N>
struct num { static const constexpr auto value = N; };

template <class F, std::size_t... Is>
void for_(F func, std::index_sequence<Is...>)
{
  (func(num<Is>{}), ...);
}

template <std::size_t N, typename F>
void for_(F func)
{
  for_(func, std::make_index_sequence<N>());
}

template<typename T> 
void test_all_single(){
    using BSlice = BitSliceRef<0, 1, 1, T>;
    static_assert(std::is_same<BSlice, BitSliceRefSingle<0, 1, T>>::value);
    for_<BSlice::Helper::bitsize_u>([&] (auto i) {
        for_<BSlice::Helper::bitsize_u-i.value + 1>([&i] (auto j) {
            test_single<i.value, i.value + j.value, T>();
        });
    });
}

int main() {

    test_all_single<uint8_t>();
    test_all_single<uint16_t>();
    test_all_single<uint32_t>();
    test_all_single<uint64_t>();


    //Regs r;
    //std::cout << std::bitset<16>(r.memory[0]) << " " << (int)static_cast<uint8_t>(r.PREV()) << " " << (int)static_cast<uint8_t>(r.NEXT()) << std::endl;
    //r.PREV() = 4;
    //r.NEXT() = 255;
    //std::cout << std::bitset<16>(r.memory[0]) << " " << (int)static_cast<uint8_t>(r.PREV()) << " " << (int)static_cast<uint8_t>(r.NEXT()) << std::endl;
//
    //uint16_t x = r.PREV() + r.NEXT();
//
    //std::cout << r.ID() << " " << static_cast<uint32_t>(r.ID()) << std::endl;
    //r.ID() = 1<<26;
    //std::cout << r.ID() << " " << static_cast<uint32_t>(r.ID()) << std::endl;

}
