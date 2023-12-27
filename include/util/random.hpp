#pragma once
#include <common.hpp>
#include <random>
#include <type_traits>
class Random
{
public:
    Random() : rng(std::random_device{}()) {}
    
    static bool Init() {
        return CoinFlip(); //just so singleton is created during init 
    }
    template <typename T = int>
    static inline T Range(T min, T max) {
         if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> dist(min, max);
            return dist(inst().rng);
        } else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(inst().rng);
        }
    }
 //random helper functions that can do like 1/3 odds, etc for easy use in like enemy AI and weapon damage calculations
    static inline bool CoinFlip(){
        return Range<int>(0,100) >= 50;
    }
    static inline bool Chance(float probability) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(inst().rng) < probability;
    }

     template <typename T>
    static inline T AddVariation(T baseValue, T variationRange) {
        // Generate a random value in the range [-variationRange, variationRange]
        T variation = Range(-variationRange, variationRange);
        return baseValue + variation;
    }
     template <typename T>
    static inline T AddVariation(T baseValue, T variationRange, T minValue) {
        // Generate a random value in the range [-variationRange, variationRange]
        T variation = Range(-variationRange, variationRange);

        return std::max(baseValue + variation, minValue);
    }
      template <typename T>
    static inline T AddVariation(T baseValue, T variationRange, T minValue, T maxValue) {
        // Generate a random value in the range [-variationRange, variationRange]
        T variation = Range(-variationRange, variationRange);

        return std::clamp(baseValue + variation, minValue, maxValue);
    }


   


    template <typename T>
    static T WeightedChoice(const std::vector<std::pair<T, float>>& choices) {
        std::vector<float> weights;
        for (const auto& choice : choices) {
            weights.push_back(choice.second);
        }

        std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
        return choices[dist(inst().rng)].first;
        //ex. auto result = Random::WeightedChoice<std::string>({{"Option1", 0.5f}, {"Option2", 0.5f}});
    }
    template <typename Container>
    static auto RandomElement(const Container& container) -> decltype(*container.begin()) {
        std::uniform_int_distribution<size_t> dist(0, container.size() - 1);
        auto it = container.begin();
        std::advance(it, dist(inst().rng));
        return *it;
        //ex. auto item = Random::RandomElement(itemsVector);
    }


private:
    static inline Random& inst(){
        static Random r;
        return r;
    }

   // std::mt19937 rng;
    std::knuth_b rng;
};