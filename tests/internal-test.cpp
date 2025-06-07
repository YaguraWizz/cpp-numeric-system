#include "gtest/gtest.h"	
#include "Internal.h"

namespace numsystem {
    namespace {
        using OverflowOps = impl::OverflowAwareOps;

        // Вспомогательные constexpr функции для тестов
        template<typename T>
        constexpr std::pair<T, T> constexpr_sum(T a, T b, T carry_in) {
            T carry = carry_in;
            T result = OverflowOps::sum<T>(a, b, carry);
            return { result, carry };
        }

        template<typename T>
        constexpr std::pair<T, T> constexpr_subtract(T a, T b, T borrow_in) {
            T borrow = borrow_in;
            T result = OverflowOps::subtract<T>(a, b, borrow);
            return { result, borrow };
        }
    }



    TEST(StateInfoTest, DefaultConstructor) {
        impl::StateInfo s;
        EXPECT_EQ(s.value(), 0);
        EXPECT_FALSE(s.sign());
    }
    TEST(StateInfoTest, ValueAndSignSetGet) {
        uint64_t val = 123456789;
        bool sign = true;

        impl::StateInfo s(val, sign);
        EXPECT_EQ(s.value(), val);
        EXPECT_TRUE(s.sign());

        s.value(987654321);
        s.sign(false);

        EXPECT_EQ(s.value(), 987654321);
        EXPECT_FALSE(s.sign());
    }
    TEST(StateInfoTest, SignBitManipulation) {
        impl::StateInfo s;
        s.value(0xFFFFFFFFFFFFFFF0ULL);

        s.sign(true);
        EXPECT_TRUE(s.sign());
        EXPECT_EQ(s.value(), 0x7FFFFFFFFFFFFFF0ULL); // только младшие 63 бита

        s.sign(false);
        EXPECT_FALSE(s.sign());
        EXPECT_EQ(s.value(), 0x7FFFFFFFFFFFFFF0ULL); // то же самое
    }
    TEST(StorageTest, DefaultEmpty) {
        impl::Storage<int> store;
        EXPECT_EQ(store.size(), 0);
        EXPECT_TRUE(store.empty());
        EXPECT_EQ(store.value(), 0);
        EXPECT_FALSE(store.sign());
    }
    TEST(StorageTest, ValueAndSignProxy) {
        impl::Storage<int> store;
        store.value(42);
        store.sign(true);

        EXPECT_EQ(store.value(), 42);
        EXPECT_TRUE(store.sign());

        store.sign(false);
        EXPECT_FALSE(store.sign());
    }
    TEST(StorageTest, VectorOperations) {
        impl::Storage<int> store;

        store.push_back(10);
        store.push_back(20);
        store.push_back(30);

        EXPECT_EQ(store.size(), 3);
        EXPECT_FALSE(store.empty());

        EXPECT_EQ(store[0], 10);
        EXPECT_EQ(store[1], 20);
        EXPECT_EQ(store[2], 30);

        store.pop_back();
        EXPECT_EQ(store.size(), 2);
        EXPECT_EQ(store.back(), 20);

        store.clear();
        EXPECT_TRUE(store.empty());
    }
    TEST(StorageTest, ResizeAndReserve) {
        impl::Storage<int> store;
        store.reserve(10);
        store.resize(5, 99);

        EXPECT_EQ(store.size(), 5);
        for (size_t i = 0; i < 5; ++i) {
            EXPECT_EQ(store[i], 99);
        }
    }
    TEST(StorageTest, Iterators) {
        impl::Storage<int> store;
        store.push_back(1);
        store.push_back(2);
        store.push_back(3);

        int sum = 0;
        for (auto it = store.begin(); it != store.end(); ++it) {
            sum += *it;
        }
        EXPECT_EQ(sum, 6);

        sum = 0;
        for (auto val : store) {
            sum += val;
        }
        EXPECT_EQ(sum, 6);
    }
    TEST(StateInfoTest, ConstexprMethods) {
        constexpr impl::StateInfo s1(1234, true);
        static_assert(s1.value() == 1234, "value constexpr check");
        static_assert(s1.sign() == true, "sign constexpr check");
    }
    TEST(StorageTest, ConstexprValueSign) {
        impl::Storage<int> store;
        store.value(42);
        store.sign(true);

        EXPECT_EQ(store.value(), 42);
        EXPECT_TRUE(store.sign());
    }
    TEST(StorageTest, CopyAndMove) {
        impl::Storage<int> a;
        a.value(10);
        a.sign(true);
        a.push_back(1);
        a.push_back(2);

        impl::Storage<int> b = a; // копия
        EXPECT_EQ(b.value(), 10);
        EXPECT_TRUE(b.sign());
        EXPECT_EQ(b.size(), 2);
        EXPECT_EQ(b[0], 1);
        EXPECT_EQ(b[1], 2);

        impl::Storage<int> c = std::move(a); // перемещение
        EXPECT_EQ(c.value(), 10);
        EXPECT_TRUE(c.sign());
        EXPECT_EQ(c.size(), 2);
    }
    TEST(StorageTest, ConstIterators) {
        const impl::Storage<int> store = [] {
            impl::Storage<int> s;
            s.push_back(1);
            s.push_back(2);
            s.push_back(3);
            return s;
            }();

        int sum = 0;
        for (auto it = store.cbegin(); it != store.cend(); ++it) {
            sum += *it;
        }
        EXPECT_EQ(sum, 6);
    }
    TEST(StorageTest, Swap) {
        impl::Storage<int> a, b;
        a.push_back(1);
        b.push_back(2);
        std::swap(a, b);

        EXPECT_EQ(a[0], 2);
        EXPECT_EQ(b[0], 1);
    }


