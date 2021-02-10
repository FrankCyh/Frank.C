#include <iostream>
#include <UnitTest++/UnitTest++.h>

using namespace std;

int main(int argc, char** argv) {
    cout << "This is " << argv[0] << endl;

    //Runs all defined unit tests
    int num_failures = UnitTest::RunAllTests();

    return num_failures;
}
