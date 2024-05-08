#ifndef POLY_H
#define POLY_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <utility>
#include <algorithm>
using power = size_t;
using coeff = int;

class polynomial
{
private:
    std::unordered_map<power, coeff> terms;

public:
    polynomial();
    polynomial(const polynomial &other);

    template <typename Iter>
    polynomial(Iter begin, Iter end)
    {
        for (auto it = begin; it != end; ++it)
        {
            terms[it->first] = it->second;
        }
    }

    void print() const;

    polynomial &operator=(const polynomial &other)
    {
        if (this != &other)
        {
            terms = other.terms;
        }
        return *this;
    }

    polynomial operator+(const polynomial &rhs) const;
    polynomial operator+(int rhs) const;
    polynomial operator-(const polynomial &rhs) const;
    polynomial operator*(const polynomial &rhs) const;
    polynomial operator*(int rhs) const;
    polynomial operator%(const polynomial &rhs) const;

    size_t find_degree_of() const;
    std::vector<std::pair<power, coeff>> canonical_form() const;
};

inline polynomial operator+(int lhs, const polynomial &rhs)
{
    return rhs + lhs;
}

inline polynomial operator*(int lhs, const polynomial &rhs)
{
    return rhs * lhs;
}

#endif