    TEST(OverflowAwareOpsTest, SumNoCarry) {
        constexpr auto res = constexpr_sum<unsigned int>(10, 20, 0);
        unsigned int result = res.first;
        unsigned int carry = res.second;
        EXPECT_EQ(result, 30u);
        EXPECT_EQ(carry, 0u);
    }
    TEST(OverflowAwareOpsTest, SumWithCarryFromPrevious) {
        constexpr auto res = constexpr_sum<unsigned int>(10, 20, 1);
        unsigned int result = res.first;
        unsigned int carry = res.second;
        EXPECT_EQ(result, 31u);
        EXPECT_EQ(carry, 0u);
    }
    TEST(OverflowAwareOpsTest, SumOverflow) {
        constexpr unsigned int max = std::numeric_limits<unsigned int>::max();
        constexpr auto res = constexpr_sum<unsigned int>(max, 1, 0);
        unsigned int result = res.first;
        unsigned int carry = res.second;
        EXPECT_EQ(result, 0u);
        EXPECT_EQ(carry, 1u);
    }
    TEST(OverflowAwareOpsTest, SumOverflowWithCarry) {
        constexpr unsigned int max = std::numeric_limits<unsigned int>::max();
        constexpr auto res = constexpr_sum<unsigned int>(max, 0, 1);
        unsigned int result = res.first;
        unsigned int carry = res.second;
        EXPECT_EQ(result, 0u);
        EXPECT_EQ(carry, 1u);
    }
    TEST(OverflowAwareOpsTest, SubtractNoBorrow) {
        constexpr auto res = constexpr_subtract<unsigned int>(20, 10, 0);
        unsigned int result = res.first;
        unsigned int borrow = res.second;
        EXPECT_EQ(result, 10u);
        EXPECT_EQ(borrow, 0u);
    }
    TEST(OverflowAwareOpsTest, SubtractWithBorrowFromPrevious) {
        constexpr auto res = constexpr_subtract<unsigned int>(20, 10, 1);
        unsigned int result = res.first;
        unsigned int borrow = res.second;
        EXPECT_EQ(result, 9u);
        EXPECT_EQ(borrow, 0u);
    }
    TEST(OverflowAwareOpsTest, SubtractBorrow) {
        constexpr auto res = constexpr_subtract<unsigned int>(0, 1, 0);
        unsigned int result = res.first;
        unsigned int borrow = res.second;
        EXPECT_EQ(result, std::numeric_limits<unsigned int>::max());
        EXPECT_EQ(borrow, 1u);
    }
    TEST(OverflowAwareOpsTest, SubtractBorrowWithPreviousBorrow) {
        constexpr auto res = constexpr_subtract<unsigned int>(0, 0, 1);
        unsigned int result = res.first;
        unsigned int borrow = res.second;
        EXPECT_EQ(result, std::numeric_limits<unsigned int>::max());
        EXPECT_EQ(borrow, 1u);
    }


