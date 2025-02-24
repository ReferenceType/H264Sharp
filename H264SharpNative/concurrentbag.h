#include <atomic>
#include <memory>
#include <optional>
#include <cstdint>

template<typename T>
class ConcurrentBag {
private:
    // Using a tagged pointer to prevent ABA problems
    struct TaggedPtr {
        std::uintptr_t ptr;  // Pointer in the lower bits
        std::uint32_t tag;   // Version counter in the upper bits

        TaggedPtr() noexcept : ptr(0), tag(0) {}
        TaggedPtr(std::uintptr_t p, std::uint32_t t) noexcept : ptr(p), tag(t) {}
    };

    struct Node {
        T data;
        TaggedPtr next;

        Node(const T& item) : data(item), next() {}
    };

    // Atomic tagged pointer
    struct AtomicTaggedPtr {
        std::atomic<std::uint64_t> value;

        AtomicTaggedPtr() : value(0) {}

        TaggedPtr load(std::memory_order order = std::memory_order_seq_cst) {
            std::uint64_t val = value.load(order);
            return TaggedPtr(val & ((1ull << 48) - 1), val >> 48);
        }

        void store(TaggedPtr ptr, std::memory_order order = std::memory_order_seq_cst) {
            std::uint64_t val = (static_cast<std::uint64_t>(ptr.tag) << 48) | ptr.ptr;
            value.store(val, order);
        }

        bool compare_exchange_weak(TaggedPtr& expected, TaggedPtr desired,
            std::memory_order success,
            std::memory_order failure) {
            std::uint64_t exp_val = (static_cast<std::uint64_t>(expected.tag) << 48) | expected.ptr;
            std::uint64_t des_val = (static_cast<std::uint64_t>(desired.tag) << 48) | desired.ptr;

            bool success_flag = value.compare_exchange_weak(exp_val, des_val, success, failure);
            if (!success_flag) {
                expected = TaggedPtr(exp_val & ((1ull << 48) - 1), exp_val >> 48);
            }
            return success_flag;
        }
    };

    AtomicTaggedPtr head;

public:
    ConcurrentBag() {}

    ~ConcurrentBag() {
        // Cleanup remaining nodes
        auto current = head.load(std::memory_order_acquire);
        while (current.ptr != 0) {
            Node* node = reinterpret_cast<Node*>(current.ptr);
            current = node->next;
            delete node;
        }
    }

    // Prevent copying and assignment
    ConcurrentBag(const ConcurrentBag&) = delete;
    ConcurrentBag& operator=(const ConcurrentBag&) = delete;

    void add(const T& item) {
        Node* new_node = new Node(item);
        TaggedPtr old_head;
        TaggedPtr new_head;

        do {
            old_head = head.load(std::memory_order_relaxed);
            new_head = TaggedPtr(reinterpret_cast<std::uintptr_t>(new_node), old_head.tag + 1);
            new_node->next = old_head;
        } while (!head.compare_exchange_weak(old_head, new_head,
            std::memory_order_release,
            std::memory_order_relaxed));
    }

    void add(T&& item) {
        Node* new_node = new Node(item);
        TaggedPtr old_head;
        TaggedPtr new_head;

        do {
            old_head = head.load(std::memory_order_relaxed);
            new_head = TaggedPtr(reinterpret_cast<std::uintptr_t>(new_node), old_head.tag + 1);
            new_node->next = old_head;
        } while (!head.compare_exchange_weak(old_head, new_head,
            std::memory_order_release,
            std::memory_order_relaxed));
    }

    bool tryTake(T& out) {
        TaggedPtr old_head;
        TaggedPtr new_head;

        do {
            old_head = head.load(std::memory_order_relaxed);
            if (old_head.ptr == 0) {
                return false;  // Bag is empty
            }
            Node* node = reinterpret_cast<Node*>(old_head.ptr);
            new_head = TaggedPtr(
                reinterpret_cast<std::uintptr_t>(reinterpret_cast<Node*>(node->next.ptr)),
                old_head.tag + 1
            );
        } while (!head.compare_exchange_weak(old_head, new_head,
            std::memory_order_release,
            std::memory_order_relaxed));

        Node* node = reinterpret_cast<Node*>(old_head.ptr);
        out = node->data;
        delete node;
        return true;
    }

    bool isEmpty() const {
        return head.load(std::memory_order_relaxed).ptr == 0;
    }
};