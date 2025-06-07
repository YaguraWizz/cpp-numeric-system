#include "gtest/gtest.h"
#include "BinaryArithmetic.h"
#include "FactorialArithmetic.h"


namespace numsystem {

#define SKIPPING(_Ty) if constexpr (std::is_same_v<TypeParam, _Ty>) GTEST_SKIP() << "Skipping: arithmetic operators not fully implemented.";


    TEST(FactorialArithmetic, LocaleTest) {
        FactorialArithmetic valueI(10);
        FactorialArithmetic valueS("10");
        auto sv1 = to_string(valueI);
        auto sv2 = to_string(valueS);
        auto sv3 = std::to_string(10);
        EXPECT_EQ(to_string(valueI), std::to_string(10));
        EXPECT_EQ(to_string(valueS), std::to_string(10));
        EXPECT_EQ(valueI, valueS);
    }

    template<typename _Ty, typename = std::enable_if_t<std::is_integral<_Ty>::value>>
    struct type_info {
        static constexpr _Ty max = std::numeric_limits<_Ty>::max();
        static constexpr _Ty min = std::numeric_limits<_Ty>::min();
    };

    template<typename T>
    inline constexpr bool always_false = false;

    template <typename T>
    class INumericTest : public ::testing::Test {};

    using AllTypes = ::testing::Types<BinaryArithmetic , FactorialArithmetic>;
    TYPED_TEST_SUITE(INumericTest, AllTypes);

