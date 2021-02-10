#include <UnitTest++/UnitTest++.h>
#include "main.cpp"

TEST(MoneyConstructorCurrency) {
    //Setup
    const std::string currency = "CAD";
    const std::string currency2 = "USD";

    const float amount = 42.00;
    const float amount2 = 45.00;
    //Processing
    Money money(amount, currency);
    Money money2(amount2, currency2);
    //Verification
    CHECK_EQUAL(currency, money.get_currency());  // pass
    // initialized with same currency
    //    CHECK_EQUAL(money.get_amount(), money2.get_amount());  // fail
    // initialized with currency

    // if one the the test in one TEST fails, then the total test failed
}

struct ConstructorFixture {
    //performs the data initialization for each TEST_FIXTURE
    ConstructorFixture()
    : currencyCAD("CAD"), currencyUSD("USD"), value10(42), value20(20) {
    }
    // constructor can choose from one of these values

    const std::string currencyCAD;
    const std::string currencyUSD;
    const float value10;
    const float value20;
};


TEST_FIXTURE(ConstructorFixture, MoneyConstructorAmount) {
    // class name is passed as the first argument, test name(whatever) is passed as the second argument
    // Within the TEST_FIXTUREs we can directly access the fixture class’s member variables.
    Money money1(value10, currencyCAD);
    Money money2(value10, currencyCAD);
    CHECK_EQUAL(money1.get_amount(), money2.get_amount());  // pass
}
TEST_FIXTURE(ConstructorFixture, MoneyConstructorCurrency) {
    Money money1(value10, currencyCAD);
    Money money2(value20, currencyUSD);
    //    CHECK(money1 == money2); // fail // need to overload operator ==
}

SUITE(MoneyConstructorAccessor) {
    // add a suite to group related tests together
    TEST(test_run_time) {
        UNITTEST_TIME_CONSTRAINT(10);  // The test must finish in 10 ms
        UnitTest::TimeHelpers::SleepMs(100);  // Add 100 ms in the test
        // The test will fail
    }
}

