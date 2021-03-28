//
// Created by azakharov on 3/27/2021.
//

#include "sprint_3_tasks.h"

#include <algorithm>
#include <numeric>
#include <tuple>

namespace intermediate {

using namespace std::literals;

Rational::Rational(int numerator, int denominator) {
    if (denominator_ == 0)
        throw std::invalid_argument("Fraction denominator could not be equal to zero"s);

    std::tie(numerator_, denominator_) = convertToIrreducibleView(numerator, denominator);
}

int Rational::getNumerator() const {
    return numerator_;
}

int Rational::getDenominator() const {
    return denominator_;
}

Rational operator+(Rational rational) {
    return rational;
}

Rational& Rational::operator+=(Rational other) {
    int numerator = numerator_ * other.denominator_ + other.numerator_ * denominator_;
    int denominator = denominator_ * other.denominator_;

    std::tie(numerator_, denominator_) = convertToIrreducibleView(numerator, denominator);
    return *this;
}

Rational operator+(Rational left, Rational right) {
    return left += right;
}

Rational operator-(Rational rational) {
    return {-rational.numerator_, rational.denominator_};
}

Rational& Rational::operator-=(Rational other) {
    int numerator = numerator_ * other.denominator_ - other.numerator_ * denominator_;
    int denominator = denominator_ * other.denominator_;

    std::tie(numerator_, denominator_) = convertToIrreducibleView(numerator, denominator);
    return *this;
}

Rational operator-(Rational left, Rational right) {
    return left -= right;
}

Rational& Rational::operator*=(Rational other) {
    int numerator = numerator_ * other.numerator_;
    int denominator = denominator_ * other.denominator_;

    std::tie(numerator_, denominator_) = convertToIrreducibleView(numerator, denominator);
    return *this;
}

Rational operator*(Rational left, Rational right) {
    return left *= right;
}

Rational& Rational::operator/=(Rational other) {
    if (other.numerator_ == 0)
        throw std::invalid_argument("Could not divide on fraction with zero numerator"s);

    int numerator = numerator_ * other.denominator_;
    int denominator = denominator_ * other.numerator_;

    std::tie(numerator_, denominator_) = convertToIrreducibleView(numerator, denominator);
    return *this;
}

Rational operator/(Rational left, Rational right) {
    return left /= right;
}

bool operator==(Rational left, Rational right) {
    return left.numerator_ == right.numerator_ && left.denominator_ == right.denominator_;
}

bool operator!=(Rational left, Rational right) {
    return !(left == right);
}

bool operator<(Rational left, Rational right) {
    return left.numerator_ * right.denominator_ < right.numerator_ * left.denominator_;
}

bool operator>(Rational left, Rational right) {
    return left.numerator_ * right.denominator_ > right.numerator_ * left.denominator_;
}

bool operator<=(Rational left, Rational right) {
    return left < right || left == right;
}

bool operator>=(Rational left, Rational right) {
    return left > right || left == right;
}

std::ostream& operator<<(std::ostream& os, const Rational& rational) {
    return os << rational.numerator_ << "/" << rational.denominator_ << std::endl;
}

std::istream& operator>>(std::istream& is, Rational& rational) {
    is >> rational.numerator_;
    is.get();
    is >> rational.denominator_;

    return is;
}

std::vector<int> Rational::splitOnDividers(int value) {
    std::vector<int> dividers;

    while (value != 1) {
        for (int current_divider = 2; current_divider <= value; ++current_divider) {
            if (value % current_divider == 0) {
                dividers.emplace_back(current_divider);
                value /= current_divider;
                break;
            }
        }
    }

    return dividers;
}

std::pair<int, int> Rational::convertToIrreducibleView(int numerator, int denominator) {
    auto numerator_dividers = splitOnDividers(std::abs(numerator));
    auto denominator_dividers = splitOnDividers(std::abs(denominator));

    std::vector<int> numerator_difference;
    std::vector<int> denominator_difference;

    std::set_difference(numerator_dividers.begin(), numerator_dividers.end(), denominator_dividers.begin(),
                        denominator_dividers.end(), std::inserter(numerator_difference, numerator_difference.begin()));

    std::set_difference(denominator_dividers.begin(), denominator_dividers.end(), numerator_dividers.begin(),
                        numerator_dividers.end(),
                        std::inserter(denominator_difference, denominator_difference.begin()));

    auto multiply = [](int& result, const int& divider) { return result * divider; };

    int sign = numerator * denominator > 0 ? 1 : -1;

    return {sign * std::accumulate(numerator_difference.begin(), numerator_difference.end(), 1, multiply),
            std::abs(std::accumulate(denominator_difference.begin(), denominator_difference.end(), 1, multiply))};
}

}  // namespace intermediate
