#include "BinaryArithmetic.h"

namespace numsystem {
    namespace {
        using BNO = impl::BigNumberOperations;
        using OverflowOps = impl::OverflowAwareOps;   
        struct BinaryAccess {
            template<typename Container>
            static std::optional<typename Container::value_type> extract(const Container& data, size_t index) {
                using value_type = typename Container::value_type;
                return (index < data.size()) ? data[index] : value_type{};
            }

            template<typename Container>
            static void put(Container& data, size_t index, uint64_t value) {
                if (data.size() <= index) {
                    data.resize(index + 1);
                }
                data[index] = value;
            }
        };   

        template<typename value_type>
        impl::Storage<value_type> shift_left(const impl::Storage<value_type>& input, uint64_t shift) {
            if (input.empty() || shift == 0)
                return input;

            uint64_t word_shift = shift / input.VALUE_COUNT_BIT;
            uint64_t bit_shift = shift % input.VALUE_COUNT_BIT;

            impl::Storage<value_type> result{};
            result.resize(input.size() + word_shift, 0);

            // Сдвиг по словам
            for (size_t i = 0; i < input.size(); ++i) {
                result[i + word_shift] = input[i];
            }

            // Побитовый сдвиг с переносом
            if (bit_shift != 0) {
                value_type carry = 0;
                for (size_t i = word_shift; i < result.size(); ++i) {
                    value_type current = result[i];
                    result[i] = (current << bit_shift) | carry;
                    carry = current >> (input.VALUE_COUNT_BIT - bit_shift);
                }
                if (carry != 0) {
                    result.push_back(carry);
                }
            }

            return result;
        }
    }

    BinaryArithmetic::BinaryArithmetic(std::string_view value) {
        if (!BNO::is_integral_valid_string(value)) {
            throw std::invalid_argument("Invalid input string for integral string disability error. Value: " + std::string(value));
        }
        if (value.front() == '-') {
            _storage.sign(true);
            value.remove_prefix(1); 
        }

        // Если строка состоит только из одного нуля, то результат = 0
        if (value.size() == 1 && value.front() == '0') {
            _storage.push_back(0);
            return;
        }

        // Копируем value в std::string для безопасной модификации
        std::string _value(value);
        std::vector<value_type> storage{};
        value_type word = 0;
        int bit_index = 0;

        // Процесс преобразования числа в бинарную форму
        while (_value != "0") {
            // Остаток от деления на 2 — это младший бит
            auto [quotient, remainder] = BNO::divide_string_by_integral(_value, 2);
            std::swap(_value, quotient);
            
            word |= static_cast<value_type>(remainder) << (bit_index);
            bit_index++;

            // Как только слово заполнилось (битов >= VALUE_COUNT_BIT), добавляем его в вектор
            if (bit_index == _storage.VALUE_COUNT_BIT) {
                storage.push_back(word); // Добавляем слово в вектор
                word = 0;                // Очищаем для следующего слова
                bit_index = 0;           // Сбрасываем индекс
            }
        }

        // Если остались незаписанные биты, добавляем их в вектор
        if (bit_index != 0) {
            storage.push_back(word);
        }

        BNO::remove_zeros(_storage.data(), BNO::TrimMode::Leading);
        std::swap(_storage.data(), storage);
    }    
    int BinaryArithmetic::compare(const BinaryArithmetic& other) const noexcept {
        using value_type = BinaryArithmetic::value_type;

        // 1. Сравнение знаков
        if (sign() != other.sign()) {
            return sign() ? -1 : 1;
        }

        bool this_is_zero = is_zero();
        bool other_is_zero = other.is_zero();
        if (this_is_zero && other_is_zero) return 0;
        if (this_is_zero && !other_is_zero) return -1;
        if (!this_is_zero && other_is_zero) return 1;

        // 2. Сравнение по модулю 
        size_t max_size = std::max(_storage.size(), other._storage.size());
        for (size_t i = max_size; i-- > 0;) {
            value_type lhs_word = BinaryAccess::extract(_storage, i).value_or(0);
            value_type rhs_word = BinaryAccess::extract(other._storage, i).value_or(0);

            if (lhs_word < rhs_word) {
                return sign() ? 1 : -1;
            }
            if (lhs_word > rhs_word) {
                return sign() ? -1 : 1;
            }
        }

        // 3. Значения равны
        return 0;
    }

