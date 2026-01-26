#include <unordered_map>
#include <tuple>
#include <string>
#include "buffer.h"

#define BW_PER_LINK 200

struct tuple_hash_routers {
    template <class T1, class T2, class T3, class T4>
    std::size_t operator()(const std::tuple<T1, T2, T3, T4>& tpl) const noexcept{
        const auto& item1 = std::get<0>(tpl);
        const auto& item2 = std::get<1>(tpl);
        const auto& item3 = std::get<2>(tpl);
        const auto& item4 = std::get<3>(tpl);
        size_t h1 = std::hash<T1>{}(item1);
        size_t h2 = std::hash<T2>{}(item2);
        size_t h3 = std::hash<T3>{}(item3);
        size_t h4 = std::hash<T4>{}(item4);
        
        // Combinar los hashes
        size_t seed = 0;
        seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

class router
{
private:
    unsigned number_of_networks;
    double frequency;
    bool first_buffer = true;
    unsigned links_per_gpu;
    std::unordered_map<std::tuple<bool, bool, unsigned, unsigned>, buffer*, tuple_hash_routers> buffers;

public:
    router(unsigned number_of_networks, double freq);
    ~router();
    void add_buffer(bool is_request, bool is_push, unsigned input, unsigned output, buffer* buffer);
    void push_request(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle);
    void push_reply(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle);
    bool has_buffer_reply(unsigned input, unsigned output, unsigned int size);
    bool has_buffer_request(unsigned input, unsigned output, unsigned int size);
    mem_fetch* top_reply(unsigned module_number, unsigned long cycle);
    void pop_reply(unsigned module_number, unsigned long cycle);
    mem_fetch* top_request(unsigned module_number,unsigned long cycle);
    void pop_request(unsigned module_number, unsigned long cycle);
    void cycle(unsigned long cycle);
};