    TEST(BigNumberOperationsTest, IsIntegralValidString) {
        using BNO = impl::BigNumberOperations;

        EXPECT_TRUE(BNO::is_integral_valid_string("0"));
        EXPECT_TRUE(BNO::is_integral_valid_string("123"));
        EXPECT_TRUE(BNO::is_integral_valid_string("-123"));

        EXPECT_FALSE(BNO::is_integral_valid_string(""));
        EXPECT_FALSE(BNO::is_integral_valid_string("-"));
        EXPECT_FALSE(BNO::is_integral_valid_string("01"));   // ведущий ноль
        EXPECT_FALSE(BNO::is_integral_valid_string("-0123")); // ведущий ноль после минуса
        EXPECT_FALSE(BNO::is_integral_valid_string("12a3"));
    }
    TEST(BigNumberOperationsTest, AddStrings) {
        using BNO = impl::BigNumberOperations;

        EXPECT_EQ(BNO::add_strings("123", "456"), "579");
        EXPECT_EQ(BNO::add_strings("999", "1"), "1000");
        EXPECT_EQ(BNO::add_strings("0", "0"), "0");
    }
    TEST(BigNumberOperationsTest, SubtractStrings) {
        using BNO = impl::BigNumberOperations;

        EXPECT_EQ(BNO::subtract_strings("1000", "1"), "999");
        EXPECT_EQ(BNO::subtract_strings("123", "123"), "0");
        EXPECT_EQ(BNO::subtract_strings("456", "123"), "333");

        // Проверка исключения
        EXPECT_THROW(BNO::subtract_strings("123", "456"), std::runtime_error);
    }
    TEST(BigNumberOperationsTest, MultiplyStrings) {
        using BNO = impl::BigNumberOperations;

        EXPECT_EQ(BNO::multiply_strings("0", "123456"), "0");
        EXPECT_EQ(BNO::multiply_strings("1", "999"), "999");
        EXPECT_EQ(BNO::multiply_strings("123", "456"), "56088");
    }
    TEST(BigNumberOperationsTest, DivideStringByIntegral) {
        using BNO = impl::BigNumberOperations;

        auto [q1, r1] = BNO::divide_string_by_integral("123", 10);
        EXPECT_EQ(q1, "12");
        EXPECT_EQ(r1, 3);

        auto [q2, r2] = BNO::divide_string_by_integral("1000", 10);
        EXPECT_EQ(q2, "100");
        EXPECT_EQ(r2, 0);

        EXPECT_THROW(BNO::divide_string_by_integral("123", 0), std::runtime_error);
    }
    TEST(BigNumberOperationsTest, DivideStrings) {
        using BNO = impl::BigNumberOperations;

        auto [q1, r1] = BNO::divide_strings("123", "10");
        EXPECT_EQ(q1, "12");
        EXPECT_EQ(r1, "3");

        auto [q2, r2] = BNO::divide_strings("123", "123");
        EXPECT_EQ(q2, "1");
        EXPECT_EQ(r2, "0");

        auto [q3, r3] = BNO::divide_strings("123", "200");
        EXPECT_EQ(q3, "0");
        EXPECT_EQ(r3, "123");

        EXPECT_THROW(BNO::divide_strings("123", "0"), std::overflow_error);
    }
    TEST(BigNumberOperationsTest, RemoveLeadingZeros) {
        using BNO = impl::BigNumberOperations;

        std::string str1 = "000123";
        BNO::remove_zeros(str1, BNO::TrimMode::Leading);
        EXPECT_EQ(str1, "123");

        std::string str2 = "0000";
        BNO::remove_zeros(str2, BNO::TrimMode::Leading);
        EXPECT_EQ(str2, "0");

        std::string str3 = "0";
        BNO::remove_zeros(str3, BNO::TrimMode::Leading);
        EXPECT_EQ(str3, "0");
    }



 
}