    TYPED_TEST(INumericTest, ConstructionAndRepresentation) {
        const auto integral_test_case = [](auto value) {
            using ValueType = decltype(value);
            
            TypeParam from_value(value);
            TypeParam from_string(std::to_string(value));

            // 1. Проверяем, что to_string() корректно представляет число,
            //    независимо от способа инициализации.
            EXPECT_EQ(to_string(from_value), std::to_string(value))
                << "to_string(from_value) mismatch for value: " << value;
            EXPECT_EQ(to_string(from_string), std::to_string(value))
                << "to_string(from_string) mismatch for string: " << std::to_string(value);

            // 2. Проверяем, что конвертация обратно в примитивный тип работает корректно.
            //    Это также подтверждает, что внутреннее представление верное.
            if constexpr (std::is_same_v<ValueType, int> || std::is_same_v<ValueType, int64_t>) {
                EXPECT_EQ(static_cast<int64_t>(from_value), static_cast<int64_t>(value))
                    << "static_cast<int64_t> mismatch for value: " << value;
                EXPECT_EQ(static_cast<int64_t>(from_string), static_cast<int64_t>(value))
                    << "static_cast<int64_t> mismatch for string: " << std::to_string(value);
            }
            else if constexpr (std::is_same_v<ValueType, uint64_t> || std::is_same_v<ValueType, uint32_t>) {
                EXPECT_EQ(static_cast<uint64_t>(from_value), static_cast<uint64_t>(value))
                    << "static_cast<uint64_t> mismatch for value: " << value;
              
                EXPECT_EQ(static_cast<uint64_t>(from_string), static_cast<uint64_t>(value))
                    << "static_cast<uint64_t> mismatch for string: " << std::to_string(value);
            }
        };
        const auto string_integral_test_case = [](std::string_view value) {
            TypeParam from_value(value);
            EXPECT_EQ(to_string(from_value), value)
                << "to_string(from_value) mismatch for string: " << value;
        };

        integral_test_case(0);
        integral_test_case(type_info<uint32_t>::max);
        integral_test_case(type_info<uint32_t>::min);
        integral_test_case(type_info<uint64_t>::max);
        integral_test_case(type_info<uint64_t>::min);
        integral_test_case(type_info<int32_t>::max);
        integral_test_case(type_info<int32_t>::min);
        integral_test_case(type_info<int64_t>::max);
        integral_test_case(type_info<int64_t>::min);  

        string_integral_test_case(std::string(50, '9'));
        string_integral_test_case('1' + std::string(50, '0'));
        string_integral_test_case('1' + std::string(100, '9'));
    }
    TYPED_TEST(INumericTest, CopyConstructor) {
        TypeParam a("12345678901234567890");
        TypeParam b = a;
        EXPECT_EQ(a, b);
        // Убедимся, что это не поверхностная копия
        b += 1;
        EXPECT_NE(a, b);
        EXPECT_EQ(a.sign(), b.sign());
    }
    TYPED_TEST(INumericTest, MoveConstructor) {
        TypeParam a("12345678901234567890");
        std::string a_str = to_string(a);
        TypeParam b = std::move(a);
        EXPECT_EQ(to_string(b), a_str);
        EXPECT_EQ(a.sign(), b.sign());
        // Проверить, что 'a' теперь в валидном, но неопределенном состоянии (обычно пустое)
        // или что оно стало нулем, если это определено для BinaryArithmetic.
        // Например, EXPECT_EQ(to_string(a), "0"); если move оставляет исходный объект в нулевом состоянии
    }    
    TYPED_TEST(INumericTest, ComparisonOperators) {
        const auto test_comparison_operators = [](auto v1, auto v2, auto v3) {
            using ValueType = std::decay_t<decltype(v1)>;

            // Проверка порядка (общая для чисел и строк)
            auto check_order = [&](const TypeParam& x, const TypeParam& y) {
                EXPECT_TRUE(x < y) << "test_comparison_operators.check_order 1 ";
                EXPECT_TRUE(y > x) << "test_comparison_operators.check_order 2 ";
                EXPECT_TRUE(x <= y) << "test_comparison_operators.check_order 3 ";
                EXPECT_TRUE(y >= x) << "test_comparison_operators.check_order 4 ";
                };

            // Проверка равенства/неравенства
            auto check_equality = [&](const TypeParam& x, const TypeParam& y) {
                EXPECT_TRUE(x == x) << "test_comparison_operators.check_equality 1";
                EXPECT_TRUE(y == y) << "test_comparison_operators.check_equality 2";
                EXPECT_TRUE(x != y) << "test_comparison_operators.check_equality 3";
                };

            // Проверка эквивалентности представлений: число ↔ строка
            auto check_equivalence = [&](const TypeParam& from_val, const TypeParam& from_str) {
                EXPECT_TRUE(from_val == from_str) << "test_comparison_operators.check_equivalence 1";
                EXPECT_FALSE(from_val != from_str) << "test_comparison_operators.check_equivalence 2";
                };

            if constexpr (std::is_integral_v<ValueType>) {
                // Конструируем как из числа, так и из строки
                TypeParam a(v1), b(v2), c(v3);
                TypeParam a_str(std::to_string(v1));
                TypeParam b_str(std::to_string(v2));
                TypeParam c_str(std::to_string(v3));

                check_order(a, b); 
                check_order(b, c);
                check_order(a, c);

                check_equality(a, b);
                check_equality(b, c);
                check_equality(a, c);

                check_equivalence(a, a_str);
                check_equivalence(b, b_str);
                check_equivalence(c, c_str);

                check_order(a_str, b_str);
                check_order(b_str, c_str);
                check_order(a_str, c_str);

                EXPECT_TRUE(c == c_str) << "test_comparison_operators";
            }
            else if constexpr (std::is_convertible_v<ValueType, std::string_view>) {
                // Только строковые сравнения
                TypeParam a(v1), b(v2), c(v3);

                check_order(a, b);
                check_order(b, c);
                check_order(a, c);

                check_equality(a, b);
                check_equality(b, c);
                check_equality(a, c);
            }
            else {
                static_assert(always_false<ValueType>, "Unsupported type for comparison test");
            }
        };

        // Целочисленные тесты (полные проверки)
        test_comparison_operators(
            type_info<uint64_t>::max - 2,
            type_info<uint64_t>::max - 1,
            type_info<uint64_t>::max
        );

        test_comparison_operators(
            type_info<int64_t>::min,
            type_info<int64_t>::min + 1,
            type_info<int64_t>::min + 2
        );

        // Только строковые тесты (без преобразований туда-обратно)
        test_comparison_operators(
            std::string(50, '1'),
            std::string(50, '2'),
            std::string(50, '3')
        );
    }
    TYPED_TEST(INumericTest, UnaryOperators) {
        const auto test_unary_ops = [&](auto raw) {
            TypeParam from_val(raw);
            TypeParam from_str(std::to_string(raw));

            // +a: знак не должен измениться
            EXPECT_EQ((+from_val).sign(), from_val.sign()) << "test_unary_ops (+av)";
            EXPECT_EQ((+from_str).sign(), from_str.sign()) << "test_unary_ops (+as)";

            // -a: знак должен быть противоположным
            EXPECT_NE((-from_val).sign(), from_val.sign()) << "test_unary_ops (-av)";
            EXPECT_NE((-from_str).sign(), from_str.sign()) << "test_unary_ops (-as)";

            // !a: true только если значение равно нулю
            EXPECT_NE(static_cast<bool>(!from_val), static_cast<bool>(from_str)) << "test_unary_ops (!av)";
            EXPECT_NE(static_cast<bool>(!from_str), static_cast<bool>(from_str)) << "test_unary_ops (!as)";
        };
        const auto test_unary_ops_str = [&](std::string_view rawstr) {
            TypeParam from_str(rawstr);

            // +a: знак не должен измениться
            EXPECT_EQ((+from_str).sign(), from_str.sign()) << "test_unary_ops_str (+as)";

            // -a: знак должен быть противоположным
            EXPECT_NE((-from_str).sign(), from_str.sign()) << "test_unary_ops_str (-as)";

            // !a: true только если значение равно нулю
            EXPECT_NE(static_cast<bool>(!from_str), static_cast<bool>(from_str)) << "test_unary_ops_str (!as)";
        };

        // Числовые значения (примитивы)
        test_unary_ops(5);
        test_unary_ops(0);
        test_unary_ops(-8);

        // Строки, представляющие числа
        test_unary_ops_str("5");
        test_unary_ops_str("0");
        test_unary_ops_str("-8");

        // Большие строки
        test_unary_ops_str("1234567890123456789012345678901234567890");
        test_unary_ops_str("-9876543210987654321098765432109876543210");
        test_unary_ops_str("0");
        test_unary_ops_str("-0");
    }
    TYPED_TEST(INumericTest, IncrementDecrement) {
        const auto test_inc_dec = [](auto raw) {
            using IntT = decltype(raw);

            // Тест для инициализации от числа
            {
                TypeParam value(raw);
                EXPECT_EQ(static_cast<IntT>(value), raw) << "test_inc_dec";

                ++value;
                EXPECT_EQ(static_cast<IntT>(value), raw + 1) << "test_inc_dec";

                value++;
                EXPECT_EQ(static_cast<IntT>(value), raw + 2) << "test_inc_dec";

                --value;
                EXPECT_EQ(static_cast<IntT>(value), raw + 1) << "test_inc_dec";

                value--;
                EXPECT_EQ(static_cast<IntT>(value), raw) << "test_inc_dec";
            }

            // Тест для инициализации от строки
            {
                TypeParam value(std::to_string(raw));
                EXPECT_EQ(std::to_string(static_cast<IntT>(value)), std::to_string(raw));

                ++value;
                EXPECT_EQ(static_cast<IntT>(value), raw + 1) << "test_inc_dec";

                value++;
                EXPECT_EQ(static_cast<IntT>(value), raw + 2) << "test_inc_dec";

                --value;
                EXPECT_EQ(static_cast<IntT>(value), raw + 1) << "test_inc_dec";

                value--;
                EXPECT_EQ(static_cast<IntT>(value), raw) << "test_inc_dec";
            }
        };

        test_inc_dec(10);     // положительное значение
        test_inc_dec(0);      // ноль
        test_inc_dec(-5);     // отрицательное значение
    }
    TYPED_TEST(INumericTest, DivisionByZero) {
        TypeParam a(42);
        TypeParam zero(0);
        TypeParam a_str(std::to_string(42));
        TypeParam zero_str("0");

        // Числовая и строковая инициализация должны вести себя одинаково
        EXPECT_THROW((a / zero), std::overflow_error);
        EXPECT_THROW((a % zero), std::overflow_error);
        EXPECT_THROW((a_str / zero_str), std::overflow_error);
        EXPECT_THROW((a_str % zero_str), std::overflow_error);
    } 

