#pragma once

#include <string>
#include <vector>
#include <limits>
#include <cstdint>
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <optional>

namespace numsystem {
    namespace impl {
        class StateInfo {
        private:
            static constexpr uint64_t SIGN_MASK = 1ULL << 63;
            static constexpr uint64_t VALUE_MASK = ~SIGN_MASK;

            uint64_t data_ = 0;
        public:
            constexpr StateInfo() = default;
            constexpr StateInfo(uint64_t value, bool sign)
                : data_{ (sign ? SIGN_MASK : 0) | (value & VALUE_MASK) } {
            }
            [[nodiscard]] constexpr uint64_t value() const noexcept {
                return data_ & VALUE_MASK;
            }
            constexpr void value(uint64_t value) noexcept {
                data_ = (data_ & SIGN_MASK) | (value & VALUE_MASK);
            }
            [[nodiscard]] constexpr bool sign() const noexcept {
                return (data_ & SIGN_MASK) != 0;
            }
            constexpr void sign(bool negative) noexcept {
                if (negative) data_ |= SIGN_MASK;
                else          data_ &= ~SIGN_MASK;
            }
        };

        template <typename T>
        class Storage {
            static_assert(std::is_integral_v<T>, "T must be an integral type");
        public:
            using value_type = std::make_unsigned_t<T>;
            static constexpr size_t VALUE_SIZE = sizeof(value_type);
            static constexpr value_type MAX_VALUE = std::numeric_limits<value_type>::max();
            static constexpr value_type MIN_VALUE = std::numeric_limits<value_type>::min();
            static constexpr int VALUE_COUNT_BIT = std::numeric_limits<value_type>::digits;
        private:
            StateInfo state_;
            std::vector<value_type> data_;
        public:
            constexpr Storage() : state_({}), data_(0) {};
   
            std::vector<value_type>& data() { return data_; }
            const std::vector<value_type>& data() const { return data_; }

            // State access
            [[nodiscard]] constexpr uint64_t value() const noexcept { return state_.value(); }
            constexpr void value(uint64_t val) noexcept { state_.value(val); }

            [[nodiscard]] constexpr bool sign() const noexcept { return state_.sign(); }
            constexpr void sign(bool s) noexcept { state_.sign(s); }

            // Data operations
            void clear() noexcept { data_.clear(); }

            [[nodiscard]] size_t size() const noexcept { return data_.size(); }
            [[nodiscard]] bool empty() const noexcept { return data_.empty(); }

            void push_back(value_type val) { data_.push_back(val); }
            void pop_back() { data_.pop_back(); }

            [[nodiscard]] value_type& back() { return data_.back(); }
            [[nodiscard]] const value_type& back() const { return data_.back(); }

            [[nodiscard]] value_type& operator[](size_t index) { return data_[index]; }
            [[nodiscard]] const value_type& operator[](size_t index) const { return data_[index]; }

            void reserve(size_t capacity) { data_.reserve(capacity); }
            void resize(size_t new_size, value_type val = 0) { data_.resize(new_size, val); }

            // Iterators
            [[nodiscard]] auto begin() noexcept { return data_.begin(); }
            [[nodiscard]] auto end() noexcept { return data_.end(); }
            [[nodiscard]] auto begin() const noexcept { return data_.begin(); }
            [[nodiscard]] auto end() const noexcept { return data_.end(); }
            [[nodiscard]] auto cbegin() const noexcept { return data_.cbegin(); }
            [[nodiscard]] auto cend() const noexcept { return data_.cend(); }

            [[nodiscard]] auto rbegin() noexcept { return data_.rbegin(); }
            [[nodiscard]] auto rend() noexcept { return data_.rend(); }
            [[nodiscard]] auto rbegin() const noexcept { return data_.rbegin(); }
            [[nodiscard]] auto rend() const noexcept { return data_.rend(); }
            [[nodiscard]] auto crbegin() const noexcept { return data_.crbegin(); }
            [[nodiscard]] auto crend() const noexcept { return data_.crend(); }
        };

        struct OverflowAwareOps {
            template<typename _Ty, typename = std::enable_if_t<std::is_unsigned<_Ty>::value>>
            static constexpr _Ty sum(_Ty ai, _Ty bi, _Ty& carry) noexcept {
                _Ty result = ai + bi + carry;
                carry = (result < ai || result < bi || (carry && result == ai)) ? 1 : 0;
                return result;
            }

            template<typename _Ty, typename = std::enable_if_t<std::is_unsigned<_Ty>::value>>
            static constexpr _Ty subtract(_Ty ai, _Ty bi, _Ty& borrow) noexcept {
                _Ty result = ai - bi - borrow;
                borrow = (ai < bi + borrow) ? 1 : 0;
                return result;
            }         
        };

