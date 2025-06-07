#pragma once
#include "Internal.h"
#include <iostream>

namespace numsystem {
    namespace internal {

        struct FactorAccess {
            static constexpr size_t MAXINDEX = (1ULL << 63) - 1;
            static constexpr uint64_t count_bits(uint64_t value) noexcept {
                uint64_t width = 0;
                while (value != 0) {
                    ++width;
                    value >>= 1;
                }
                return width;
            }
            static constexpr uint64_t log2_floor(uint64_t value) {
                return (value == 0) ? 0 : count_bits(value) - 1;
            }
            static constexpr uint64_t safe_shift_left(uint64_t x, uint64_t s) {
                return (s >= 64) ? 0 : (x << s);
            }
            static constexpr uint64_t total_bits_up_to(uint64_t index) {
                if (index == 0 || index == 1) return 0;
                if (index > MAXINDEX) {
                    throw std::invalid_argument("index is too large: exceeds MAXINDEX");
                }

                uint64_t N = index - 1;                             // Гарантированно N > 0
                uint64_t M = log2_floor(N);                         // M = floor(log2(N))
                uint64_t pow2 = safe_shift_left(1ULL, M + 1);       // 2^(M+1)
                return N + (M * N - (pow2 - M - 2));
            }

            template<typename _Ty>
            static std::optional<uint64_t> extract(const impl::Storage<_Ty>& data, size_t index) {
                using value_type = typename impl::Storage<_Ty>::value_type;
                if (index > MAXINDEX-10) { 
                    if (index > MAXINDEX) throw std::out_of_range("extract: index is out of allowed range");
                }
                uint64_t poswd = total_bits_up_to(index);   // позиция
                uint64_t sizewd = count_bits(index);        // размер
                if (sizewd == 0) return 0;                  // Обработка нулевой длины

                // Общая длина хранилища в битах
                // Проверка выхода за границы
                uint64_t VALUE_COUNT_BIT = std::numeric_limits<value_type>::digits;
                size_t count_bits_in_storage = data.size() * std::numeric_limits<value_type>::digits;
                if (poswd >= count_bits_in_storage) {
                    return std::nullopt; // Начало чтения за границами данных
                }
                if (poswd + sizewd > count_bits_in_storage) {
                    return std::nullopt; // Конец чтения за границами данных
                }
                      
                uint64_t result = 0;								// Результат (значение коэффициента)
                uint64_t bits_processed = 0;						// Сколько бит уже прочитано для этого коэффициента
                uint64_t current_bit_in_storage = poswd;	        // Абсолютный индекс текущего читаемого бита в _storage

                // Читаем биты, пока не наберем bit_length
                while (bits_processed < sizewd) {
                    // Определяем, в каком слове и по какому смещению находится текущий бит
                    uint64_t word_idx = current_bit_in_storage / VALUE_COUNT_BIT;
                    uint64_t bit_offset_in_word = current_bit_in_storage % VALUE_COUNT_BIT;

                    // Сколько бит мы можем прочитать из текущего слова, начиная с текущего смещения?
                    uint64_t bits_available_in_word = VALUE_COUNT_BIT - bit_offset_in_word;

                    // Сколько бит нужно прочитать на этом шаге (минимум из оставшейся длины коэффициента и доступных бит в слове)
                    uint64_t bits_to_read_in_step = std::min(sizewd - bits_processed, bits_available_in_word);

                    // Читаем текущее слово как uint64_t для удобства побитовых операций
                    uint64_t word_64 = data[word_idx];

                    // Извлекаем нужный кусок бит из слова. Сдвигаем вправо, чтобы нужные биты оказались в младших позициях.
                    uint64_t chunk = word_64 >> bit_offset_in_word;

                    // Маскируем, чтобы оставить только нужное количество бит для этого шага
                    // Если читаем ровно до конца uint64_t или VALUE_COUNT_BIT, маска не нужна (или ~0ULL)
                    if (bits_to_read_in_step < 64) { // Маска 1ULL << N - 1 безопасна, пока N < 64
                        chunk &= (1ULL << bits_to_read_in_step) - 1;
                    }

                    // Добавляем извлеченный кусок к результату. Сдвигаем его влево на количество уже прочитанных бит.
                    result |= chunk << bits_processed;

                    // Обновляем счетчики и позицию
                    bits_processed += bits_to_read_in_step;
                    current_bit_in_storage += bits_to_read_in_step;
                }
                return result;
            }

