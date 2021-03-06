//
// immer - immutable data structures for C++
// Copyright (C) 2016 Juan Pedro Bolivar Puente
//
// This file is part of immer.
//
// immer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// immer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with immer.  If not, see <http://www.gnu.org/licenses/>.
//

#include "../util.hpp"
#include "../dada.hpp"

#include <immer/algorithm.hpp>

#include <catch.hpp>
#include <boost/range/adaptors.hpp>

#include <algorithm>
#include <numeric>
#include <vector>

#ifndef VECTOR_T
#error "define the vector template to use in VECTOR_T"
#endif

template <typename V=VECTOR_T<unsigned>>
auto make_test_vector(unsigned min, unsigned max)
{
    auto v = V{};
    for (auto i = min; i < max; ++i)
        v = v.push_back({i});
    return v;
}

TEST_CASE("instantiation")
{
    auto v = VECTOR_T<int>{};
    CHECK(v.size() == 0u);
}

TEST_CASE("push back one element")
{
    SECTION("one element")
    {
        const auto v1 = VECTOR_T<int>{};
        auto v2 = v1.push_back(42);
        CHECK(v1.size() == 0u);
        CHECK(v2.size() == 1u);
        CHECK(v2[0] == 42);
    }

    SECTION("many elements")
    {
        const auto n = 666u;
        auto v = VECTOR_T<unsigned>{};
        for (auto i = 0u; i < n; ++i) {
            v = v.push_back(i * 42);
            CHECK(v.size() == i + 1);
            for (auto j = 0u; j < v.size(); ++j)
                CHECK(v[j] == j * 42);
        }
    }
}

TEST_CASE("update")
{
    const auto n = 42u;
    auto v = make_test_vector(0, n);

    SECTION("set")
    {
        const auto u = v.set(3u, 13u);
        CHECK(u.size() == v.size());
        CHECK(u[2u] == 2u);
        CHECK(u[3u] == 13u);
        CHECK(u[4u] == 4u);
        CHECK(u[40u] == 40u);
        CHECK(v[3u] == 3u);
    }

    SECTION("set further")
    {
        auto v = make_test_vector(0, 666);

        auto u = v.set(3u, 13u);
        u = u.set(200u, 7u);
        CHECK(u.size() == v.size());

        CHECK(u[2u] == 2u);
        CHECK(u[4u] == 4u);
        CHECK(u[40u] == 40u);
        CHECK(u[600u] == 600u);

        CHECK(u[3u] == 13u);
        CHECK(u[200u] == 7u);

        CHECK(v[3u] == 3u);
        CHECK(v[200u] == 200u);
    }

    SECTION("set further more")
    {
        auto v = make_test_vector(0, 666u);

        for (auto i = 0u; i < v.size(); ++i) {
            v = v.set(i, i+1);
            CHECK(v[i] == i+1);
        }
    }

    SECTION("update")
    {
        const auto u = v.update(10u, [] (auto x) { return x + 10; });
        CHECK(u.size() == v.size());
        CHECK(u[10u] == 20u);
        CHECK(v[40u] == 40u);

        const auto w = v.update(40u, [] (auto x) { return x - 10; });
        CHECK(w.size() == v.size());
        CHECK(w[40u] == 30u);
        CHECK(v[40u] == 40u);
    }
}

TEST_CASE("iterator")
{
    const auto n = 666u;
    auto v = make_test_vector(0, n);

    SECTION("works with range loop")
    {
        auto i = 0u;
        for (const auto& x : v)
            CHECK(x == i++);
        CHECK(i == v.size());
    }

    SECTION("works with standard algorithms")
    {
        auto s = std::vector<unsigned>(n);
        std::iota(s.begin(), s.end(), 0u);
        std::equal(v.begin(), v.end(), s.begin(), s.end());
    }

    SECTION("can go back from end")
    {
        auto expected  = n - 1;
        CHECK(expected == *--v.end());
    }

    SECTION("works with reversed range adaptor")
    {
        auto r = v | boost::adaptors::reversed;
        auto i = n;
        for (const auto& x : r)
            CHECK(x == --i);
    }

    SECTION("works with strided range adaptor")
    {
        auto r = v | boost::adaptors::strided(5);
        auto i = 0u;
        for (const auto& x : r)
            CHECK(x == 5 * i++);
    }

    SECTION("works reversed")
    {
        auto i = n;
        for (auto iter = v.rbegin(), last = v.rend(); iter != last; ++iter)
            CHECK(*iter == --i);
    }

    SECTION("advance and distance")
    {
        auto i1 = v.begin();
        auto i2 = i1 + 100;
        CHECK(100u == *i2);
        CHECK(100  == i2 - i1);
        CHECK(50u  == *(i2 - 50));
        CHECK(-30  == (i2 - 30) - i2);
    }
}

