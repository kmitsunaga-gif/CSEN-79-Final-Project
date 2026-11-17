// gen_alerts - writes a Project 2 command script to standard output.
//
//   gen_alerts --workload W1|W2|W3 [--subs 100000] [--events 10000] [--seed 42]
//
// The script first creates --subs subscriptions, then issues --events events.
// Users are named u0, u1, ... with about 5 subscriptions each.
//
// W1 realistic:   subscriptions follow a Zipf curve over topics (a few topics are very
//                 popular); publications are spread evenly over topics.
// W2 hot topic:   half of all subscriptions are to news/world.
// W3 price storm: almost every subscription is a price threshold on items/phones/item0,
//                 and most events lower that item's price step by step.
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "rng.h"

int main(int argc, char** argv) {
    std::string workload = "W1";
    long long subs = 100000, events = 10000;
    unsigned long long seed = 42;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto val = [&]() -> std::string {
            if (i + 1 >= argc) { std::cerr << "missing value for " << a << '\n'; std::exit(2); }
            return argv[++i];
        };
        if (a == "--workload") workload = val();
        else if (a == "--subs") subs = std::stoll(val());
        else if (a == "--events") events = std::stoll(val());
        else if (a == "--seed") seed = std::stoull(val());
        else { std::cerr << "unknown option " << a << '\n'; return 2; }
    }
    if (workload != "W1" && workload != "W2" && workload != "W3") {
        std::cerr << "workload must be W1, W2, or W3\n";
        return 2;
    }

    // Topic catalogue: broad parent topics, leaf topics, and items with prices.
    const char* news[] = {"world", "sports", "tech", "business", "science",
                          "health", "politics", "culture", "travel", "local"};
    const char* signs[] = {"aries", "taurus", "gemini", "cancer", "leo", "virgo",
                           "libra", "scorpio", "sagittarius", "capricorn", "aquarius", "pisces"};
    const char* kinds[] = {"phones", "laptops", "tvs", "cameras", "headphones"};
    std::vector<std::string> parents, leaves, items;
    parents.push_back("news");
    for (auto c : news) parents.push_back(std::string("news/") + c);
    for (auto c : news)
        for (int k = 0; k < 50; ++k) {
            leaves.push_back(std::string("news/") + c + "/s" + std::to_string(k));
        }
    for (auto s : signs) {
        leaves.push_back(std::string("horoscope/") + s);
    }
    for (auto k : kinds)
        for (int j = 0; j < 50; ++j) items.push_back(std::string("items/") + k + "/item" + std::to_string(j));

    kit::Rng rng(seed);
    kit::Zipf leaf_zipf(leaves.size(), 1.0), item_zipf(items.size(), 1.0);
    // About 5 subscriptions per user; in W3 each user holds one price threshold.
    long long users = workload == "W3" ? subs : (subs / 5 > 0 ? subs / 5 : 1);
    std::set<std::pair<long long, std::string>> taken;
    std::vector<std::pair<long long, std::string>> made;
    std::vector<long long> price(items.size(), 0);
    for (auto& p : price) p = rng.between(20000, 150000);
    std::string out;
    out.reserve(static_cast<size_t>(subs + events) * 40);

    auto user_name = [](long long u) { return "u" + std::to_string(u); };

    long long first = 0;
    if (workload == "W2")  // half of all users follow news/world
        for (; first < users / 2 && first < subs; ++first) {
            taken.insert({first, "news/world"});
            made.push_back({first, "news/world"});
            out += "SUB " + user_name(first) + " news/world\n";
        }
    long long attempts = 0;
    for (long long i = first; i < subs; ++i) {
        if (++attempts > 50 * subs + 1000) {
            std::cerr << "could not place every subscription; stopped at " << i << '\n';
            break;
        }
        long long u = rng.between(0, users - 1);
        std::string topic, extra;
        double r = rng.uniform();
        if (workload == "W3" && r < 0.95) {
            topic = items[0];
            extra = " below " + kit::cents(rng.between(10000, 99999));
        } else if (r < 0.15) {
            size_t k = item_zipf.sample(rng);
            topic = items[k];
            extra = " below " + kit::cents(price[k] * rng.between(50, 95) / 100);
        } else {
            // Mostly specific topics (Zipf over leaves); a few broad ones like news or news/tech.
            topic = rng.chance(0.03) ? parents[rng.between(0, (long long)parents.size() - 1)]
                                     : leaves[leaf_zipf.sample(rng)];
            double m = rng.uniform();
            if (topic.rfind("horoscope/", 0) == 0 && m < 0.7)
                extra = " at " + std::string(rng.chance(0.5) ? "08:00" : "07:30");
            else if (m < 0.05)
                extra = " every " + std::to_string(rng.between(1, 12));
        }
        if (!taken.insert({u, topic}).second) {  // already subscribed: try another user
            --i;
            continue;
        }
        made.push_back({u, topic});
        out += "SUB " + user_name(u) + " " + topic + extra + "\n";
    }

    long long w3_price = 100000;
    for (long long e = 0; e < events; ++e) {
        double r = rng.uniform();
        if (workload == "W3" && r < 0.8) {
            w3_price -= rng.between(1, 20);
            if (w3_price < 100) w3_price = 100;
            out += "PRICE " + items[0] + " " + kit::cents(w3_price) + "\n";
        } else if (r < 0.70) {
            std::string topic = workload == "W2" && rng.chance(0.1) ? "news/world/s0" : leaves[rng.between(0, (long long)leaves.size() - 1)];
            out += "PUB " + topic + " m" + std::to_string(e) + "\n";
        } else if (r < 0.85) {
            size_t k = item_zipf.sample(rng);
            price[k] = price[k] * rng.between(85, 105) / 100;
            if (price[k] < 100) price[k] = 100;
            out += "PRICE " + items[k] + " " + kit::cents(price[k]) + "\n";
        } else if (r < 0.95) {
            out += "TICK 30\n";
        } else if (r < 0.98 && !made.empty()) {
            size_t k = static_cast<size_t>(rng.between(0, (long long)made.size() - 1));
            out += "UNSUB " + user_name(made[k].first) + " " + made[k].second + "\n";
            made[k] = made.back();
            made.pop_back();
        } else {
            long long u = rng.between(0, users - 1);
            out += (rng.chance(0.5) ? "INBOX " : "HISTORY ") + user_name(u) + "\n";
        }
    }
    std::fwrite(out.data(), 1, out.size(), stdout);
}