        struct BigNumberOperations {
            static constexpr char ZERO = '0'; // Более явное название
            enum class TrimMode { Leading, Trailing };
            static constexpr bool is_integral_valid_string(std::string_view str) noexcept {
                if (str.empty()) {
                    return false;
                }

                size_t start_idx = 0;
                if (str[0] == '-') {
                    if (str.length() == 1) { // Только минус не является числом
                        return false;
                    }
                    start_idx = 1;
                }

                // Проверяем на ведущие нули (кроме случая "0")
                if (str.length() > start_idx + 1 && str[start_idx] == ZERO) {
                    // Если строка типа "0123" или "-0123"
                    return false;
                }

                for (size_t i = start_idx; i < str.length(); ++i) {
                    if (str[i] < ZERO || str[i] > '9') {
                        return false;
                    }
                }
                return true;
            }
            static constexpr bool greater_or_equal(std::string_view a, std::string_view b) noexcept {
                if (a.length() != b.length()) {
                    return a.length() > b.length();
                }
                return a.compare(b) >= 0; // std::string_view::compare более явно
            }
 
            template<typename Container>
            static inline void remove_zeros(Container& c, TrimMode mode) {
                if (c.empty()) return;

                using ValueType = typename Container::value_type;
                const ValueType zero = [] {
                    if constexpr (std::is_same_v<ValueType, char>)
                        return BigNumberOperations::ZERO;  // символ '0'
                    else
                        return ValueType(0);               // число 0
                    }();

                const auto is_not_zero = [zero](ValueType ch) { return ch != zero; };

                if (mode == TrimMode::Leading) {
                    auto first_non_zero = std::find_if(c.begin(), c.end(), is_not_zero);
                    c.erase(c.begin(), first_non_zero);
                }
                else { // TrimMode::Trailing
                    while (!c.empty() && c.back() == zero) {
                        c.pop_back();
                    }
                }

                if (c.empty()) {
                    c.push_back(zero);
                }
            }
            static std::pair<std::string, uint64_t> divide_string_by_integral(std::string_view str, uint64_t divisor) {
                if (divisor == 0) {
                    throw std::runtime_error("Division by zero.");
                }
                if (str.empty() || (str.length() == 1 && str[0] == ZERO)) {
                    return { "0", 0 };
                }

                uint64_t remainder = 0;
                std::string quotient;
                quotient.reserve(str.length());

                for (char ch : str) {
                    uint64_t accumulated = remainder * 10 + static_cast<uint64_t>(ch - ZERO);
                    quotient.push_back(static_cast<char>((accumulated / divisor) + ZERO));
                    remainder = accumulated % divisor;
                }

                remove_zeros(quotient, TrimMode::Leading);
                return { quotient, remainder };
            }
     