    TYPED_TEST(INumericTest, ArithmeticOperators) {
        //SKIPPING(FactorialArithmetic)
        const auto test_arithmetic_small = [](auto raw_a, auto raw_b) {
            using IntT = decltype(raw_a);
            TypeParam a(raw_a), b(raw_b);

            // Testing addition
            {  
                TypeParam tmp = a;
                EXPECT_EQ(to_string(tmp + b), std::to_string(IntT(raw_a + raw_b)))
                    << "Addition (tmp + b): a=" << raw_a << ", b=" << raw_b;
                tmp += b;
                EXPECT_EQ(to_string(tmp), std::to_string(IntT(raw_a + raw_b)))
                    << "Addition (tmp += b): a=" << raw_a << ", b=" << raw_b;
            }

            // Testing subtraction
            {
                TypeParam tmp = a;
                EXPECT_EQ(to_string(tmp - b), std::to_string(IntT(raw_a - raw_b)))
                    << "Subtraction (tmp - b): a=" << raw_a << ", b=" << raw_b;
                tmp -= b;
                EXPECT_EQ(to_string(tmp), std::to_string(IntT(raw_a - raw_b)))
                    << "Subtraction (tmp -= b): a=" << raw_a << ", b=" << raw_b;
            }

            // Testing multiplication
            {
                TypeParam tmp = a;
                EXPECT_EQ(to_string(tmp * b), std::to_string(IntT(raw_a * raw_b)))
                    << "Multiplication (tmp * b): a=" << raw_a << ", b=" << raw_b;
                tmp *= b;
                EXPECT_EQ(to_string(tmp), std::to_string(IntT(raw_a * raw_b)))
                    << "Multiplication (tmp *= b): a=" << raw_a << ", b=" << raw_b;
            }

            // Testing division and remainder (only if b ≠ 0)
            if (raw_b != 0) {
               
                {
                    TypeParam tmp = a;
                    EXPECT_EQ(to_string(tmp / b), std::to_string(IntT(raw_a / raw_b)))
                        << "Division (tmp / b): a=" << raw_a << ", b=" << raw_b;
                    tmp /= b;
                    EXPECT_EQ(to_string(tmp), std::to_string(IntT(raw_a / raw_b)))
                        << "Division (tmp /= b): a=" << raw_a << ", b=" << raw_b;
                }

                {
                    TypeParam tmp = a;
                    EXPECT_EQ(to_string(tmp % b), std::to_string(IntT(raw_a % raw_b)))
                        << "Remainder (tmp % b): a=" << raw_a << ", b=" << raw_b;
                    tmp %= b;
                    EXPECT_EQ(to_string(tmp), std::to_string(IntT(raw_a % raw_b)))
                        << "Remainder (tmp %= b): a=" << raw_a << ", b=" << raw_b;
                }
            }
        };

        const auto test_arithmetic_big = [](auto s_a, auto s_b) {
            TypeParam a(s_a), b(s_b);

            // (a + b) == (a += b)
            {
                TypeParam tmp1 = a, tmp2 = a;
                EXPECT_EQ(to_string(tmp1 + b), to_string(tmp2 += b))
                    << "Big numbers: addition (a + b) vs (a += b). a=" << s_a << ", b=" << s_b;
            }

            // (a - a) == 0
            {
                TypeParam tmp1 = a, tmp2 = a;
                EXPECT_EQ(to_string(tmp1 - a), "0")
                    << "Big numbers: subtraction (a - a). a=" << s_a;
                tmp2 -= a;
                EXPECT_EQ(to_string(tmp2), "0")
                    << "Big numbers: subtraction (a -= a). a=" << s_a;
            }

            // (a * b) == (a *= b)
            {
                TypeParam tmp1 = a, tmp2 = a;
                EXPECT_EQ(to_string(tmp1 * b), to_string(tmp2 *= b))
                    << "Big numbers: multiplication (a * b) vs (a *= b). a=" << s_a << ", b=" << s_b;
            }

            // Division and remainder — only if b ≠ 0 (for TypeParam)
            // Check that b is not zero (or the string "0")
            if (to_string(b).find_first_not_of('0') != std::string::npos) {
                {
                    TypeParam tmp1 = a, tmp2 = a; // Using 'a' for tmp1/tmp2 as your original code did b / a
                    EXPECT_EQ(to_string(tmp1 / b), to_string(tmp2 /= b))
                        << "Big numbers: division (a / b) vs (a /= b). a=" << s_a << ", b=" << s_b;
                }
                {
                    TypeParam tmp1 = a, tmp2 = a; // Similarly, using 'a'
                    EXPECT_EQ(to_string(tmp1 % b), to_string(tmp2 %= b))
                        << "Big numbers: remainder (a % b) vs (a %= b). a=" << s_a << ", b=" << s_b;
                }
            }
            else {
                // Can add a message if b is zero and the test was skipped
                // std::cout << "Skipping division/remainder: b is zero. a=" << s_a << ", b=" << s_b << std::endl;
            }

            // Testing division by itself (only if a ≠ 0)
            if (to_string(a).find_first_not_of('0') != std::string::npos) {
                TypeParam tmp = a;
                EXPECT_EQ(to_string(tmp / tmp), "1")
                    << "Big numbers: division by self (tmp / tmp). tmp=" << to_string(tmp);
                EXPECT_EQ(to_string(tmp % tmp), "0")
                    << "Big numbers: remainder of division by self (tmp % tmp). tmp=" << to_string(tmp);
            }
            else {
                // std::cout << "Skipping division by self: a is zero. a=" << s_a << std::endl;
            }
        };

        // Small numbers
        test_arithmetic_small(6, 3);
        test_arithmetic_small(10, -5);
        test_arithmetic_small(-10, -3);
        test_arithmetic_small(0, 5);
        test_arithmetic_small(5, 0); 

        // ---- BIG NUMBERS ----
        const std::vector<size_t> sizes = { 20, 50, 100 };
        for (size_t len : sizes) {
            test_arithmetic_big(std::string(len, '1'), std::string(len, '9'));
            // Add more variations for big numbers if needed
            test_arithmetic_big(std::string(len, '9'), std::string(len, '1'));
            test_arithmetic_big(std::string(len, '5'), std::string(len, '2'));
            test_arithmetic_big("1" + std::string(len - 1, '0'), "2"); // Example: 100...000 / 2
            test_arithmetic_big("2", "1" + std::string(len - 1, '0')); // Example: 2 / 100...000
        }
    }

