#define EXAMPLE_BEGIN_MARK
#define DELETE_DEFINE_MACRO(...) __VA_ARGS__

#ifdef ENABLE_OUTPUT
    #define INSERT_EXAMPLE_BODY(...) __VA_ARGS__
    #define COUT(x) std::cout << x << std::endl
#else
    #define COUT(x)
    #define INSERT_EXAMPLE_BODY(...) 
#endif

#include "gtest/gtest.h" //[DELETE_DEFINE_MACRO]

#include "BinaryArithmetic.h"
#include "FactorialArithmetic.h"
#include <string>
INSERT_EXAMPLE_BODY(#include <iostream>)

namespace numsystem {

DELETE_DEFINE_MACRO(TEST(Example_CrossSystemArithmetic, SumOfLargeNumbers)) {
    INSERT_EXAMPLE_BODY(using namespace numsystem;)
    std::string largeNumStr1 = "123456789012345678901234567890";
    std::string largeNumStr2 = "98765432109876543210987654321";

    BinaryArithmetic binA(largeNumStr1);
    BinaryArithmetic binB(largeNumStr2);
    BinaryArithmetic binSum = binA + binB;
    COUT("Binary Sum:      " << to_string(binSum));

    FactorialArithmetic factA(largeNumStr1);
    FactorialArithmetic factB(largeNumStr2);
    FactorialArithmetic factSum = factA + factB;
    COUT("Factorial Sum:   " << to_string(factSum));

    bool expect = to_string(binSum) == to_string(factSum);
    DELETE_DEFINE_MACRO(EXPECT_TRUE(expect);)

    INSERT_EXAMPLE_BODY(
    if (expect) {
        COUT("Results match for addition!");
    } else {
       COUT("Results DO NOT match for addition!");
    })

    std::string numToMultiply = "5";
    BinaryArithmetic binProduct = binA * BinaryArithmetic(numToMultiply);
    FactorialArithmetic factProduct = factA * FactorialArithmetic(numToMultiply);

    expect = to_string(binProduct) == to_string(factProduct);
    DELETE_DEFINE_MACRO(EXPECT_TRUE(expect);)

    INSERT_EXAMPLE_BODY(
    COUT("Binary Product:  " << to_string(binProduct));
    COUT("Factorial Prod.: " << to_string(factProduct));
    if (expect) {
        COUT("Results match for multiplication!");
    } else {
        COUT("Results DO NOT match for multiplication!");
    })

} 
} // namespace numsystem

#define EXAMPLE_END_MARK