            static std::string add_strings(std::string_view a, std::string_view b) {
                // Убедимся, что 'a' всегда больше или равно 'b' по длине для упрощения логики
                if (a.length() < b.length()) {
                    std::swap(a, b);
                }

                std::string result_str;
                result_str.reserve(a.length() + 1); // Максимально возможная длина

                int carry = 0;
                int i = static_cast<int>(a.length()) - 1;
                int j = static_cast<int>(b.length()) - 1;

                while (i >= 0 || carry) { // Условие изменено для корректной обработки последнего переноса
                    int digit_a = (i >= 0) ? (a[i] - ZERO) : 0;
                    int digit_b = (j >= 0) ? (b[j] - ZERO) : 0;

                    int sum = digit_a + digit_b + carry;
                    result_str.push_back(static_cast<char>((sum % 10) + ZERO));
                    carry = sum / 10;

                    --i;
                    --j;
                }

                std::reverse(result_str.begin(), result_str.end());
                remove_zeros(result_str, TrimMode::Leading);
                return result_str;
            }
            static std::string subtract_strings(std::string_view a, std::string_view b) {
                if (b == "0") return std::string(a);
                if (a == b) return "0"; // Если числа равны, результат 0

                // Если a < b, это приведет к отрицательному числу.
                // Здесь можно либо выбросить исключение, либо реализовать логику для отрицательных чисел.
                // Для простоты, предполагаем a >= b.
                if (!greater_or_equal(a, b)) {
                    // Можно выбросить исключение или вернуть специальное значение,
                    // например, "Ошибка: уменьшаемое меньше вычитаемого".
                    throw std::runtime_error("Subtraction of a larger number from a smaller one is not supported for positive results.");
                }

                std::string result_str;
                result_str.reserve(a.length());

                int borrow = 0;
                int i = static_cast<int>(a.length()) - 1;
                int j = static_cast<int>(b.length()) - 1;

                while (i >= 0) {
                    int digit_a = a[i] - ZERO;
                    int digit_b = (j >= 0) ? (b[j] - ZERO) : 0;

                    int diff = digit_a - digit_b - borrow;
                    if (diff < 0) {
                        diff += 10;
                        borrow = 1;
                    }
                    else {
                        borrow = 0;
                    }
                    result_str.push_back(static_cast<char>(diff + ZERO));
                    --i;
                    --j;
                }

                std::reverse(result_str.begin(), result_str.end());
                remove_zeros(result_str, TrimMode::Leading);
                return result_str;
            }
            static std::string multiply_strings(std::string_view a, std::string_view b) {
                if (a == "0" || b == "0") {
                    return "0";
                }

                // Вектор для хранения промежуточных результатов. Размер a.length() + b.length().
                // Инициализируем нулями.
                std::vector<int> result_digits(a.length() + b.length(), 0);

                // Умножаем поочередно каждую цифру.
                for (int i = static_cast<int>(a.length()) - 1; i >= 0; --i) {
                    int digit_a = a[i] - ZERO;
                    for (int j = static_cast<int>(b.length()) - 1; j >= 0; --j) {
                        int digit_b = b[j] - ZERO;

                        int product = digit_a * digit_b;
                        int sum = product + result_digits[i + j + 1];

                        result_digits[i + j + 1] = sum % 10;
                        result_digits[i + j] += sum / 10; // Добавляем перенос к следующей позиции
                    }
                }

                std::string res;
                res.reserve(result_digits.size());
                for (int digit : result_digits) {
                    res.push_back(static_cast<char>(digit + ZERO));
                }

                remove_zeros(res, TrimMode::Leading);
                return res;
            }
            static std::pair<std::string, std::string> divide_strings(std::string_view a, std::string_view b) {
                if (b == "0") {
                    throw std::overflow_error("Division by zero.");
                }
                if (a == "0") {
                    return { "0", "0" };
                }
                if (!greater_or_equal(a, b)) {
                    return { "0", std::string(a) }; // Частное 0, остаток 'a'
                }

                std::string quotient_str;
                quotient_str.reserve(a.length());
                std::string current_remainder_str = ""; // Текущий остаток

                for (char digit_char : a) {
                    current_remainder_str.push_back(digit_char);
                    remove_zeros(current_remainder_str, TrimMode::Leading); // Удаляем ведущие нули, если они появились

                    int count = 0;
                    // Пока текущий остаток больше или равен делителю, вычитаем и увеличиваем счетчик
                    while (greater_or_equal(current_remainder_str, b)) {
                        current_remainder_str = subtract_strings(current_remainder_str, b);
                        ++count;
                    }
                    quotient_str.push_back(static_cast<char>(count + ZERO));
                }
                remove_zeros(quotient_str, TrimMode::Leading);
                remove_zeros(current_remainder_str, TrimMode::Leading);
         
                return { quotient_str, current_remainder_str };
            }
        };
    }


