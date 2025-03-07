#pragma once

#include "serialbuffer.hpp"
#include "vimr_api.hpp"
#include "gridvec.hpp"
#include "serializable.hpp"
#include "perf.hpp"
#include <vector>
#include <map>
#include <iostream>
#include <mutex>

namespace VIMR {
  template<class T>
  class Pool;

  struct VIMR_INTERFACE Voxel {
    static constexpr size_t MAX_BYTES = 16;
    unsigned char* data = new unsigned char[MAX_BYTES]{};
    Voxel* parent{};
    Voxel* children[8]{};
    uint8_t bitfield{};
    uint64_t morton{};
    GridVec pos{};

    Voxel() = default;
    Voxel(const Voxel& _v);
    Voxel* ensure_child(unsigned int _p, unsigned int _b, Pool<Voxel>& _pool);
    void erase_child(unsigned int _idx, unsigned int _bit);
  };

  /*
   * Fixed-depth octree, with memory pooling, fast construction from RGBD images, and fast serial conversion.
   * Note that not all operations are always possible/safe - the intended order of uasge is:
   *
   * 1. Construct by ensure_voxel()
   * 2. Serialize by pack()
   * 3. Stream/record the serial encoding
   * 4. Deserialize stream/recording by unpack() (possibly also unpack_merge())
   * 5. Access voxel positions by repeated calls to get_next_voxel() (e.g. to copy to the renderer plugin)
   *
   * In other use cases some care is required:
   *
   *  - An octree constructed by Serialize::unpack() will have valid leaf node positions, and order of iteration
   *    over leaves is by 3D morton code.
   *
   *  - An octree constructed by Serialize::unpack() and then Octree::merge_from() will have valid leaf node
   *    positions but there is no iteration order guarantee. A call to finalize() restores iteration order,
   *    and is required before things like find_nearest_voxel() will produce valid results.
   *
   *  - Any call to ensure_voxel() invalidates order guarantees and may insert leaf nodes without valid positions.
   *    In this state the octree can be serialized, and things like find_nearest_voxel_approx()
   *    will work, but a call to finalize() is require dto restore iteration order and for find_nearest_voxel().
   *    The onus is on the caller of ensure_voxel() to set the leaf node positions if those are reqiured.
   *
   *  - If the point cloud density is much finer than the voxel resolution (i.e. most/all voxels will contain
   *    multiple points) the nuse pack_into instead of repeated calls to ensure_voxel().
   */
  class VIMR_INTERFACE Octree : public Serializable {
   public:
    // max depth is limited by 16-bit position indices in the voxel renderer plugin
    static constexpr uint8_t max_depth = 16;

    Octree(size_t _depth = 9, uint8_t _vox_bytes = 4, size_t _vox_pool_size = 5 * 196608, size_t _pool_grow_size = 4096);
    virtual ~Octree();

    Octree& operator=(const Octree& _other);

    /*
     * Clear all allocated voxels and then reset the node and voxel pools
     *
     * Will call memset on a bunch of node data, avoid use in time-critical threads
     */
    void clear();

    /*
     * Does not check if _p is within the bounds of the tree - use contains(_p) first if you're not sure.
     * Always returns non-null.
     *
     * Construct by repeated calls to this function if the voxel resolution is much coarser than the point cloud
     */
    Voxel* ensure_voxel(const GridVec& _p);

    /*
     * Compute level node lists required for find_nearest_voxel,
     * Note that if you're calling this then you should also have set Voxel::Pos for all inserted voxels
     */
    void finalize();

    struct ConstructionNode {
      GridVec leaf_pos{};
      char data[4]{};
      int64_t key;
      Voxel* parent;
      uint8_t bitfields[16];
    };
    /*
     * Use this if you don't need an in-meomry octree (i.e. just recoridng or streaming).
     * Faster than using ensure_voxel if the point cloud density is close to the voxel grid resolution
     * (i.e. with one cloud per point)
     */
    static int pack_into(std::vector<ConstructionNode>& _data, BufWriter& _dst, uint8_t _tdepth, uint8_t _vbytes);
    /*
     * Returns a pointer to the leaf node if an occupied voxel exists at _p
     * Returns nullptr if _p is unoccupied
     */
    Voxel* try_get_voxel(const GridVec& _p) const;