    TYPED_TEST(INumericTest, AbsFunction) {
        // Test abs(positive)
        TypeParam positive("12345");
        EXPECT_EQ(abs(positive), TypeParam("12345"));

        // Test abs(negative)
        TypeParam negative("-98765");
        EXPECT_EQ(abs(negative), TypeParam("98765"));

        // Test abs(zero)
        TypeParam zero("0");
        EXPECT_EQ(abs(zero), TypeParam("0"));

        TypeParam large_pos("10000000000000000000000000000000000000000000000000");
        TypeParam large_neg("-20000000000000000000000000000000000000000000000000");
        EXPECT_EQ(abs(large_pos), large_pos);
        EXPECT_EQ(abs(large_neg), TypeParam("20000000000000000000000000000000000000000000000000"));
    }
    TYPED_TEST(INumericTest, PowFunction) {
        //SKIPPING(FactorialArithmetic)

        // Base positive, exponent positive
        EXPECT_EQ(pow(TypeParam(2), 0), TypeParam(1));
        EXPECT_EQ(pow(TypeParam(2), 1), TypeParam(2));
        EXPECT_EQ(pow(TypeParam(2), 3), TypeParam(8));
        EXPECT_EQ(pow(TypeParam(5), 4), TypeParam(625));

        // Base negative, exponent even
        EXPECT_EQ(pow(TypeParam(-2), 2), TypeParam(4));
        EXPECT_EQ(pow(TypeParam(-3), 4), TypeParam(81));

        // Base negative, exponent odd
        EXPECT_EQ(pow(TypeParam(-2), 1), TypeParam(-2));
        EXPECT_EQ(pow(TypeParam(-2), 3), TypeParam(-8));
        EXPECT_EQ(pow(TypeParam(-5), 3), TypeParam(-125));

        // Base 0, exponent positive
        EXPECT_EQ(pow(TypeParam(0), 1), TypeParam(0));
        EXPECT_EQ(pow(TypeParam(0), 5), TypeParam(0));

        // Base X, exponent 0
        EXPECT_EQ(pow(TypeParam(12345), 0), TypeParam(1));
        EXPECT_EQ(pow(TypeParam(-678), 0), TypeParam(1));
        EXPECT_EQ(pow(TypeParam(0), 0), TypeParam(1)); // Стандартное определение 0^0 = 1

        // Base 1, exponent X
        EXPECT_EQ(pow(TypeParam(1), 100), TypeParam(1));

        // Base -1, exponent even/odd
        EXPECT_EQ(pow(TypeParam(-1), 10), TypeParam(1));
        EXPECT_EQ(pow(TypeParam(-1), 11), TypeParam(-1));

        // Larger numbers/exponents
        TypeParam base_large("1000"); // 10^3
        TypeParam exp_large_result(("1" + std::string(90, '0'))); // 10^3 * 30 = 10^90
        EXPECT_EQ(pow(base_large, 30), exp_large_result);

        TypeParam large_base("123456789");
        EXPECT_EQ(pow(large_base, 2), large_base * large_base);
    }
    TYPED_TEST(INumericTest, SqrtFunction) {
        //SKIPPING(FactorialArithmetic)
         
        // Perfect squares
        EXPECT_EQ(sqrt(TypeParam(0)), TypeParam(0));
        EXPECT_EQ(sqrt(TypeParam(1)), TypeParam(1));
        EXPECT_EQ(sqrt(TypeParam(4)), TypeParam(2));
        EXPECT_EQ(sqrt(TypeParam(9)), TypeParam(3));
        EXPECT_EQ(sqrt(TypeParam(100)), TypeParam(10));
        EXPECT_EQ(sqrt(TypeParam(625)), TypeParam(25));
        EXPECT_EQ(sqrt(TypeParam(1000000)), TypeParam(1000));
        EXPECT_EQ(
            sqrt(
                TypeParam("12345678901234567890123456789012345678900000000000000000000000000000000000000000000000000000000000000")),
                TypeParam("111111110611111109936111105818611081081542864454310")
        ); // sqrt(1.2345... * 10^99) ~ 1.1111... * 10^49
        
        TypeParam large_perfect_square("1" + std::string(48, '0')); // 10^48
        TypeParam large_root("1" + std::string(24, '0')); // 10^24
        EXPECT_EQ(sqrt(large_perfect_square), large_root);

        // Non-perfect squares (integer square root should return floor)
        EXPECT_EQ(sqrt(TypeParam(2)), TypeParam(1));
        EXPECT_EQ(sqrt(TypeParam(3)), TypeParam(1));
        EXPECT_EQ(sqrt(TypeParam(8)), TypeParam(2));
        EXPECT_EQ(sqrt(TypeParam(15)), TypeParam(3));
        EXPECT_EQ(sqrt(TypeParam(99)), TypeParam(9));

        // Test sqrt of negative value (should throw std::domain_error)
        TypeParam negative_value("-1");
        EXPECT_THROW(sqrt(negative_value), std::domain_error);
        EXPECT_THROW(sqrt(TypeParam("-123")), std::domain_error);
    }

