#include "poly.h"
#include <thread>
#include <mutex>

polynomial::polynomial() { terms[0] = 0; }
polynomial::polynomial(const polynomial &other) : terms(other.terms) {}
std::mutex mtx;

void polynomial::print() const
{
    auto can_form = this->canonical_form();
    bool first = true;
    for (const auto &term : can_form)
    {
        if (!first)
            std::cout << " + ";
        std::cout << term.second << "x^" << term.first;
        first = false;
    }
    std::cout << std::endl;
}

polynomial polynomial::operator+(const polynomial &rhs) const
{
    polynomial result = *this;
    for (const auto &term : rhs.terms)
    {
        result.terms[term.first] += term.second;
        if (result.terms[term.first] == 0)
        {
            result.terms.erase(term.first);
        }
    }
    return result;
}

polynomial polynomial::operator+(int rhs) const
{
    polynomial result = *this;
    result.terms[0] += rhs;
    if (result.terms[0] == 0 && result.terms.size() > 1)
    {
        result.terms.erase(0);
    }
    return result;
}

polynomial polynomial::operator-(const polynomial &rhs) const
{
    polynomial result = *this;
    for (const auto &term : rhs.terms)
    {
        result.terms[term.first] -= term.second;
        if (result.terms[term.first] == 0)
        {
            result.terms.erase(term.first);
        }
    }
    return result;
}

polynomial polynomial::operator*(const polynomial &rhs) const
{
    polynomial result;
    size_t numTerms = this->terms.size();
    size_t numCores = std::thread::hardware_concurrency();
    size_t threshold = 8;
    if (numTerms < threshold)
    {
        for (const auto &left_term : this->terms)
        {
            for (const auto &right_term : rhs.terms)
            {
                int new_power = left_term.first + right_term.first;
                double new_coeff = left_term.second * right_term.second;
                result.terms[new_power] += new_coeff;
                if (result.terms[new_power] == 0)
                {
                    result.terms.erase(new_power);
                }
            }
        }
    }
    else
    {
        std::vector<std::thread> threads;
        auto it = this->terms.begin();
        size_t distance = numTerms;
        size_t per_thread = distance / numCores;
        for (size_t i = 0; i < numCores; ++i)
        {
            auto last = (i == numCores - 1) ? this->terms.end() : std::next(it, per_thread);
            threads.emplace_back([&result, &rhs, it, last]()
                                 {
                                     polynomial local_result;
                                     for (auto iter = it; iter != last; ++iter)
                                     {
                                         for (const auto &r_term : rhs.terms)
                                         {
                                             int new_power = iter->first + r_term.first;
                                             double new_coeff = iter->second * r_term.second;
                                             local_result.terms[new_power] += new_coeff;
                                         }
                                     }
                                     std::lock_guard<std::mutex> guard(mtx);
                                     for (const auto &term : local_result.terms)
                                     {
                                         result.terms[term.first] += term.second;
                                         if (result.terms[term.first] == 0)
                                         {
                                             result.terms.erase(term.first);
                                         }
                                     } });
            it = last;
        }

        for (auto &thread : threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }

    return result;
}

polynomial polynomial::operator*(int rhs) const
{
    polynomial result = *this;
    if (rhs == 0)
    {
        result.terms.clear();
        result.terms[0] = 0;
    }
    else
    {
        for (auto iter = result.terms.begin(); iter != result.terms.end();)
        {
            iter->second *= rhs;
            if (iter->second == 0)
            {
                iter = result.terms.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
    return result;
}

polynomial polynomial::operator%(const polynomial &rhs) const
{
    polynomial dividend = *this;
    polynomial divisor = rhs;
    auto find_highest_degree = [](const std::unordered_map<power, coeff> &terms) -> std::pair<power, coeff>
    {
        power max_power = 0;
        coeff max_coeff = 0;
        for (const auto &term : terms)
        {
            if (term.first > max_power || (term.first == max_power && term.second != 0))
            {
                max_power = term.first;
                max_coeff = term.second;
            }
        }
        return {max_power, max_coeff};
    };

    while (!dividend.terms.empty())
    {
        auto [dividend_lead_power, dividend_lead_coeff] = find_highest_degree(dividend.terms);
        auto [divisor_lead_power, divisor_lead_coeff] = find_highest_degree(divisor.terms);
        if (dividend_lead_power < divisor_lead_power)
        {
            break;
        }

        power diff_power = dividend_lead_power - divisor_lead_power;
        coeff diff_coeff = dividend_lead_coeff / divisor_lead_coeff;
        polynomial term_to_subtract;
        term_to_subtract.terms[diff_power] = diff_coeff;
        polynomial subtractor = term_to_subtract * divisor;
        dividend = dividend - subtractor;
    }

    return dividend;
}

size_t polynomial::find_degree_of() const
{
    if (terms.empty())
        return 0;
    long unsigned int max_degree = 0;
    for (const auto &term : terms)
    {
        if (term.second != 0 && term.first > max_degree)
        {
            max_degree = term.first;
        }
    }
    return max_degree;
}

std::vector<std::pair<unsigned long, int>> polynomial::canonical_form() const
{
    std::vector<std::pair<unsigned long, int>> form;
    for (const auto &term : terms)
    {
        if (term.second != 0)
        {
            form.emplace_back(term.first, term.second);
        }
    }
    if (form.empty())
    {
        form.emplace_back(0, 0);
    }
    std::sort(form.begin(), form.end(), [](const auto &a, const auto &b)
              { return a.first > b.first; });
    return form;
}