    BinaryArithmetic BinaryArithmetic::add(const BinaryArithmetic& other) const {
        BinaryArithmetic result{};
        value_type carry = 0;
        size_t size = std::max(_storage.size(), other._storage.size());
        result._storage.reserve(size);

        for (size_t idx = 0; idx < size; ++idx) {
            auto ai = BinaryAccess::extract(_storage, idx);
            auto bi = BinaryAccess::extract(other._storage, idx);
            BinaryAccess::put(result._storage, idx, OverflowOps::sum(ai.value_or(0), bi.value_or(0), carry));
        }

        if(carry) result._storage.push_back(1);
        result.trim_leading_zeros();
        return result;
    }
    BinaryArithmetic BinaryArithmetic::subtract(const BinaryArithmetic& other) const {
        BinaryArithmetic result{};
        value_type borrow = 0;
        size_t size = std::max(_storage.size(), other._storage.size());
        result._storage.reserve(size);

        for (size_t idx = 0; idx < size; ++idx) {
            auto ai = BinaryAccess::extract(_storage, idx);
            auto bi = BinaryAccess::extract(other._storage, idx);
            BinaryAccess::put(result._storage, idx, OverflowOps::subtract(ai.value_or(0), bi.value_or(0), borrow));
        }

        if(borrow) result._storage.push_back(1);
        result.trim_leading_zeros();
        return result;
    }
    BinaryArithmetic BinaryArithmetic::multiply(const BinaryArithmetic& other) const {
        if (is_zero() || other.is_zero()) return BinaryArithmetic(0);

        BinaryArithmetic result(0);
        for (size_t elem_idx = 0; elem_idx < other._storage.size(); ++elem_idx) {
            value_type word = other._storage[elem_idx];
            for (size_t bit_idx = 0; bit_idx < _storage.VALUE_COUNT_BIT; ++bit_idx) {
                if ((word >> bit_idx) & 1) {
                    size_t bit_pos = elem_idx * _storage.VALUE_COUNT_BIT + bit_idx;
                    BinaryArithmetic temp{};
                    temp._storage = shift_left<value_type>(_storage, bit_pos);
                    temp._storage.sign(false); // inline abs
                    result += temp;
                }
            }
        }

        // Установка знака
        result.sign(this->sign() ^ other.sign());
        result.trim_leading_zeros();
        return result;
    }
    BinaryArithmetic BinaryArithmetic::divide(const BinaryArithmetic& other) const {
        if (other.is_zero()) throw std::overflow_error("Division by zero");
        if (is_zero()) return BinaryArithmetic(0);

        using value_type = BinaryArithmetic::value_type;
        constexpr size_t BITS = Storage<value_type>::VALUE_COUNT_BIT;

        BinaryArithmetic lhs_abs = abs(*this);
        BinaryArithmetic rhs_abs = abs(other);

        BinaryArithmetic quotient(0);
        BinaryArithmetic remainder(0);

        quotient.sign(sign() != other.sign());

        const auto& data = lhs_abs._storage.data();

        // Подсчёт старшего бита lhs_abs
        int64_t total_bits = 0;
        for (int64_t i = static_cast<int64_t>(data.size()) - 1; i >= 0; --i) {
            if (data[i] != 0) {
                for (int64_t bit = static_cast<int64_t>(BITS) - 1; bit >= 0; --bit) {
                    if ((data[i] >> bit) & 1) {
                        total_bits = i * static_cast<int64_t>(BITS) + bit + 1;
                        break;
                    }
                }
                if (total_bits > 0) break;
            }
        }

        for (int64_t i = total_bits - 1; i >= 0; --i) {
            remainder._storage = shift_left<value_type>(remainder._storage, 1);

            size_t elem_idx = i / BITS;
            size_t bit_idx = i % BITS;
            if (elem_idx < data.size() && ((data[elem_idx] >> bit_idx) & 1)) {
                remainder += 1;
            }

            if (remainder >= rhs_abs) {
                remainder -= rhs_abs;

                size_t q_elem = i / BITS;
                size_t q_bit = i % BITS;
                if (q_elem >= quotient._storage.size())
                    quotient._storage.resize(q_elem + 1, 0);
                quotient._storage[q_elem] |= (static_cast<value_type>(1) << q_bit);
            }
        }

        quotient.trim_leading_zeros();
        if (quotient.is_zero()) {
            quotient._storage.push_back(0);
        }
        return quotient;
    }
    BinaryArithmetic BinaryArithmetic::modulo(const BinaryArithmetic& other) const {
        if (other.is_zero()) throw std::overflow_error("Division by zero");
        if (is_zero()) return BinaryArithmetic(0);

        BinaryArithmetic quotient = divide(other);
        BinaryArithmetic remainder = *this - (quotient * other);

        // Остаток всегда имеет знак lhs
        remainder.sign(this->sign());
        remainder.trim_leading_zeros();
        return remainder;
    }