    // Тест-кейс для проверки конкретного шага деления в факториальной конвертации
    TEST(BinaryArithmeticTest, DivisionForFactorialConversionStep3) {
        // GTEST_SKIP() << "Skipping: arithmetic operators not fully implemented.";
        // Согласно логу пользователя, на 3-м шаге (деление на 3)
        // число было 65550.
        // Ожидаемый результат деления 65550 на 3:
        // 65550 / 3 = 21850
        // 65550 % 3 = 0

        BinaryArithmetic num(65550);
        BinaryArithmetic divisor(3);

        // Выполняем деление и остаток с использованием вашего класса
        BinaryArithmetic actual_quotient = num / divisor;
        BinaryArithmetic actual_remainder = num % divisor;

        // Определяем ожидаемые корректные результаты
        BinaryArithmetic expected_quotient(21850);
        BinaryArithmetic expected_remainder(0);

        // Проверяем, совпадает ли фактическое частное с ожидаемым
        ASSERT_EQ(expected_quotient, actual_quotient)
            << "Error: Incorrect quotient when dividing 65550 / 3";

        // Проверяем, совпадает ли фактический остаток с ожидаемым
        ASSERT_EQ(expected_remainder, actual_remainder)
            << "Error: Incorrect remainder when dividing 65550 % 3";
    }

