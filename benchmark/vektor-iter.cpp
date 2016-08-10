
#include <nonius/nonius_single.h++>

#include <immu/vektor.hpp>
#include <immu/dvektor.hpp>
#include <immu/ivektor.hpp>

#include <vector>
#include <list>
#include <numeric>

NONIUS_PARAM("size", std::size_t{1000})

NONIUS_BENCHMARK("std::vector", [] (nonius::parameters params)
{
    auto benchmark_size = params.get<std::size_t>("size");

    auto v = std::vector<unsigned>(benchmark_size);
    std::iota(v.begin(), v.end(), 0u);

    return [=] {
        auto volatile x = std::accumulate(v.begin(), v.end(), 0u);
        return x;
    };
})

NONIUS_BENCHMARK("std::list", [] (nonius::parameters params)
{
    auto benchmark_size = params.get<std::size_t>("size");

    auto v = std::list<unsigned>{};
    for (auto i = 0u; i < benchmark_size; ++i)
        v.push_back(i);

    return [=] {
        auto volatile x = std::accumulate(v.begin(), v.end(), 0u);
        return x;
    };
})

template <typename Vektor,
          std::size_t Limit=std::numeric_limits<std::size_t>::max()>
auto generic()
{
    return [] (nonius::parameters params)
    {
        auto benchmark_size = params.get<std::size_t>("size");
        if (benchmark_size > Limit) benchmark_size = 1;

        auto v = Vektor{};
        for (auto i = 0u; i < benchmark_size; ++i)
            v = v.push_back(i);

        return [=] {
            auto volatile x = std::accumulate(v.begin(), v.end(), 0u);
            return x;
        };
    };
}

NONIUS_BENCHMARK("immu::vektor/4B",   generic<immu::vektor<unsigned,4>>())
NONIUS_BENCHMARK("immu::vektor/5B",   generic<immu::vektor<unsigned,5>>())
NONIUS_BENCHMARK("immu::vektor/6B",   generic<immu::vektor<unsigned,6>>())
NONIUS_BENCHMARK("immu::dvektor/4B",  generic<immu::dvektor<unsigned,4>>())
NONIUS_BENCHMARK("immu::dvektor/5B",  generic<immu::dvektor<unsigned,5>>())
NONIUS_BENCHMARK("immu::dvektor/6B",  generic<immu::dvektor<unsigned,6>>())
NONIUS_BENCHMARK("immu::ivektor",     generic<immu::ivektor<unsigned>, 10000>())