    /*
     * Find the octree with the shortest tree traverse distance to the given grid position.
     *
     * Don't call on empty octree.
     * Make sure that finalize() has been called if the octree was constructed by ensure_voxel() rather than serial decoding
     *
     * Note that this finds the voxel with the shortest distnace in tree-space, which is not
     * the same as the shortest distnace in euclidean space.
     */
    Voxel* get_nearest_voxel_approx(const GridVec& _p) const;

    /*
     * Find the occupied voxel with the shortest euclidean distance to the given grid position.
     * Starts from get_nearest_voxel_approx and then searches nearby nodes
     *
     * Don't call on empty octree.
     * Make sure that the positions have been set and finalize() has been called if the octree was constructed by ensure_voxel() rather than serial decoding
     */
    Voxel* get_nearest_voxel(const GridVec& _p) const;

    /*
     * Iterate over all *allocated* leaf nodes.
     * Order of iteration is by 3D morton encoding if constructed *only* by Serialize::unpack().
     * Order is not guaranteed after merge_from() or ensure_voxel() has been called.
     */
    Voxel* get_next_voxel() const;

    /*
     * Convenience iterator, order of iteration is always by 3D morton code but unpack() or finalize()
     * must have been called for this to work.
     *
     * Don't call ensure_voxel in the loop since that might cause reallocation of the underlying memory.
     */
    Voxel** begin();
    Voxel** end();

    /*
     * same as Serializable::unpack, but don't call clear() between unpack() and merge_from()
     */
    bool merge_from(BufReader* _srcb);

    int vox_count() const;
    int grid_width() const;
    size_t tree_depth() const;
    uint8_t vox_serial_bytes() const;
    SerializableType serial_type() const override;
    GridVec cent_shift{};

    void set_vox_serial_bytes(uint8_t _n_bytes);
    bool contains(const GridVec& _p) const;
    void dbgprint() {
      auto level = depth - 1;
      printf("d=%i, bpv=%i\n", depth, vox_size_bytes);
      printf("%i  ", level + 1);
      auto printbits = [](uint8_t _b) {
        for (int i = 0; i < 8; i++) {
          printf("%i", (_b & (1 << i)) ? 1 : 0);
        }
      };
      printbits(root->bitfield);
      printf("\n");
      trav_buffer[level + 1].clear();
      trav_buffer[level + 1].push_back(root);
      while (level) {
        trav_buffer[level].clear();
        printf("%i  ", level);
        for (auto& node: trav_buffer[level + 1]) {
          for (auto& i: node->children) {
            if (i != nullptr) {
              printbits((i)->bitfield);
              printf(" ");
              trav_buffer[level].push_back(i);
            }
          }
        }
        printf("\n");
        level--;
      }
    }
   protected:
    /*
     * Invoked by Serializable::pack
     */
    bool encode(BufWriter* _dst) override;
    /*
     * This is invoked by Serializable::unpack. Make sure that clear() is called before unpack
     */
    bool decode(BufReader* _src) override;
    void set_depth(size_t _depth);
    bool decode_into_tree(BufReader* _src);
    uint8_t depth = 0;
    std::vector<Voxel*> trav_buffer[max_depth + 1] = {};
    unsigned width{};
    uint8_t vox_size_bytes{};
    Voxel* root = new Voxel();
    Pool<Voxel>* node_pool{};
    Pool<Voxel>* vox_pool{};
    int64_t level_bits[max_depth]{};
    int64_t sign_mask{};

    static const std::map<int, int> bit_to_octant;
    static const int octant_to_bit[8];

    static const uint64_t morton[max_depth];
    static const uint64_t morton_mask[max_depth];

    static unsigned int child_octant(const GridVec& _p, int64_t _l_bit);
    static void child_pos(const GridVec& _pp, int idx, int _lshift, GridVec& _cp);
    static void child_pos_sign(const GridVec& _pp, int idx, int64_t _smask, GridVec& _cp);
  };

}

