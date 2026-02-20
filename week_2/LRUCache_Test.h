//
// Created by 15728 on 2026/2/20.
//

#ifndef CLIONPROJECTS_LRUCACHE_H
#define CLIONPROJECTS_LRUCACHE_H


#include <list>
#include <unordered_map>
#include <stdexcept>

/**
 * 单线程 LRU 缓存模板类
 * @tparam Key   键类型
 * @tparam Value 值类型
 */
template<typename Key, typename Value>
class LRUCache_Test {
public:
    /**
     * 构造函数
     * @param capacity 最大容量，必须大于 0
     */
    explicit LRUCache_Test(size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("Capacity must be positive");
        }
    }

    /**
     * 获取键对应的值 查数据 + 移到头部
     * @param key 待查询键
     * @return 键对应的值
     * @throw std::out_of_range 如果键不存在于缓存中
     */
    Value get(const Key& key) {
        auto it = node_map_.find(key);
        if (it == node_map_.end()) {
            throw std::out_of_range("Key not found in cache");
        }
        // 将访问的节点提升为最近使用（移至链表头部）
        move_to_front(it->second);
        return it->second->value;
    }

    /**
     * 插入或更新键值对 插/更新数据 + 淘汰
     * @param key   键
     * @param value 值
     */
    void put(const Key& key, const Value& value) {
        auto it = node_map_.find(key);
        if (it != node_map_.end()) {
            // 键已存在：更新值并移至头部
            it->second->value = value;
            move_to_front(it->second);
        } else {
            // 键不存在：需要插入新节点
            if (lru_list_.size() >= capacity_) {
                evict_lru();   // 淘汰最久未使用的节点
            }
            // 在链表头部插入新节点
            lru_list_.push_front(CacheNode{key, value});
            node_map_[key] = lru_list_.begin();
        }
    }

    /**
     * 检查键是否存在于缓存中
     */
    bool contains(const Key& key) const {
        return node_map_.find(key) != node_map_.end();
    }

    /**
     * 当前缓存中的元素个数
     */
    [[nodiscard]] size_t size() const {
        return lru_list_.size();
    }

    /**
     * 返回缓存容量
     */
    [[nodiscard]] size_t capacity() const {
        return capacity_;
    }

    void print() const {
        std::cout << "Cache [最近使用 -> 最久未使用]: ";
        // 遍历双向链表（从头部到尾部）
        for (const auto& node : lru_list_) {
            std::cout << "[" << node.key << ": " << node.value << "] ";
        }
        std::cout << std::endl;
    }

private:
    // 缓存节点结构（存储键值对）
    struct CacheNode {
        Key key;
        Value value;
    };

    // 链表迭代器类型别名（简化书写）
    using ListIterator = typename std::list<CacheNode>::iterator;

    /**
     * 将指定迭代器指向的节点移动到链表头部（最近使用）
     */
    void move_to_front(ListIterator it) {
        // splice 操作：将 it 指向的节点剪切到链表头部，常数时间完成
        lru_list_.splice(lru_list_.begin(), lru_list_, it);
    }

    /**
     * 淘汰最久未使用的节点（链表尾部节点）
     */
    void evict_lru() {
        if (lru_list_.empty()) return;
        // 获取尾部节点的迭代器
        auto last = --lru_list_.end();
        // 从哈希表中删除对应的键
        node_map_.erase(last->key);
        // 从链表中删除节点
        lru_list_.pop_back();
    }

private:
    size_t capacity_;                       // 缓存最大容量
    std::list<CacheNode> lru_list_;         // 双向链表，头部最近使用，尾部最久未使用
    std::unordered_map<Key, ListIterator> node_map_; // 哈希表：键 → 链表节点迭代器
};


#endif //CLIONPROJECTS_LRUCACHE_H