            template<typename _Ty>
            static void put(impl::Storage<_Ty>& data, size_t index, uint64_t value) {
                using value_type = typename impl::Storage<_Ty>::value_type;
                using RawType = std::remove_cv_t<std::remove_reference_t<decltype(data)>>;
                if (index > MAXINDEX) { throw std::out_of_range("put: index is out of allowed range"); }
                uint64_t poswd = total_bits_up_to(index);   // позиция
                uint64_t sizewd = count_bits(index);        // размер
                if (sizewd == 0) return;                    // Обработка нулевой длины
                if (index < value) { throw std::logic_error("value exceeds base (index) in factorial number system: value must be <= index"); }  // В факториальной системе по основнанию не может находится число больше самого основания            
                if (data.value() < index) data.value(index);

                uint64_t VALUE_COUNT_BIT = std::numeric_limits<value_type>::digits;
                size_t count_bits_in_storage = data.size() * std::numeric_limits<value_type>::digits;
       
                // Проверка, что хранилище имеет достаточный размер
                uint64_t total_words_needed = (poswd + sizewd + VALUE_COUNT_BIT - 1) / VALUE_COUNT_BIT;
                if (total_words_needed > data.size()) data.resize(total_words_needed, 0);

                uint64_t bits_processed = 0;				// Сколько бит из value уже обработано
                uint64_t current_bit_in_storage = poswd;	// Абсолютный индекс текущего записываемого бита в _storage

                
                // Записываем биты из value, пока не запишем bit_length
                while (bits_processed < sizewd) {
                    // Определяем, в каком слове и по какому смещению находится текущая позиция записи
                    uint64_t word_idx = current_bit_in_storage / VALUE_COUNT_BIT;
                    uint64_t bit_offset_in_word = current_bit_in_storage % VALUE_COUNT_BIT;

                    // Сколько бит мы можем записать в текущее слово, начиная с текущего смещения?
                    uint64_t bits_available_in_word = VALUE_COUNT_BIT - bit_offset_in_word;

                    // Сколько бит нужно записать на этом шаге (минимум из оставшейся длины коэффициента и доступных бит в слове)
                    uint64_t bits_to_write_in_step = std::min(sizewd - bits_processed, bits_available_in_word);

                    // Извлекаем нужный кусок бит из значения value. Сдвигаем вправо, чтобы нужные биты оказались в младших позициях.
                    uint64_t chunk_from_value = value >> bits_processed;
                    // Маскируем, чтобы оставить только нужное количество бит для этого шага
                    if (bits_to_write_in_step < 64) { // Маска безопасна, пока N < 64
                        chunk_from_value &= (1ULL << bits_to_write_in_step) - 1;
                    }

                    // Создаем маску для очистки соответствующих бит в целевом слове.
                    // Маска должна иметь 1 там, куда мы будем записывать, и 0 в остальных местах.
                    uint64_t mask_val_64 = (bits_to_write_in_step == VALUE_COUNT_BIT) ? ~0ULL : (1ULL << bits_to_write_in_step) - 1;
                    uint64_t mask_shifted_64 = mask_val_64 << bit_offset_in_word; // Сдвигаем маску на нужное смещение

                    // Читаем текущее слово как uint64_t, модифицируем и записываем обратно
                    uint64_t word_64 = data[word_idx];

                    // Очищаем биты в целевом слове, используя инвертированную маску
                    word_64 &= ~mask_shifted_64;

                    // Устанавливаем новые биты, сдвигая кусок значения на нужное смещение
                    word_64 |= static_cast<uint64_t>(chunk_from_value) << bit_offset_in_word; // static_cast на случай, если chunk_from_value был только int

                    // Записываем модифицированное слово обратно в хранилище
                    // Приведение к value_type автоматически отбросит старшие биты, если VALUE_COUNT_BIT < 64
                    data[word_idx] = static_cast<value_type>(word_64);

                    // Обновляем счетчики и позицию
                    bits_processed += bits_to_write_in_step;
                    current_bit_in_storage += bits_to_write_in_step;
                }
            }    
        };
    }