    // Дополнительный тест для следующего шага, который использует правильное частное
    // Если предыдущий тест прошел, этот проверит деление 21850 на 4
    TEST(BinaryArithmeticTest, DivisionForFactorialConversionStep4_CorrectInput) {
        // GTEST_SKIP() << "Skipping: arithmetic operators not fully implemented.";
        // Если деление 65550 / 3 было бы верным (21850), то следующий шаг
        // факториальной конвертации был бы деление 21850 на 4.
        // 21850 / 4 = 5462
        // 21850 % 4 = 2

        BinaryArithmetic num(21850);
        BinaryArithmetic divisor(4);

        BinaryArithmetic actual_quotient = num / divisor;
        BinaryArithmetic actual_remainder = num % divisor;

        BinaryArithmetic expected_quotient(5462);
        BinaryArithmetic expected_remainder(2);

        ASSERT_EQ(expected_quotient, actual_quotient)
            << "Error: Incorrect quotient when dividing 21850 / 4";

        ASSERT_EQ(expected_remainder, actual_remainder)
            << "Error: Incorrect quotient when dividing 21850 % 4";
    }

    TEST(FactorAccess, CountBits) {
        using FA = internal::FactorAccess;
        EXPECT_EQ(FA::count_bits(0), 0);
        EXPECT_EQ(FA::count_bits(1), 1);
        EXPECT_EQ(FA::count_bits(2), 2);
        EXPECT_EQ(FA::count_bits(3), 2);
        EXPECT_EQ(FA::count_bits(255), 8);
        EXPECT_EQ(FA::count_bits(256), 9);
        EXPECT_EQ(FA::count_bits(UINT64_MAX), 64);
    }
    TEST(FactorAccess, Log2Floor) {
        using FA = internal::FactorAccess;
        EXPECT_EQ(FA::log2_floor(0), 0);
        EXPECT_EQ(FA::log2_floor(1), 0);
        EXPECT_EQ(FA::log2_floor(2), 1);
        EXPECT_EQ(FA::log2_floor(3), 1);
        EXPECT_EQ(FA::log2_floor(4), 2);
        EXPECT_EQ(FA::log2_floor(1023), 9);
        EXPECT_EQ(FA::log2_floor(1024), 10);
    }
    TEST(FactorAccess, SafeShiftLeft) {
        using FA = internal::FactorAccess;
        EXPECT_EQ(FA::safe_shift_left(1, 0), 1);
        EXPECT_EQ(FA::safe_shift_left(1, 1), 2);
        EXPECT_EQ(FA::safe_shift_left(1, 63), 1ULL << 63);
        EXPECT_EQ(FA::safe_shift_left(1, 64), 0);
        EXPECT_EQ(FA::safe_shift_left(1, 65), 0);
    }
    TEST(FactorAccess, TotalBitsUpToValid) {
        using FA = internal::FactorAccess;
        EXPECT_EQ(FA::total_bits_up_to(0), 0);
        EXPECT_EQ(FA::total_bits_up_to(1), 0);
        EXPECT_EQ(FA::total_bits_up_to(2), 1);  // Минимальное значение N=1
        EXPECT_EQ(FA::total_bits_up_to(4), 5);  // Проверка вручную: N=3, M=1, pow2=4
        EXPECT_NO_THROW(FA::total_bits_up_to(FA::MAXINDEX));
    }
    TEST(FactorAccess, TotalBitsUpToThrows) {
        using FA = internal::FactorAccess;
        EXPECT_THROW(FA::total_bits_up_to(FA::MAXINDEX + 1), std::invalid_argument);
    }
    TEST(FactorAccess, PutAndExtractSimple) {
        using FA = internal::FactorAccess;
        using Storage = impl::Storage<uint64_t>;

        Storage storage;

        // Записываем и извлекаем 1 по индексу 1 (value <= index)
        FA::put(storage, 1, 1);
        auto result1 = FA::extract(storage, 1);
        ASSERT_TRUE(result1.has_value());
        EXPECT_EQ(result1.value(), 1);

        // Максимально допустимое значение для индекса 4 — это 4
        FA::put(storage, 4, 4);
        auto result2 = FA::extract(storage, 4);
        ASSERT_TRUE(result2.has_value());
        EXPECT_EQ(result2.value(), 4);

        // Нулевое значение всегда допустимо
        FA::put(storage, 5, 0);
        auto result3 = FA::extract(storage, 5);
        ASSERT_TRUE(result3.has_value());
        EXPECT_EQ(result3.value(), 0);
    }
    TEST(FactorAccess, PutRejectsInvalidValue) {
        using FA = internal::FactorAccess;
        using Storage = impl::Storage<uint64_t>;

        Storage storage;

        // Попытка записать значение больше индекса должна выбросить исключение
        EXPECT_THROW(FA::put(storage, 3, 4), std::logic_error);  // 4 > 3 — недопустимо
        EXPECT_THROW(FA::put(storage, 10, 100), std::logic_error);
    }
    TEST(FactorAccess, PutAndExtractWideRangeValid) {
        using FA = internal::FactorAccess;
        using Storage = impl::Storage<uint64_t>;

        Storage storage;

        // Проверяем запись и чтение всех допустимых значений value -> [0, index]
        for (size_t index = 1; index <= 100; ++index) {
            uint64_t value = index / 2; // Всегда value <= index
            FA::put(storage, index, value);
            auto result = FA::extract(storage, index);
            ASSERT_TRUE(result.has_value()) << "Index: " << index;
            EXPECT_EQ(result.value(), value) << "Index: " << index;
        }
    }
}
