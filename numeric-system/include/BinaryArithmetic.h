#pragma once
#include "Internal.h"

namespace numsystem {

    class BinaryArithmetic : public IntegralBase<BinaryArithmetic> {
    public:
        // --- Конструкторы ---
        constexpr BinaryArithmetic() : _storage() {}
        BinaryArithmetic(const char* value) : BinaryArithmetic(std::string_view(value)) {}
        BinaryArithmetic(std::string_view value);

        // --- Неявный конструктор из целого (для операций +int и т.д.) ---
        template<typename _Ty, typename = std::enable_if_t<std::is_integral<_Ty>::value && !std::is_same_v<_Ty, bool>>>
        constexpr BinaryArithmetic(_Ty value) noexcept {
            sign((value < 0));
            if (value == 0) {
                // Явно гарантируем, что sign = false для нуля:
                sign(false);
                _storage.push_back(0);
                return;
            }

            _storage.clear();
            using UnsignedTy = std::make_unsigned_t<_Ty>;
            UnsignedTy abs_value = static_cast<UnsignedTy>(_storage.sign() ? -value : value);

            while (abs_value != 0) {
                value_type raw_word = static_cast<value_type>(abs_value & _storage.MAX_VALUE);
                _storage.push_back(raw_word);
                abs_value >>= _storage.VALUE_COUNT_BIT;
            }

            // Убираем любые ведущие нули (и, если все слова были нулями,
            // trim_leading_zeros() сам установит {0} + sign(false))
            trim_leading_zeros();
        }

        // --- Неявное приведение к integral (если нужно) ---
        template<typename _Ty, typename = std::enable_if_t<std::is_integral<_Ty>::value && !std::is_same_v<_Ty, bool>>>
        [[nodiscard]] explicit operator _Ty() const {
            // Сначала проверяем, действительно ли это «0»:
            if (is_zero()) {
                return _Ty(0);
            }

            // Максимальное количество бит в _Ty
            // Проверка, что число не занимает больше бит, чем _Ty
            const size_t max_bits = sizeof(_Ty) * 8;
            size_t total_bits = _storage.size() * _storage.VALUE_COUNT_BIT;
            if (total_bits > max_bits) {
                throw std::overflow_error("Value exceeds the bit width of the target integral type");
            }

            // Общий случай: собираем результат из слов
            uint64_t result = 0;
            size_t shift = 0;

            for (size_t i = 0; i < _storage.size() && shift < max_bits; ++i) {
                result |= static_cast<uint64_t>(_storage[i]) << shift;
                shift += _storage.VALUE_COUNT_BIT;
            }

            if constexpr (std::is_unsigned_v<_Ty>) {
                return static_cast<_Ty>(result);
            }
            else {
                using Signed64 = std::make_signed_t<decltype(result)>;
                Signed64 signed_val = static_cast<Signed64>(result);
                if (_storage.sign()) signed_val = -signed_val;
                return static_cast<_Ty>(signed_val);
            }
        }

        template<typename _Ty, typename = std::enable_if_t<std::is_integral<_Ty>::value>>
        BinaryArithmetic& operator=(_Ty value) {
            *this = BinaryArithmetic(value);
            return *this;
        }

        [[nodiscard]] int compare(const BinaryArithmetic& other) const noexcept;
        [[nodiscard]] BinaryArithmetic add(const BinaryArithmetic& other) const;
        [[nodiscard]] BinaryArithmetic divide(const BinaryArithmetic& other) const;
        [[nodiscard]] BinaryArithmetic modulo(const BinaryArithmetic& other) const;
        [[nodiscard]] BinaryArithmetic subtract(const BinaryArithmetic& other) const;
        [[nodiscard]] BinaryArithmetic multiply(const BinaryArithmetic& other) const;


        inline void sign(bool s) noexcept { _storage.sign(s); }
        [[nodiscard]] inline bool sign() const noexcept { return _storage.sign(); }
        [[nodiscard]] inline explicit operator bool() const noexcept { return !this->is_zero(); }
        friend std::string to_string(const BinaryArithmetic& other);
    private:
        using value_type = uint8_t;
        impl::Storage<value_type> _storage;
        bool is_zero() const noexcept;           // тут просто проверка «весь вектор == {0}»
        void trim_leading_zeros() noexcept;      // тут реальное pop_back, убирающее лишние нули
    };



}