    // Базовый CRTP-класс для сравнения через единый метод compare()
    // Метод compare(const Derived&) должен быть определён в классе-наследнике и возвращать:
    //   - отрицательное значение (< 0), если *this < other
    //   - ноль (0), если *this == other
    //   - положительное значение (> 0), если *this > other
    template <typename Derived>
    struct BaseComparable {
        friend constexpr bool operator<(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) < 0;
        }
        friend constexpr bool operator>(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) > 0;
        }
        friend constexpr bool operator<=(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) <= 0;
        }
        friend constexpr bool operator>=(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) >= 0;
        }
        friend constexpr bool operator==(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) == 0;
        }
        friend constexpr bool operator!=(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) != 0;
        }
    };


    // Базовый CRTP-класс для арифметики.
    // Класс-наследник (Derived) должен обеспечить реализацию методов:
    //   - Derived add(const Derived&) const;
    //   - Derived subtract(const Derived&) const;
    //   - Derived multiply(const Derived&) const;
    //   - Derived divide(const Derived&) const;
    //   - Derived modulo(const Derived&) const;
    // Все эти методы возвращают новый объект-результат, а не изменяют *this.
    template <typename Derived>
    struct BaseArithmetic {
        friend constexpr Derived operator+(const Derived& lhs, const Derived& rhs) {
            if (lhs.sign() == rhs.sign()) {
                // a + b | (-a) + (-b)
                Derived result = lhs.add(rhs);
                result.sign(lhs.sign());
                return result;
            }
            else if (abs(lhs) == abs(rhs)) {
                return 0;
            }
            else {
                const bool lhsAbsGe = abs(lhs) > abs(rhs);
                Derived larger = lhsAbsGe ? lhs : rhs;
                Derived smaller = lhsAbsGe ? rhs : lhs;
                Derived result = larger.subtract(smaller);
                result.sign(larger.sign());
                return result;
            }
        }
        friend constexpr Derived operator-(const Derived& lhs, const Derived& rhs) {
            Derived _rhs(rhs); _rhs.sign(!_rhs.sign());
            return lhs + _rhs;
        }
        friend constexpr Derived operator*(const Derived& lhs, const Derived& rhs) {
            return lhs.multiply(rhs);
        }
        friend constexpr Derived operator/(const Derived& lhs, const Derived& rhs) {
            return lhs.divide(rhs);
        }
        friend constexpr Derived operator%(const Derived& lhs, const Derived& rhs) {
            return lhs.modulo(rhs);
        }

        // Операторы составного присваивания реализованы через бинарные операторы выше.
        constexpr Derived& operator+=(const Derived& rhs) {
            Derived& self = static_cast<Derived&>(*this);
            self = self + rhs;
            return self;
        }
        constexpr Derived& operator-=(const Derived& rhs) {
            Derived& self = static_cast<Derived&>(*this);
            self = self - rhs;
            return self;
        }
        constexpr Derived& operator*=(const Derived& rhs) {
            Derived& self = static_cast<Derived&>(*this);
            self = self * rhs;
            return self;
        }
        constexpr Derived& operator/=(const Derived& rhs) {
            Derived& self = static_cast<Derived&>(*this);
            self = self / rhs;
            return self;
        }
        constexpr Derived& operator%=(const Derived& rhs) {
            Derived& self = static_cast<Derived&>(*this);
            self = self % rhs;
            return self;
        }

        // Инкремент и декремент (префиксная/постфиксная формы) через операцию +1 и -1.
        constexpr Derived& operator++() {
            Derived& self = static_cast<Derived&>(*this);
            self = self + Derived(1);
            return self;
        }
        constexpr Derived operator++(int) {
            Derived& self = static_cast<Derived&>(*this);
            Derived old = self;
            self = self + Derived(1);
            return old;
        }
        constexpr Derived& operator--() {
            Derived& self = static_cast<Derived&>(*this);
            self = self - Derived(1);
            return self;
        }
        constexpr Derived operator--(int) {
            Derived& self = static_cast<Derived&>(*this);
            Derived old = self;
            self = self - Derived(1);
            return old;
        }

        constexpr Derived operator-() const noexcept {
            Derived copy = static_cast<const Derived&>(*this);
            copy.sign(!copy.sign());
            return copy;
        }
        constexpr Derived operator+() noexcept {
            return Derived(static_cast<Derived&>(*this)); // copy
        }
    };


    // Общий класс, объединяющий сравнение и арифметику.
    // Здесь Derived класс должен реализовать:
    //   - int compare(const Derived&) const noexcept;
    //   - Derived add(const Derived&) const noexcept;
    //   - Derived subtract(const Derived&) const noexcept;
    //   - Derived multiply(const Derived&) const noexcept;
    //   - Derived divide(const Derived&) const noexcept;
    //   - Derived modulo(const Derived&) const noexcept;
    //   - std::string to_string() const noexcept;  // или без noexcept, если локализация/память может бросать
    template <typename Derived>
    struct IntegralBase : public BaseComparable<Derived>, public BaseArithmetic<Derived> {    
        friend constexpr Derived abs(const Derived& value) noexcept {
            if (value < Derived{}) {
                return -value;
            }
            return value;
        }
        friend Derived pow(Derived base, unsigned int exp) {
            Derived result = Derived{ 1 };
            while (exp > 0) {
                if (exp % 2 == 1)
                    result = result * base;
                base = base * base;
                exp /= 2;
            }
            return result;
        }
        friend Derived sqrt(const Derived& value) {
            if (value < Derived{}) {
                throw std::domain_error("sqrt of negative value");
            }
            if (value == Derived{}) {
                return Derived{};
            }

            Derived low = Derived{ 1 };
            Derived high = value;
            Derived mid, mid_squared;

            while (low <= high) {
                mid = (low + high) / Derived{ 2 };
                mid_squared = mid * mid;
               
                if (mid_squared == value) {
                    return mid;
                }
                else if (mid_squared < value) {
                    low = mid + Derived{ 1 };
                }
                else {
                    high = mid - Derived{ 1 };
                }
            }
            return high; // целочисленный квадратный корень
        }
    };

}
