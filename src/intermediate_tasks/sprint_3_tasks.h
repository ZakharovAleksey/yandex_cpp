//
// Created by azakharov on 3/27/2021.
//

#pragma once

#include <iostream>
#include <vector>

namespace intermediate {

class Rational {
   public: // Constructors
    Rational(int value) : numerator_(value) {}

    Rational(int numerator, int denominator);

   public: // Methods
    int getNumerator() const;

    int getDenominator() const;

    Rational& operator+=(Rational other);

    Rational& operator-=(Rational other);

    Rational& operator*=(Rational other);

    Rational& operator/=(Rational other);

    friend Rational operator+(Rational rational);

    friend Rational operator+(Rational left, Rational right);

    friend Rational operator-(Rational rational);

    friend Rational operator-(Rational left, Rational right);

    friend Rational operator*(Rational left, Rational right);

    friend Rational operator/(Rational left, Rational right);

    friend bool operator==(Rational left, Rational right);

    friend bool operator!=(Rational left, Rational right);

    friend bool operator<(Rational left, Rational right);

    friend bool operator>(Rational left, Rational right);

    friend bool operator<=(Rational left, Rational right);

    friend bool operator>=(Rational left, Rational right);

    friend std::ostream& operator<<(std::ostream& os, const Rational& rational);

    friend std::istream& operator>>(std::istream& is, Rational& rational);

   private: // Methods
    static std::vector<int> splitOnDividers(int value);

    static std::pair<int, int> convertToIrreducibleView(int numerator, int denominator);

   private: // Fields
    int numerator_ = 0;
    int denominator_ = 1;
};
}  // namespace intermediate