#include <type_traits>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <functional>

template<uint8_t n>
using upper_bound = 
    typename std::conditional<(n <= 8), uint8_t,
        typename std::conditional<(n <= 16), uint16_t, 
            typename std::conditional<(n <= 32), uint32_t,
                uint64_t
            >::type
        >::type
    >::type;

template<uint8_t start_bit, uint8_t end_bit, uint8_t n, typename underlying_t>
class BitSliceRefCommon {
    public:
        static_assert(sizeof(underlying_t) <= 8);

        static constexpr uint8_t bitsize_u = 8*sizeof(underlying_t);
        static constexpr uint8_t bitsize = end_bit - start_bit + (n-1)*bitsize_u;
        
        using alias_t = upper_bound<bitsize>;

        static constexpr uint8_t bitsize_a = 8*sizeof(alias_t);

        static_assert(start_bit < bitsize_u && end_bit <= bitsize_u);
        static_assert(bitsize_a >= bitsize);

        static constexpr underlying_t u1 = static_cast<underlying_t>(1);
        static constexpr underlying_t u0 = static_cast<underlying_t>(0);

        static constexpr alias_t a1 = static_cast<alias_t>(1);
        static constexpr alias_t a0 = static_cast<alias_t>(0);

        static constexpr underlying_t leftshifted_by_endbit = (end_bit == bitsize_u) ? u0 : (u1<<end_bit);
};

template<uint8_t start_bit, uint8_t end_bit, typename underlying_t>
class BitSliceRefSingle {
    public:
        using Self = BitSliceRefSingle<start_bit, end_bit, underlying_t>;
        using Helper = BitSliceRefCommon<start_bit, end_bit, 1, underlying_t>;
        using alias_t = typename Helper::alias_t;


        BitSliceRefSingle(underlying_t& arg) : underlying_field(arg) {}

        Self& operator=(alias_t value) {
            underlying_field &= ~mask;
            underlying_field |= (static_cast<underlying_t>(value)<<start_bit) & mask;
            return *this;
        }

        operator alias_t() const {
            return static_cast<alias_t>( (mask & underlying_field)>>start_bit );
        }

        template<uint8_t _start_bit, uint8_t _end_bit, typename _underlying_t>
        friend std::ostream& operator<<(std::ostream& stream, const BitSliceRefSingle<_start_bit, _end_bit, _underlying_t>& bsr);
    private:
        underlying_t& underlying_field;

        static constexpr underlying_t mask = Helper::leftshifted_by_endbit - (Helper::u1<<start_bit);
};

template<uint8_t start_bit, uint8_t end_bit, uint8_t n, typename underlying_t> //requires (n > 1)
class BitSliceRefMultiple {
    public:
        using Self = BitSliceRefMultiple<start_bit, end_bit, n, underlying_t>;
        using Helper = BitSliceRefCommon<start_bit, end_bit, n, underlying_t>;
        using alias_t = typename Helper::alias_t;

        template<typename... Args>
        BitSliceRefMultiple(Args&&... args) : underlying_fields{std::forward<Args>(args)...} { }

        Self& operator=(alias_t value) {
            underlying_fields[0].get() &= ~mask_start;
            underlying_fields[0].get() |= (static_cast<underlying_t>(value)<<start_bit);

            for(uint8_t i = 1; i < n-1; i++) {
                underlying_fields[i].get() = static_cast<underlying_t>( value>>(i*Helper::bitsize_u - start_bit) );
            }

            underlying_fields[n-1].get() &= ~mask_end;
            underlying_fields[n-1].get() |= static_cast<underlying_t>( value>>((n-1)*Helper::bitsize_u - start_bit) ) & mask_end;
            return *this;
        }

        operator alias_t() const {
            alias_t value = static_cast<alias_t>( (mask_start & underlying_fields[0].get())>>start_bit );
            
            for(uint8_t i = 1; i < n-1; i++) {
                value |= static_cast<alias_t>(underlying_fields[i].get())<<(i*Helper::bitsize_u - start_bit);
            }

            value |= static_cast<alias_t>(underlying_fields[n-1].get() & mask_end)<<((n-1)*Helper::bitsize_u - start_bit);
            return value;
        }

        template<uint8_t _start_bit, uint8_t _end_bit, uint8_t _n, typename _underlying_t>
        friend std::ostream& operator<<(std::ostream& stream, const BitSliceRefMultiple<_start_bit, _end_bit, _n, _underlying_t>& bsr);
    private:
        std::reference_wrapper<underlying_t> underlying_fields[n];

        static constexpr underlying_t mask_start = Helper::u0 - (Helper::u1<<start_bit);
        static constexpr underlying_t mask_end = Helper::leftshifted_by_endbit - Helper::u1;
};

template<uint8_t start_bit, uint8_t end_bit, uint8_t n, typename underlying_t>
using BitSliceRef = typename std::conditional<n == 1,
    BitSliceRefSingle<start_bit, end_bit, underlying_t>,
    BitSliceRefMultiple<start_bit, end_bit, n, underlying_t>
>::type;

//template <uint8_t start_bit, uint8_t end_bit, typename underlying_t>
//BitSliceRef<start_bit, end_bit, 1, underlying_t>& BitSliceRef<start_bit, end_bit, 1, underlying_t>::operator=(upper_bound<end_bit-start_bit> value) {
//    underlying_fields[0].get() &= ~mask_single;
//    underlying_fields[0].get() |= (static_cast<underlying_t>(value)<<start_bit) & mask_single;
//    return *this;
//}
//
//template <uint8_t a, uint8_t b>
//concept GT = a > b;
//
//template <uint8_t start_bit, uint8_t end_bit, uint8_t n, typename underlying_t> requires GT<n, 1>
//Self& operator=(alias_t value) {
//    underlying_fields[0].get() &= ~mask_start;
//    underlying_fields[0].get() |= (static_cast<underlying_t>(value)<<start_bit);
//
//    for(uint8_t i = 1; i < n-1; i++) {
//        underlying_fields[i].get() = static_cast<underlying_t>( value>>(i*bitsize_u - start_bit) );
//    }
//
//    underlying_fields[n-1].get() &= ~mask_end;
//    underlying_fields[n-1].get() |= static_cast<underlying_t>( value>>((n-1)*bitsize_u - start_bit) ) & mask_end;
//    return *this;
//}

template<uint8_t start_bit, uint8_t end_bit, uint8_t n, typename underlying_t>
std::ostream& operator<<(std::ostream& stream, const BitSliceRef<start_bit, end_bit, n, underlying_t>& bsr) {
    using BSlice = BitSliceRef<start_bit, end_bit, n, underlying_t>;
    for(underlying_t field : bsr.underlying_fields) {
        std::cout << std::bitset<BSlice::bitsize_u>(field) << " ";
    }
    return stream;
}


        
        