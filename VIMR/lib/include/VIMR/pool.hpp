#pragma once

#include <functional>
#include <vector>

namespace VIMR {
  /*
   * This is a memory pool that can grow on demand and which preserves the addresses
   * of the elements in the pool when it does grow. The second point is critical for
   * the Octree, where tree nodes have pointers to other nodes. This is why there is
   * memory management nonsense in here.
   *
   * get_next will automatically grow the underlying storage by _grow_size, underlying
   * storage is a STL vector so you're likely to get one-off performance penalties
   * associated with memory [re]allocation.
   */
  template<class T>
  class Pool {
   public:
    Pool(size_t _init_size, size_t _grow_size) {
      pool.reserve(_init_size);
      grow_pool(_init_size);
      grow_size = _grow_size;
    }
    ~Pool() {
      while (!pool.empty()) {
        delete pool.back();
        pool.pop_back();
      }
    }
    /*
     * get_next returns the next unused element from the pool. If the pool is full then
     * it is first grown by _grow_size and then the next unused element is returned.
     */
    T* get_next() {
      if (head >= pool.size()) grow_pool(grow_size);
      return pool[head++];
    }
    /*
     * Does not clear the underlying memory, just resets the 'head' pointer to zero
     */
    void reset() {
      head = 0;
      idx_get = 0;
    }
    int size() {
      return static_cast<int>(head);
    }
    size_t capacity() {
      return pool.size();
    }
    T* get_next_existing() {
      if (idx_get == head) {
        idx_get = 0;
        return nullptr;
      }
      return pool[idx_get++];
    }
   private:
    void grow_pool(int _num_to_add) {
      for (int i = 0; i < _num_to_add; i++) pool.push_back(new T());
    }
    std::vector<T*> pool;
    size_t head = 0;
    size_t grow_size;
    size_t idx_get = 0;
  };

}
