//
// Created by 15728 on 2026/2/20.
//

// ThreadSafeLRUCache.h

#include <atomic>
#include <mutex>
#include <shared_mutex> // 进阶版会用到

#include "LRUCache_Test.h"

template<typename Key, typename Value>
class ThreadSafeLRUCache {
public:
    explicit ThreadSafeLRUCache(size_t capacity)
        : internal_cache_(capacity) {}

    // 线程安全的 get
    Value get(const Key& key) {
        // [A] 注意：get 操作虽然是“读数据”，但因为要更新 LRU 链表顺序（修改内部状态）
        // 所以这里我们依然使用 unique_lock (排他锁)，防止多个线程同时修改链表导致崩溃。
        // 如果你想极致性能，可以把“读取值”和“更新顺序”分开，但这会增加逻辑复杂度。
        std::unique_lock<std::shared_mutex> lock(mutex_);

        try {
            Value val = internal_cache_.get(key);
            ++hit_count_; // [B] 统计：命中
            return val;
        } catch (const std::out_of_range&) {
            ++miss_count_; // [B] 统计：未命中
            throw; // 重新抛出异常，让上层处理
        }
    }

    // 线程安全的 put
    void put(const Key& key, const Value& value) {
        // [A] 写操作，必须使用 unique_lock (排他锁)
        std::unique_lock<std::shared_mutex> lock(mutex_);
        internal_cache_.put(key, value);
    }

    // [B] 新增：获取缓存命中率
    double get_hit_rate() const {
        auto hits = hit_count_.load();
        auto misses = miss_count_.load();
        auto total = hits + misses;
        return total == 0 ? 0.0 : static_cast<double>(hits) / total;
    }

    // [B] 新增：重置统计数据
    void reset_stats() const{
        hit_count_.store(0);
        miss_count_.store(0);
    }

    // [B] 新增：获取命中次数
    size_t get_hit_count() const { return hit_count_.load(); }

    // [B] 新增：获取未命中次数
    size_t get_miss_count() const { return miss_count_.load(); }

    // 代理其他需要的接口...
    bool contains(const Key& key) const {

        std::shared_lock<std::shared_mutex> lock(mutex_);
        return internal_cache_.contains(key);
    }

    size_t size() const {

        std::shared_lock<std::shared_mutex> lock(mutex_);
        return internal_cache_.size();
    }

private:
    LRUCache_Test<Key, Value> internal_cache_;

    // [A] 升级：从 std::mutex 变为 std::shared_mutex
    mutable std::shared_mutex mutex_;

    // [B] 新增：统计计数器 (使用 atomic 避免统计时也要加锁)
    mutable std::atomic<size_t> hit_count_{};
    mutable std::atomic<size_t> miss_count_{};
};