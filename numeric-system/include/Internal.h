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
        /**
         * \~english
         * @brief Structure to store sign and a 63-bit value.
         *
         * This structure efficiently packs a boolean sign and a 63-bit unsigned integer
         * into a single `uint64_t`. The most significant bit (MSB) is used for the sign,
         * and the remaining 63 bits store the value.
         *
         * \~russian
         * @brief Структура для хранения знака и 63-битного значения.
         *
         * Эта структура эффективно упаковывает булев знак и 63-битное беззнаковое целое число
         * в один `uint64_t`. Старший бит (MSB) используется для знака,
         * а оставшиеся 63 бита хранят значение.
         */
        class StateInfo {
        private:
            /// \~english @brief Mask for the sign bit (MSB).
            /// \~russian @brief Маска для бита знака (старший бит).
            static constexpr uint64_t SIGN_MASK = 1ULL << 63;
            /// \~english @brief Mask for the value bits (all except MSB).
            /// \~russian @brief Маска для битов значения (все, кроме старшего).
            static constexpr uint64_t VALUE_MASK = ~SIGN_MASK;

            /// \~english @brief Internal storage for sign and value.
            /// \~russian @brief Внутреннее хранилище для знака и значения.
            uint64_t data_ = 0;
        public:
            /**
             * \~english
             * @brief Default constructor. Initializes with value 0 and positive sign.
             * \~russian
             * @brief Конструктор по умолчанию. Инициализирует значением 0 и положительным знаком.
             */
            constexpr StateInfo() = default;
            /**
             * \~english
             * @brief Constructor that initializes with a specific value and sign.
             * @param value The 63-bit unsigned integer value.
             * @param sign True if the number is negative, false otherwise.
             * \~russian
             * @brief Конструктор, инициализирующий с определенным значением и знаком.
             * @param value 63-битное беззнаковое целое значение.
             * @param sign True, если число отрицательное, иначе false.
             */
            constexpr StateInfo(uint64_t value, bool sign)
                : data_{ (sign ? SIGN_MASK : 0) | (value & VALUE_MASK) } {
            }
            /**
             * \~english
             * @brief Returns the 63-bit unsigned value.
             * @return The numerical value, ignoring the sign.
             * \~russian
             * @brief Возвращает 63-битное беззнаковое значение.
             * @return Числовое значение, игнорируя знак.
             */
            [[nodiscard]] constexpr uint64_t value() const noexcept {
                return data_ & VALUE_MASK;
            }
            /**
             * \~english
             * @brief Sets the 63-bit unsigned value.
             * @param value The new numerical value. The sign bit remains unchanged.
             * \~russian
             * @brief Устанавливает 63-битное беззнаковое значение.
             * @param value Новое числовое значение. Бит знака остается неизменным.
             */
            constexpr void value(uint64_t value) noexcept {
                data_ = (data_ & SIGN_MASK) | (value & VALUE_MASK);
            }
            /**
             * \~english
             * @brief Checks if the stored number is negative.
             * @return True if the sign bit is set (negative), false otherwise (positive).
             * \~russian
             * @brief Проверяет, является ли хранимое число отрицательным.
             * @return True, если бит знака установлен (отрицательное), иначе false (положительное).
             */
            [[nodiscard]] constexpr bool sign() const noexcept {
                return (data_ & SIGN_MASK) != 0;
            }
            /**
             * \~english
             * @brief Sets the sign of the stored number.
             * @param negative True to set the sign to negative, false to set it to positive.
             * \~russian
             * @brief Устанавливает знак хранимого числа.
             * @param negative True, чтобы установить знак как отрицательный, false — как положительный.
             */
            constexpr void sign(bool negative) noexcept {
                if (negative) data_ |= SIGN_MASK;
                else          data_ &= ~SIGN_MASK;
            }
        };

        /**
         * \~english
         * @brief A template class for storing large numbers using a `std::vector` of integral types.
         * @tparam T The integral type used for storing digits (e.g., `uint32_t`, `uint64_t`).
         *
         * This class manages the underlying data storage (`std::vector<value_type>`)
         * and the sign of the large number. It provides common vector-like operations
         * and accessors for the number's state (value and sign).
         *
         * \~russian
         * @brief Шаблонный класс для хранения больших чисел с использованием `std::vector` из целых типов.
         * @tparam T Целочисленный тип, используемый для хранения разрядов (например, `uint32_t`, `uint64_t`).
         *
         * Этот класс управляет базовым хранилищем данных (`std::vector<value_type>`)
         * и знаком большого числа. Он предоставляет общие операции, похожие на `std::vector`,
         * и методы доступа к состоянию числа (значению и знаку).
         */
        template <typename T>
        class Storage {
            static_assert(std::is_integral_v<T>, "T must be an integral type");
        public:
            /// \~english @brief The unsigned integral type used for internal storage.
            /// \~russian @brief Беззнаковый целочисленный тип, используемый для внутреннего хранения.
            using value_type = std::make_unsigned_t<T>;
            /// \~english @brief Size of the `value_type` in bytes.
            /// \~russian @brief Размер `value_type` в байтах.
            static constexpr size_t VALUE_SIZE = sizeof(value_type);
            /// \~english @brief Maximum value representable by `value_type`.
            /// \~russian @brief Максимальное значение, представимое `value_type`.
            static constexpr value_type MAX_VALUE = std::numeric_limits<value_type>::max();
            /// \~english @brief Minimum value representable by `value_type`.
            /// \~russian @brief Минимальное значение, представимое `value_type`.
            static constexpr value_type MIN_VALUE = std::numeric_limits<value_type>::min();
            /// \~english @brief Number of bits in `value_type`.
            /// \~russian @brief Количество бит в `value_type`.
            static constexpr int VALUE_COUNT_BIT = std::numeric_limits<value_type>::digits;
        private:
            /// \~english @brief Stores the sign and an auxiliary value (e.g., for base conversion hints).
            /// \~russian @brief Хранит знак и вспомогательное значение (например, для подсказок при преобразовании оснований).
            StateInfo state_;
            /// \~english @brief The underlying vector storing the digits of the large number.
            /// \~russian @brief Базовый вектор, хранящий разряды большого числа.
            std::vector<value_type> data_;
        public:
            /**
             * \~english
             * @brief Default constructor. Initializes an empty storage with a positive sign.
             * \~russian
             * @brief Конструктор по умолчанию. Инициализирует пустое хранилище с положительным знаком.
             */
            constexpr Storage() : state_({}), data_(0) {};

            /**
             * \~english
             * @brief Provides non-const access to the underlying data vector.
             * @return A reference to the mutable `std::vector<value_type>`.
             * \~russian
             * @brief Предоставляет неконстантный доступ к базовому вектору данных.
             * @return Ссылка на изменяемый `std::vector<value_type>`.
             */
            std::vector<value_type>& data() { return data_; }
            /**
             * \~english
             * @brief Provides const access to the underlying data vector.
             * @return A const reference to the `std::vector<value_type>`.
             * \~russian
             * @brief Предоставляет константный доступ к базовому вектору данных.
             * @return Константная ссылка на `std::vector<value_type>`.
             */
            const std::vector<value_type>& data() const { return data_; }

            // State access
            /**
             * \~english
             * @brief Gets the auxiliary value stored in `StateInfo`.
             * @return The 63-bit auxiliary value.
             * \~russian
             * @brief Получает вспомогательное значение, хранящееся в `StateInfo`.
             * @return 63-битное вспомогательное значение.
             */
            [[nodiscard]] constexpr uint64_t value() const noexcept { return state_.value(); }
            /**
             * \~english
             * @brief Sets the auxiliary value in `StateInfo`.
             * @param val The 63-bit auxiliary value to set.
             * \~russian
             * @brief Устанавливает вспомогательное значение в `StateInfo`.
             * @param val 63-битное вспомогательное значение для установки.
             */
            constexpr void value(uint64_t val) noexcept { state_.value(val); }

            /**
             * \~english
             * @brief Gets the sign of the stored number.
             * @return True if negative, false if positive.
             * \~russian
             * @brief Получает знак хранимого числа.
             * @return True, если отрицательное, false, если положительное.
             */
            [[nodiscard]] constexpr bool sign() const noexcept { return state_.sign(); }
            /**
             * \~english
             * @brief Sets the sign of the stored number.
             * @param s True for negative, false for positive.
             * \~russian
             * @brief Устанавливает знак хранимого числа.
             * @param s True для отрицательного, false для положительного.
             */
            constexpr void sign(bool s) noexcept { state_.sign(s); }

            // Data operations
            /**
             * \~english
             * @brief Clears all elements from the internal data vector.
             * \~russian
             * @brief Удаляет все элементы из внутреннего вектора данных.
             */
            void clear() noexcept { data_.clear(); }

            /**
             * \~english
             * @brief Returns the number of elements in the internal data vector.
             * @return The size of the vector.
             * \~russian
             * @brief Возвращает количество элементов во внутреннем векторе данных.
             * @return Размер вектора.
             */
            [[nodiscard]] size_t size() const noexcept { return data_.size(); }
            /**
             * \~english
             * @brief Checks if the internal data vector is empty.
             * @return True if the vector contains no elements, false otherwise.
             * \~russian
             * @brief Проверяет, пуст ли внутренний вектор данных.
             * @return True, если вектор не содержит элементов, иначе false.
             */
            [[nodiscard]] bool empty() const noexcept { return data_.empty(); }

            /**
             * \~english
             * @brief Adds a new element to the end of the data vector.
             * @param val The value to add.
             * \~russian
             * @brief Добавляет новый элемент в конец вектора данных.
             * @param val Значение для добавления.
             */
            void push_back(value_type val) { data_.push_back(val); }
            /**
             * \~english
             * @brief Removes the last element from the data vector.
             * \~russian
             * @brief Удаляет последний элемент из вектора данных.
             */
            void pop_back() { data_.pop_back(); }

            /**
             * \~english
             * @brief Provides non-const access to the last element.
             * @return A reference to the last element.
             * \~russian
             * @brief Предоставляет неконстантный доступ к последнему элементу.
             * @return Ссылка на последний элемент.
             */
            [[nodiscard]] value_type& back() { return data_.back(); }
            /**
             * \~english
             * @brief Provides const access to the last element.
             * @return A const reference to the last element.
             * \~russian
             * @brief Предоставляет константный доступ к последнему элементу.
             * @return Константная ссылка на последний элемент.
             */
            [[nodiscard]] const value_type& back() const { return data_.back(); }

            /**
             * \~english
             * @brief Provides non-const access to an element at a specific index.
             * @param index The index of the element.
             * @return A reference to the element.
             * \~russian
             * @brief Предоставляет неконстантный доступ к элементу по указанному индексу.
             * @param index Индекс элемента.
             * @return Ссылка на элемент.
             */
            [[nodiscard]] value_type& operator[](size_t index) { return data_[index]; }
            /**
             * \~english
             * @brief Provides const access to an element at a specific index.
             * @param index The index of the element.
             * @return A const reference to the element.
             * \~russian
             * @brief Предоставляет константный доступ к элементу по указанному индексу.
             * @param index Индекс элемента.
             * @return Константная ссылка на элемент.
             */
            [[nodiscard]] const value_type& operator[](size_t index) const { return data_[index]; }

            /**
             * \~english
             * @brief Reserves capacity for the internal data vector.
             * @param capacity The new capacity to reserve.
             * \~russian
             * @brief Резервирует емкость для внутреннего вектора данных.
             * @param capacity Новая емкость для резервирования.
             */
            void reserve(size_t capacity) { data_.reserve(capacity); }
            /**
             * \~english
             * @brief Resizes the internal data vector.
             * @param new_size The new size of the vector.
             * @param val The value to initialize new elements with (defaults to 0).
             * \~russian
             * @brief Изменяет размер внутреннего вектора данных.
             * @param new_size Новый размер вектора.
             * @param val Значение, которым инициализируются новые элементы (по умолчанию 0).
             */
            void resize(size_t new_size, value_type val = 0) { data_.resize(new_size, val); }

            // Iterators
            /// \~english @brief Returns an iterator to the beginning.
            /// \~russian @brief Возвращает итератор на начало.
            [[nodiscard]] auto begin() noexcept { return data_.begin(); }
            /// \~english @brief Returns an iterator to the end.
            /// \~russian @brief Возвращает итератор на конец.
            [[nodiscard]] auto end() noexcept { return data_.end(); }
            /// \~english @brief Returns a const iterator to the beginning.
            /// \~russian @brief Возвращает константный итератор на начало.
            [[nodiscard]] auto begin() const noexcept { return data_.begin(); }
            /// \~english @brief Returns a const iterator to the end.
            /// \~russian @brief Возвращает константный итератор на конец.
            [[nodiscard]] auto end() const noexcept { return data_.end(); }

            /// \~english @brief Returns a reverse iterator to the beginning.
            /// \~russian @brief Возвращает обратный итератор на начало.
            [[nodiscard]] auto rbegin() noexcept { return data_.rbegin(); }
            /// \~english @brief Returns a reverse iterator to the end.
            /// \~russian @brief Возвращает обратный итератор на конец.
            [[nodiscard]] auto rend() noexcept { return data_.rend(); }
            /// \~english @brief Returns a reverse iterator to the beginning.
            /// \~russian @brief Возвращает обратный итератор на начало.
            [[nodiscard]] auto rbegin() const noexcept { return data_.rbegin(); }
            /// \~english @brief Returns a reverse iterator to the end.
            /// \~russian @brief Возвращает обратный итератор на конец.
            [[nodiscard]] auto rend() const noexcept { return data_.rend(); }
        };

        /**
         * \~english
         * @brief Provides static methods for overflow-aware arithmetic operations on unsigned integers.
         *
         * These methods are designed to handle carries and borrows explicitly, which is crucial
         * for multi-precision arithmetic implementations.
         * \~russian
         * @brief Предоставляет статические методы для арифметических операций с беззнаковыми целыми числами,
         * учитывающих переполнение.
         *
         * Эти методы разработаны для явной обработки переносов и заимствований, что крайне важно
         * для реализаций арифметики с произвольной точностью.
         */
        struct OverflowAwareOps {
            /**
             * \~english
             * @brief Performs addition of two unsigned integers with a carry-in.
             * @tparam _Ty The unsigned integral type.
             * @param ai The first operand.
             * @param bi The second operand.
             * @param carry A reference to the carry-in/carry-out value (0 or 1).
             * @return The sum of `ai`, `bi`, and `carry`, modulo `_Ty::max() + 1`.
             * \~russian
             * @brief Выполняет сложение двух беззнаковых целых чисел с учетом переноса.
             * @tparam _Ty Беззнаковый целочисленный тип.
             * @param ai Первый операнд.
             * @param bi Второй операнд.
             * @param carry Ссылка на значение переноса (0 или 1), которое также будет обновлено как перенос на выходе.
             * @return Сумма `ai`, `bi` и `carry`, по модулю `_Ty::max() + 1`.
             */
            template<typename _Ty, typename = std::enable_if_t<std::is_unsigned<_Ty>::value>>
            static constexpr _Ty sum(_Ty ai, _Ty bi, _Ty& carry) noexcept {
                _Ty result = ai + bi + carry;
                carry = (result < ai || result < bi || (carry && result == ai)) ? 1 : 0;
                return result;
            }

            /**
             * \~english
             * @brief Performs subtraction of two unsigned integers with a borrow-in.
             * @tparam _Ty The unsigned integral type.
             * @param ai The first operand (minuend).
             * @param bi The second operand (subtrahend).
             * @param borrow A reference to the borrow-in/borrow-out value (0 or 1).
             * @return The difference of `ai`, `bi`, and `borrow`, handling underflow by wrapping around.
             * \~russian
             * @brief Выполняет вычитание двух беззнаковых целых чисел с учетом заимствования.
             * @tparam _Ty Беззнаковый целочисленный тип.
             * @param ai Первый операнд (уменьшаемое).
             * @param bi Второй операнд (вычитаемое).
             * @param borrow Ссылка на значение заимствования (0 или 1), которое также будет обновлено как заимствование на выходе.
             * @return Разность `ai`, `bi` и `borrow`, обрабатывая переполнение путем циклического сброса.
             */
            template<typename _Ty, typename = std::enable_if_t<std::is_unsigned<_Ty>::value>>
            static constexpr _Ty subtract(_Ty ai, _Ty bi, _Ty& borrow) noexcept {
                _Ty result = ai - bi - borrow;
                borrow = (ai < bi + borrow) ? 1 : 0;
                return result;
            }
        };

        /**
         * \~english
         * @brief Provides static methods for string-based arithmetic operations on large numbers.
         *
         * These methods handle basic arithmetic (addition, subtraction, multiplication, division)
         * and utility functions (string validation, trimming zeros, comparison)
         * for numbers represented as strings.
         * \~russian
         * @brief Предоставляет статические методы для строковых арифметических операций над большими числами.
         *
         * Эти методы обрабатывают базовые арифметические операции (сложение, вычитание, умножение, деление)
         * и вспомогательные функции (проверка строк, удаление нулей, сравнение)
         * для чисел, представленных в виде строк.
         */
        struct BigNumberOperations {
            /// \~english @brief Constant character '0' for string conversions.
            /// \~russian @brief Константный символ '0' для преобразований строк.
            static constexpr char ZERO = '0';
            /// \~english @brief Enum for specifying zero trimming mode (leading or trailing).
            /// \~russian @brief Перечисление для указания режима удаления нулей (ведущие или завершающие).
            enum class TrimMode { Leading, Trailing };

            /**
             * \~english
             * @brief Checks if a string represents a valid integral number.
             *
             * A valid integral string can optionally start with a '-' sign,
             * must not contain leading zeros (unless it's "0" itself),
             * and must only contain digits '0'-'9' after the optional sign.
             *
             * @param str The string to validate.
             * @return True if the string is a valid integral number representation, false otherwise.
             * \~russian
             * @brief Проверяет, представляет ли строка допустимое целое число.
             *
             * Допустимая целочисленная строка может опционально начинаться с символа '-',
             * не должна содержать ведущих нулей (если это не "0" само по себе),
             * и после необязательного знака должна содержать только цифры '0'-'9'.
             *
             * @param str Строка для проверки.
             * @return True, если строка является допустимым представлением целого числа, иначе false.
             */
            static constexpr bool is_integral_valid_string(std::string_view str) noexcept {
                if (str.empty()) {
                    return false;
                }

                size_t start_idx = 0;
                if (str[0] == '-') {
                    if (str.length() == 1) { // Only '-' is not a number
                        return false;
                    }
                    start_idx = 1;
                }

                // Check for leading zeros (except for "0")
                if (str.length() > start_idx + 1 && str[start_idx] == ZERO) {
                    // If string is like "0123" or "-0123"
                    return false;
                }

                for (size_t i = start_idx; i < str.length(); ++i) {
                    if (str[i] < ZERO || str[i] > '9') {
                        return false;
                    }
                }
                return true;
            }
            /**
             * \~english
             * @brief Compares two string representations of numbers to check if 'a' is greater than or equal to 'b'.
             * @param a The first number string.
             * @param b The second number string.
             * @return True if 'a' is numerically greater than or equal to 'b', false otherwise.
             * \~russian
             * @brief Сравнивает два строковых представления чисел, чтобы проверить, больше или равно ли 'a' 'b'.
             * @param a Первая строка числа.
             * @param b Вторая строка числа.
             * @return True, если 'a' численно больше или равно 'b', иначе false.
             */
            static constexpr bool greater_or_equal(std::string_view a, std::string_view b) noexcept {
                if (a.length() != b.length()) {
                    return a.length() > b.length();
                }
                return a.compare(b) >= 0;
            }

            /**
             * \~english
             * @brief Removes leading or trailing zeros from a container of characters or numbers.
             * @tparam Container The type of the container (e.g., `std::string`, `std::vector<char>`).
             * @param c The container to trim.
             * @param mode The trimming mode (Leading or Trailing).
             *
             * If the container becomes empty after trimming, a single '0' (or 0 for numeric types)
             * is added to ensure it always represents a valid number.
             * \~russian
             * @brief Удаляет ведущие или завершающие нули из контейнера символов или чисел.
             * @tparam Container Тип контейнера (например, `std::string`, `std::vector<char>`).
             * @param c Контейнер для обрезки.
             * @param mode Режим обрезки (ведущие или завершающие).
             *
             * Если контейнер становится пустым после обрезки, добавляется один '0' (или 0 для числовых типов),
             * чтобы он всегда представлял допустимое число.
             */
            template<typename Container>
            static inline void remove_zeros(Container& c, TrimMode mode) {
                if (c.empty()) return;

                using ValueType = typename Container::value_type;
                const ValueType zero = [] {
                    if constexpr (std::is_same_v<ValueType, char>)
                        return BigNumberOperations::ZERO;
                    else
                        return ValueType(0);
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
            /**
             * \~english
             * @brief Divides a string-represented number by an integral divisor.
             * @param str The string representation of the dividend.
             * @param divisor The integral divisor.
             * @return A `std::pair` containing the quotient string and the remainder as a `uint64_t`.
             * @throws std::runtime_error if `divisor` is zero.
             * \~russian
             * @brief Делит число, представленное строкой, на целочисленный делитель.
             * @param str Строковое представление делимого.
             * @param divisor Целочисленный делитель.
             * @return `std::pair`, содержащий строку частного и остаток в виде `uint64_t`.
             * @throws std::runtime_error Если `divisor` равен нулю.
             */
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

            /**
             * \~english
             * @brief Adds two string-represented non-negative numbers.
             * @param a The first number string.
             * @param b The second number string.
             * @return A new string representing the sum of 'a' and 'b'.
             * \~russian
             * @brief Складывает два неотрицательных числа, представленных строками.
             * @param a Первая строка числа.
             * @param b Вторая строка числа.
             * @return Новая строка, представляющая сумму 'a' и 'b'.
             */
            static std::string add_strings(std::string_view a, std::string_view b) {
                if (a.length() < b.length()) {
                    std::swap(a, b);
                }

                std::string result_str;
                result_str.reserve(a.length() + 1);

                int carry = 0;
                int i = static_cast<int>(a.length()) - 1;
                int j = static_cast<int>(b.length()) - 1;

                while (i >= 0 || carry) {
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
            /**
             * \~english
             * @brief Subtracts one string-represented non-negative number from another.
             * @param a The minuend string. Must be greater than or equal to `b`.
             * @param b The subtrahend string.
             * @return A new string representing the difference (`a - b`).
             * @throws std::runtime_error if `a` is numerically less than `b`.
             * \~russian
             * @brief Вычитает одно неотрицательное число, представленное строкой, из другого.
             * @param a Строка уменьшаемого. Должно быть больше или равно `b`.
             * @param b Строка вычитаемого.
             * @return Новая строка, представляющая разность (`a - b`).
             * @throws std::runtime_error Если `a` численно меньше `b`.
             */
            static std::string subtract_strings(std::string_view a, std::string_view b) {
                if (b == "0") return std::string(a);
                if (a == b) return "0";

                if (!greater_or_equal(a, b)) {
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
            /**
             * \~english
             * @brief Multiplies two string-represented non-negative numbers.
             * @param a The first number string.
             * @param b The second number string.
             * @return A new string representing the product (`a * b`).
             * \~russian
             * @brief Умножает два неотрицательных числа, представленных строками.
             * @param a Первая строка числа.
             * @param b Вторая строка числа.
             * @return Новая строка, представляющая произведение (`a * b`).
             */
            static std::string multiply_strings(std::string_view a, std::string_view b) {
                if (a == "0" || b == "0") {
                    return "0";
                }

                std::vector<int> result_digits(a.length() + b.length(), 0);

                for (int i = static_cast<int>(a.length()) - 1; i >= 0; --i) {
                    int digit_a = a[i] - ZERO;
                    for (int j = static_cast<int>(b.length()) - 1; j >= 0; --j) {
                        int digit_b = b[j] - ZERO;

                        int product = digit_a * digit_b;
                        int sum = product + result_digits[i + j + 1];

                        result_digits[i + j + 1] = sum % 10;
                        result_digits[i + j] += sum / 10;
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
            /**
             * \~english
             * @brief Divides one string-represented non-negative number by another.
             * @param a The dividend string.
             * @param b The divisor string.
             * @return A `std::pair` containing the quotient string and the remainder string.
             * @throws std::overflow_error if `b` (divisor) is "0".
             * \~russian
             * @brief Делит одно неотрицательное число, представленное строкой, на другое.
             * @param a Строка делимого.
             * @param b Строка делителя.
             * @return `std::pair`, содержащий строку частного и строку остатка.
             * @throws std::overflow_error Если `b` (делитель) равен "0".
             */
            static std::pair<std::string, std::string> divide_strings(std::string_view a, std::string_view b) {
                if (b == "0") {
                    throw std::overflow_error("Division by zero.");
                }
                if (a == "0") {
                    return { "0", "0" };
                }
                if (!greater_or_equal(a, b)) {
                    return { "0", std::string(a) };
                }

                std::string quotient_str;
                quotient_str.reserve(a.length());
                std::string current_remainder_str = "";

                for (char digit_char : a) {
                    current_remainder_str.push_back(digit_char);
                    remove_zeros(current_remainder_str, TrimMode::Leading);

                    int count = 0;
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


    /**
     * \~english
     * @brief A CRTP base class for implementing comparison operators.
     *
     * Derived classes must implement a `compare(const Derived&)` method that returns:
     * - a negative value (< 0) if `*this < other`
     * - zero (0) if `*this == other`
     * - a positive value (> 0) if `*this > other`
     *
     * This class then provides `operator<`, `operator>`, `operator<=`, `operator>=`,
     * `operator==`, and `operator!=` using the `compare` method.
     * @tparam Derived The derived class that inherits from `BaseComparable`.
     * \~russian
     * @brief Базовый CRTP-класс для реализации операторов сравнения.
     *
     * Производные классы должны реализовать метод `compare(const Derived&)`, который возвращает:
     * - отрицательное значение (< 0), если `*this < other`
     * - ноль (0), если `*this == other`
     * - положительное значение (> 0), если `*this > other`
     *
     * Этот класс затем предоставляет `operator<`, `operator>`, `operator<=`, `operator>=`,
     * `operator==` и `operator!=`, используя метод `compare`.
     * @tparam Derived Производный класс, который наследует от `BaseComparable`.
     */
    template <typename Derived>
    struct BaseComparable {
        /// \~english @brief Less than operator.
        /// \~russian @brief Оператор "меньше".
        friend constexpr bool operator<(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) < 0;
        }
        /// \~english @brief Greater than operator.
        /// \~russian @brief Оператор "больше".
        friend constexpr bool operator>(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) > 0;
        }
        /// \~english @brief Less than or equal to operator.
        /// \~russian @brief Оператор "меньше или равно".
        friend constexpr bool operator<=(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) <= 0;
        }
        /// \~english @brief Greater than or equal to operator.
        /// \~russian @brief Оператор "больше или равно".
        friend constexpr bool operator>=(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) >= 0;
        }
        /// \~english @brief Equality operator.
        /// \~russian @brief Оператор равенства.
        friend constexpr bool operator==(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) == 0;
        }
        /// \~english @brief Inequality operator.
        /// \~russian @brief Оператор неравенства.
        friend constexpr bool operator!=(const Derived& lhs, const Derived& rhs) noexcept {
            return lhs.compare(rhs) != 0;
        }
    };


    /**
     * \~english
     * @brief A CRTP base class for implementing arithmetic operators.
     *
     * Derived classes must provide implementations for the following methods:
     * - `Derived add(const Derived&) const;`
     * - `Derived subtract(const Derived&) const;`
     * - `Derived multiply(const Derived&) const;`
     * - `Derived divide(const Derived&) const;`
     * - `Derived modulo(const Derived&) const;`
     * All these methods should return a new result object and not modify `*this`.
     *
     * This class then provides overloaded binary operators (`+`, `-`, `*`, `/`, `%`),
     * compound assignment operators (`+=`, `-=`, `*=`, `/=`, `%=`),
     * and increment/decrement operators (`++`, `--`) based on these fundamental methods.
     * @tparam Derived The derived class that inherits from `BaseArithmetic`.
     * \~russian
     * @brief Базовый CRTP-класс для реализации арифметических операторов.
     *
     * Производные классы должны предоставить реализации для следующих методов:
     * - `Derived add(const Derived&) const;`
     * - `Derived subtract(const Derived&) const;`
     * - `Derived multiply(const Derived&) const;`
     * - `Derived divide(const Derived&) const;`
     * - `Derived modulo(const Derived&) const;`
     * Все эти методы должны возвращать новый объект-результат и не изменять `*this`.
     *
     * Этот класс затем предоставляет перегруженные бинарные операторы (`+`, `-`, `*`, `/`, `%`),
     * операторы составного присваивания (`+=`, `-=`, `*=`, `/=`, `%=`),
     * и операторы инкремента/декремента (`++`, `--`), основанные на этих фундаментальных методах.
     * @tparam Derived Производный класс, который наследует от `BaseArithmetic`.
     */
    template <typename Derived>
    struct BaseArithmetic {
        /// \~english @brief Addition operator.
        /// \~russian @brief Оператор сложения.
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
        /// \~english @brief Subtraction operator.
        /// \~russian @brief Оператор вычитания.
        friend constexpr Derived operator-(const Derived& lhs, const Derived& rhs) {
            Derived _rhs(rhs); _rhs.sign(!_rhs.sign());
            return lhs + _rhs;
        }
        /// \~english @brief Multiplication operator.
        /// \~russian @brief Оператор умножения.
        friend constexpr Derived operator*(const Derived& lhs, const Derived& rhs) {
            return lhs.multiply(rhs);
        }
        /// \~english @brief Division operator.
        /// \~russian @brief Оператор деления.
        friend constexpr Derived operator/(const Derived& lhs, const Derived& rhs) {
            return lhs.divide(rhs);
        }
        /// \~english @brief Modulo operator.
        /// \~russian @brief Оператор взятия остатка от деления.
        friend constexpr Derived operator%(const Derived& lhs, const Derived& rhs) {
            return lhs.modulo(rhs);
        }

        // Compound assignment operators
        /// \~english @brief Compound addition assignment operator.
        /// \~russian @brief Оператор составного присваивания сложения.
        constexpr Derived& operator+=(const Derived& rhs) {
            Derived& self = static_cast<Derived&>(*this);
            self = self + rhs;
            return self;
        }
        /// \~english @brief Compound subtraction assignment operator.
        /// \~russian @brief Оператор составного присваивания вычитания.
        constexpr Derived& operator-=(const Derived& rhs) {
            Derived& self = static_cast<Derived&>(*this);
            self = self - rhs;
            return self;
        }
        /// \~english @brief Compound multiplication assignment operator.
        /// \~russian @brief Оператор составного присваивания умножения.
        constexpr Derived& operator*=(const Derived& rhs) {
            Derived& self = static_cast<Derived&>(*this);
            self = self * rhs;
            return self;
        }
        /// \~english @brief Compound division assignment operator.
        /// \~russian @brief Оператор составного присваивания деления.
        constexpr Derived& operator/=(const Derived& rhs) {
            Derived& self = static_cast<Derived&>(*this);
            self = self / rhs;
            return self;
        }
        /// \~english @brief Compound modulo assignment operator.
        /// \~russian @brief Оператор составного присваивания взятия остатка от деления.
        constexpr Derived& operator%=(const Derived& rhs) {
            Derived& self = static_cast<Derived&>(*this);
            self = self % rhs;
            return self;
        }

        // Increment and decrement (prefix/postfix forms)
        /// \~english @brief Prefix increment operator.
        /// \~russian @brief Префиксный оператор инкремента.
        constexpr Derived& operator++() {
            Derived& self = static_cast<Derived&>(*this);
            self = self + Derived(1);
            return self;
        }
        /// \~english @brief Postfix increment operator.
        /// \~russian @brief Постфиксный оператор инкремента.
        constexpr Derived operator++(int) {
            Derived& self = static_cast<Derived&>(*this);
            Derived old = self;
            self = self + Derived(1);
            return old;
        }
        /// \~english @brief Prefix decrement operator.
        /// \~russian @brief Префиксный оператор декремента.
        constexpr Derived& operator--() {
            Derived& self = static_cast<Derived&>(*this);
            self = self - Derived(1);
            return self;
        }
        /// \~english @brief Postfix decrement operator.
        /// \~russian @brief Постфиксный оператор декремента.
        constexpr Derived operator--(int) {
            Derived& self = static_cast<Derived&>(*this);
            Derived old = self;
            self = self - Derived(1);
            return old;
        }

        /// \~english @brief Unary negation operator.
        /// \~russian @brief Унарный оператор отрицания.
        constexpr Derived operator-() const noexcept {
            Derived copy = static_cast<const Derived&>(*this);
            copy.sign(!copy.sign());
            return copy;
        }
        /// \~english @brief Unary plus operator (returns a copy).
        /// \~russian @brief Унарный оператор плюс (возвращает копию).
        constexpr Derived operator+() noexcept {
            return Derived(static_cast<Derived&>(*this)); // copy
        }
    };


    /**
     * \~english
     * @brief A comprehensive CRTP base class for integral-like types, combining comparison and arithmetic functionalities.
     *
     * Derived classes inheriting from `IntegralBase` must implement the following methods:
     * - `int compare(const Derived&) const noexcept;`
     * - `Derived add(const Derived&) const noexcept;`
     * - `Derived subtract(const Derived&) const noexcept;`
     * - `Derived multiply(const Derived&) const noexcept;`
     * - `Derived divide(const Derived&) const noexcept;`
     * - `Derived modulo(const Derived&) const noexcept;`
     * - `std::string to_string() const;` (or `noexcept` if applicable)
     * - A constructor `Derived(int)` to handle `Derived{0}` and `Derived{1}` in `pow` and `sqrt`.
     *
     * This class provides common mathematical functions like absolute value (`abs`),
     * exponentiation (`pow`), and integer square root (`sqrt`).
     * @tparam Derived The derived class that inherits from `IntegralBase`.
     * \~russian
     * @brief Комплексный базовый CRTP-класс для целочисленных типов, объединяющий функциональность сравнения и арифметики.
     *
     * Производные классы, наследующие от `IntegralBase`, должны реализовать следующие методы:
     * - `int compare(const Derived&) const noexcept;`
     * - `Derived add(const Derived&) const noexcept;`
     * - `Derived subtract(const Derived&) const noexcept;`
     * - `Derived multiply(const Derived&) const noexcept;`
     * - `Derived divide(const Derived&) const noexcept;`
     * - `Derived modulo(const Derived&) const noexcept;`
     * - `std::string to_string() const;` (или `noexcept`, если применимо)
     * - Конструктор `Derived(int)` для обработки `Derived{0}` и `Derived{1}` в `pow` и `sqrt`.
     *
     * Этот класс предоставляет общие математические функции, такие как абсолютное значение (`abs`),
     * возведение в степень (`pow`) и целочисленный квадратный корень (`sqrt`).
     * @tparam Derived Производный класс, который наследует от `IntegralBase`.
     */
    template <typename Derived>
    struct IntegralBase : public BaseComparable<Derived>, public BaseArithmetic<Derived> {
        /**
         * \~english
         * @brief Calculates the absolute value of a number.
         * @param value The number whose absolute value is to be calculated.
         * @return The absolute value of `value`.
         * \~russian
         * @brief Вычисляет абсолютное значение числа.
         * @param value Число, абсолютное значение которого нужно вычислить.
         * @return Абсолютное значение `value`.
         */
        friend constexpr Derived abs(const Derived& value) noexcept {
            if (value < Derived{}) {
                return -value;
            }
            return value;
        }
        /**
         * \~english
         * @brief Calculates the power of a base to an unsigned integer exponent (integer exponentiation).
         * @param base The base number.
         * @param exp The unsigned integer exponent.
         * @return The result of `base` raised to the power of `exp`.
         * \~russian
         * @brief Вычисляет степень основания, возведенного в беззнаковый целый показатель (целочисленное возведение в степень).
         * @param base Основание.
         * @param exp Беззнаковый целый показатель степени.
         * @return Результат `base` в степени `exp`.
         */
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
        /**
         * \~english
         * @brief Calculates the integer square root of a non-negative number.
         * @param value The non-negative number to calculate the square root of.
         * @return The integer square root of `value`. Returns 0 if `value` is 0.
         * @throws std::domain_error if `value` is negative.
         * \~russian
         * @brief Вычисляет целочисленный квадратный корень из неотрицательного числа.
         * @param value Неотрицательное число, из которого нужно извлечь квадратный корень.
         * @return Целочисленный квадратный корень из `value`. Возвращает 0, если `value` равно 0.
         * @throws std::domain_error Если `value` отрицательное.
         */
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
            return high;
        }
    };

}