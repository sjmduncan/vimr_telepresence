#pragma once

#include <functional>
#include <gridvec.hpp>

namespace VIMR {
  using bres_update = std::function<void(GridVec)>;
  class Bresenham {
    static GridVec bres_sign(const GridVec& _v) {
      return {(_v.x < 0) ? -1 : 1, (_v.y < 0) ? -1 : 1, (_v.z < 0) ? -1 : 1};
    }
    static GridVec bres_shift(const GridVec& _v) {
      return {_v.x << 1, _v.y << 1, _v.z << 1};
    }
   public:
    // Computes the bresenham approximation of a line in 3D space
    // The callback is invoked incrementally as the line traverses the grid
    // If the callback returns false then the algorithm terminates early
    // otherwise it traces the whole line from _p1 to _p2
    static void bres_line_3D(const GridVec& _p1, const GridVec& _p2, const bres_update & _cb) {
      GridVec d_signed = _p2 - _p1;
      GridVec d_mag = d_signed.abs_elem();
      GridVec inc = bres_sign(d_signed);
      GridVec err_inc = bres_shift(d_mag);

      GridVec p = _p1;

      if ((d_mag.x >= d_mag.y) && (d_mag.x >= d_mag.z)) {
        int64_t ey = err_inc.y - 1;
        int64_t ez = err_inc.z - 1;
        for (auto i = 0; i < d_mag.x; i++) {
          _cb(p);
          if (ey > 0) {
            p.y += inc.y;
            ey -= err_inc.x;
          }
          if (ez > 0) {
            p.z += inc.z;
            ez -= err_inc.x;
          }
          ey += err_inc.y;
          ez += err_inc.z;
          p.x += inc.x;
        }
      }
      else if ((d_mag.y >= d_mag.x) && (d_mag.y >= d_mag.z)) {
        int64_t ex = err_inc.x - 1;
        int64_t ez = err_inc.z - 1;
        for (auto i = 0; i < d_mag.y; i++) {
          _cb(p);
          if (ex > 0) {
            p.x += inc.x;
            ex -= err_inc.y;
          }
          if (ez > 0) {
            p.z += inc.z;
            ez -= err_inc.y;
          }
          ex += err_inc.x;
          ez += err_inc.z;
          p.y += inc.y;
        }
      }
      else {
        int64_t ex = err_inc.x - 1;
        int64_t ey = err_inc.y - 1;
        for (auto i = 0; i < d_mag.z; i++) {
          _cb(p);
          if (ex > 0) {
            p.x += inc.x;
            ex -= err_inc.z;
          }
          if (ey > 0) {
            p.y += inc.y;
            ey -= err_inc.z;
          }
          ex += err_inc.x;
          ey += err_inc.y;
          p.z += inc.z;
        }
      }
      _cb(p);
    }
  };
}