TEST_CASE("accumulate")
{
    const auto n = 666u;
    auto v = make_test_vector(0, n);

    SECTION("sum collection")
    {
        auto sum = immer::accumulate(v, 0u);
        auto expected = v.size() * (v.size() - 1) / 2;
        CHECK(sum == expected);
    }
}

TEST_CASE("vector of strings")
{
    const auto n = 666u;

    auto v = VECTOR_T<std::string>{};
    for (auto i = 0u; i < n; ++i)
        v = v.push_back(std::to_string(i));

    for (auto i = 0u; i < v.size(); ++i)
        CHECK(v[i] == std::to_string(i));

    SECTION("set")
    {
        for (auto i = 0u; i < n; ++i)
            v = v.set(i, "foo " + std::to_string(i));
        for (auto i = 0u; i < n; ++i)
            CHECK(v[i] == "foo " + std::to_string(i));
    }
}

struct non_default
{
    unsigned value;
    non_default() = delete;
    operator unsigned() const { return value; }

#if IMMER_DEBUG_PRINT
    friend std::ostream& operator<< (std::ostream& os, non_default x)
    {
        os << "ND{" << x.value << "}";
        return os;
    }
#endif
};

TEST_CASE("non default")
{
    const auto n = 666u;

    auto v = VECTOR_T<non_default>{};
    for (auto i = 0u; i < n; ++i)
        v = v.push_back({ i });

    CHECK_VECTOR_EQUALS(v, boost::irange(0u, n));

    SECTION("set")
    {
        for (auto i = 0u; i < n; ++i)
            v = v.set(i, {i + 1});

        CHECK_VECTOR_EQUALS(v, boost::irange(1u, n + 1u));
    }
}

TEST_CASE("take")
{
    const auto n = 666u;

    SECTION("anywhere")
    {
        auto v = make_test_vector(0, n);

        for (auto i : test_irange(0u, n)) {
            auto vv = v.take(i);
            CHECK(vv.size() == i);
            CHECK_VECTOR_EQUALS_RANGE(vv, v.begin(), v.begin() + i);
        }
    }
}

TEST_CASE("exception safety")
{
    constexpr auto n = 666u;

    using dadaist_vector_t = typename dadaist_vector<VECTOR_T<unsigned>>::type;

    SECTION("push back")
    {
        auto v = dadaist_vector_t{};
        auto d = dadaism{};
        for (auto i = 0u; v.size() < n;) {
            auto s = d.next();
            try {
                v = v.push_back({i});
                ++i;
            } catch (dada_error) {}
            CHECK_VECTOR_EQUALS(v, boost::irange(0u, i));
        }
        CHECK(d.happenings > 0);
        IMMER_TRACE_E(d.happenings);
    }

    SECTION("update")
    {
        auto v = make_test_vector<dadaist_vector_t>(0, n);
        auto d = dadaism{};
        for (auto i = 0u; i < n;) {
            auto s = d.next();
            try {
                v = v.update(i, [] (auto x) { return dada(), x + 1; });
                ++i;
            } catch (dada_error) {}
            CHECK_VECTOR_EQUALS(v, boost::join(
                                    boost::irange(1u, 1u + i),
                                    boost::irange(i, n)));
        }
        CHECK(d.happenings > 0);
        IMMER_TRACE_E(d.happenings);
    }

    SECTION("take")
    {
        auto v = make_test_vector<dadaist_vector_t>(0, n);
        auto d = dadaism{};
        for (auto i = 0u; i < n;) {
            auto s = d.next();
            auto r = dadaist_vector_t{};
            try {
                r = v.take(i);
                CHECK_VECTOR_EQUALS(r, boost::irange(0u, i++));
            } catch (dada_error) {
                CHECK_VECTOR_EQUALS(r, boost::irange(0u, 0u));
            }
        }
        CHECK(d.happenings > 0);
        IMMER_TRACE_E(d.happenings);
    }
}