    [[nodiscard]] bool BinaryArithmetic::is_zero() const noexcept {
        for (auto w : _storage) {
            if (w != 0) return false;
        }
        return true;
    }
    void BinaryArithmetic::trim_leading_zeros() noexcept {
        BNO::remove_zeros(_storage.data(), BNO::TrimMode::Trailing);
        if (_storage.back() == 0) { // меняем знак если только число стало 0
            _storage.sign(false);
        }
    }    
    std::string to_string(const BinaryArithmetic& other) {
        const auto& refdata = other._storage;
        if (refdata.empty()) return "0";

        // данных меньше чем 2^64 используем стандартный std::to_string
        // BITS_PER_ELEMENT<uint64_t> / BITS_PER_ELEMENT<value_type>
        if (refdata.size() <= (std::numeric_limits<uint64_t>::digits / refdata.VALUE_COUNT_BIT)) {
            return (refdata.sign() ? "-" : "") + std::to_string(static_cast<uint64_t>(other));
        }

        auto multiply_vector_by_2 = [](std::vector<uint32_t>& digits) {
            uint64_t carry = 0;
            constexpr uint64_t base = 1'000'000'000ULL;
            for (auto& d : digits) {
                uint64_t val = static_cast<uint64_t>(d) * 2 + carry;
                d = static_cast<uint32_t>(val % base);
                carry = val / base;
            }
            if (carry) digits.push_back(static_cast<uint32_t>(carry));
            };
        auto add_bit_to_vector = [](std::vector<uint32_t>& digits, int bit) {
            if (bit != 1) return;

            uint64_t carry = 1;
            constexpr uint64_t base = 1'000'000'000ULL;
            for (auto& d : digits) {
                uint64_t val = static_cast<uint64_t>(d) + carry;
                d = static_cast<uint32_t>(val % base);
                carry = val / base;
                if (carry == 0) break;
            }
            if (carry) digits.push_back(static_cast<uint32_t>(carry));
            };
        auto digits_to_string = [](const std::vector<uint32_t>& digits, bool sign) ->std::string {
            if (digits.empty()) return "0";
            std::string result = std::to_string(digits.back());  // старший элемент
            for (int i = (int)digits.size() - 2; i >= 0; --i) {
                std::string part = std::to_string(digits[i]);
                // добавляем ведущие нули, чтобы каждый элемент был длиной 9 символов
                result += std::string(9 - part.length(), '0') + part;
            }

            // Use is_negative instead of _storage.sign()
            if (sign && result != "0") {
                result.insert(result.begin(), '-');
            }

            return result;
            };

        std::vector<uint32_t> digits = { 0 };
        for (auto it = refdata.rbegin(); it != refdata.rend(); ++it) {
            uint64_t current_word = *it;
            for (int i = refdata.VALUE_COUNT_BIT - 1; i >= 0; --i) {
                int bit = (current_word >> i) & 1;
                // Using the refactored static methods
                multiply_vector_by_2(digits);
                add_bit_to_vector(digits, bit);
            }
        }

        return digits_to_string(digits, refdata.sign());
    }
}