    class FactorialArithmetic : public IntegralBase<FactorialArithmetic> {
    public:
        // --- Конструкторы ---
        constexpr FactorialArithmetic() : _storage() {}
        FactorialArithmetic(const char* value) : FactorialArithmetic(std::string_view(value)) {}
        FactorialArithmetic(std::string_view value);

        // --- Неявный конструктор из целого (для операций +int и т.д.) ---
        template<typename _Ty, typename = std::enable_if_t<std::is_integral<_Ty>::value && !std::is_same_v<_Ty, bool>>>
        constexpr FactorialArithmetic(_Ty value) noexcept {
            sign((value < 0));
            if (value == 0) {
                _storage.push_back(0);
                return;
            }


            using FA = internal::FactorAccess;
            using UnsignedTy = std::make_unsigned_t<_Ty>;
            UnsignedTy abs_value = static_cast<UnsignedTy>(sign() ? -value : value);

            uint64_t divisor = 1;
            size_t idx = 1;

            while (abs_value > 0) {
                divisor *= idx;
                uint64_t remainder = abs_value % idx;
                FA::put(_storage, idx - 1, remainder);  // записывает коэфицент
                abs_value /= idx;
                ++idx;
            }

            trim_leading_zeros();
        }

        // --- Неявное приведение к integral (если нужно) ---
        template<typename _Ty, typename = std::enable_if_t<std::is_integral<_Ty>::value && !std::is_same_v<_Ty, bool>>>
        constexpr explicit operator _Ty() const {
            if (is_zero()) return _Ty(0);

            using FA = internal::FactorAccess;
            using UnsignedTy = std::make_unsigned_t<_Ty>;
            UnsignedTy result = 0;
            UnsignedTy factorial = 1;

            // Максимум 21 позиция (0..20)
            constexpr size_t max_index = 21;

            for (size_t idx = 0; idx < max_index; ++idx) {
                auto digit_opt = FA::extract(_storage, idx); // читает коэфицент 
                if (!digit_opt.has_value()) break;

                UnsignedTy digit = static_cast<UnsignedTy>(digit_opt.value());

                // Проверяем не выйдет ли за пределы типа
                if (digit > 0 && result > std::numeric_limits<UnsignedTy>::max() - digit * factorial) {
                    throw std::overflow_error("Value exceeds the bit width of the target integral type");
                }

                result += digit * factorial;

                // Обновляем факториал (i+1)!
                if (idx < max_index - 1) {
                    if (factorial > std::numeric_limits<UnsignedTy>::max() / (idx + 1)) {
                        break; // дальше не безопасно считать факториал
                    }
                    factorial *= (idx + 1);
                }
            }

            if constexpr (std::is_unsigned_v<_Ty>) {
                return static_cast<_Ty>(result);
            }
            else {
                auto signed_val = static_cast<std::make_signed_t<UnsignedTy>>(result);
                return sign() ? static_cast<_Ty>(-signed_val) : static_cast<_Ty>(signed_val);
            }
        }
        
        template<typename _Ty, typename = std::enable_if_t<std::is_integral<_Ty>::value>>
        FactorialArithmetic& operator=(_Ty value) {
            *this = FactorialArithmetic(value);
            return *this;
        }

        [[nodiscard]] int compare(const FactorialArithmetic& other) const noexcept;
        [[nodiscard]] FactorialArithmetic add(const FactorialArithmetic& other) const;
        [[nodiscard]] FactorialArithmetic divide(const FactorialArithmetic& other) const;
        [[nodiscard]] FactorialArithmetic modulo(const FactorialArithmetic& other) const;
        [[nodiscard]] FactorialArithmetic subtract(const FactorialArithmetic& other) const;
        [[nodiscard]] FactorialArithmetic multiply(const FactorialArithmetic& other) const;

        inline void sign(bool s) noexcept { _storage.sign(s); }
        [[nodiscard]] inline bool sign() const noexcept { return _storage.sign(); }
        [[nodiscard]] inline explicit operator bool() const noexcept { return !this->is_zero(); }

        friend std::string to_string(const FactorialArithmetic& other);
    private:
        using value_type = uint8_t;
        impl::Storage<value_type> _storage;
        bool is_zero() const noexcept;           // тут просто проверка «весь вектор == {0}»
        void trim_leading_zeros() noexcept;      // тут реальное pop_back, убирающее лишние нули
    };


}