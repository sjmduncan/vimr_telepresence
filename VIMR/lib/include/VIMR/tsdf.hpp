#pragma once

#include "serializablemessage.hpp"
#include "octree.hpp"
#include "pose.hpp"
#include "vimr_api.hpp"

namespace VIMR {
  namespace TSDF {
    using label_t = float;
    void VIMR_INTERFACE ins_dist_keep_nearest(const GridVec &_p, unsigned char *_color,label_t _dist, label_t _frac_dist, Octree &_grid, VoxelEncoding& _ve);
    void VIMR_INTERFACE ins_overwrite(const GridVec &_p, unsigned char *_color, label_t _label, label_t _frac, Octree &_grid, VoxelEncoding& _ve);
    void VIMR_INTERFACE from_octree(VoxelMessage &_hull, int _trunc,Octree &_tsdf);
    void VIMR_INTERFACE integrate(const Octree &_tsdf, Octree &_integrated, VoxelEncoding & _ve);
    void VIMR_INTERFACE extract_zero_crossing(const Octree &_tsdf, float _thresh, Octree &_hull);
  };
}