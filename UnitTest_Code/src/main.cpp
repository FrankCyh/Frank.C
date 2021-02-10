#include <iostream>

using namespace std;

//int main(int argc, char** argv) {
//    cout << "This is " << argv[0] << endl;
//    return 0;
//}

class Money {
    public:
    Money(const float amount, const std::string currency)
    : amount_(amount), currency_(currency) {
    }
    float get_amount() const {
        return amount_;
    }
    std::string get_currency() const {
        /* Version 1
        return "invalid";  //This is wrong
        */
        // Version 2
        return currency_;
    }
    friend bool operator==(const Money& lhs, const Money& rhs) {
        return lhs.get_amount() == rhs.get_amount() && lhs.get_currency() == rhs.get_currency();
    }

    private:
    float amount_;
    std::string currency_;
    
};
