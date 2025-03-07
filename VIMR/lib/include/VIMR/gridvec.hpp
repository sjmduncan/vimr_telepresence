#pragma once

#include "vimr_api.hpp"
#include <cstdint>
#include <cmath>

namespace VIMR {
  class VIMR_INTERFACE GridVec {
   public:
    ~GridVec() = default;
    GridVec() = default;
    GridVec(const GridVec& _g) = default;
    GridVec& operator=(const GridVec& _g) = default;
    template<typename T>
    GridVec(T _x, T _y, T _z) {
      x = static_cast<int64_t>(floor(_x));
      y = static_cast<int64_t>(floor(_y));
      z = static_cast<int64_t>(floor(_z));
    }
    template<typename T>
    GridVec(T* _data) {
      x = static_cast<int64_t>(floor(_data[0]));
      y = static_cast<int64_t>(floor(_data[1]));
      z = static_cast<int64_t>(floor(_data[2]));
    }
    template<typename T>
    GridVec(T _val) {
      auto v = static_cast<int64_t>(floor(_val));
      x = v;
      y = v;
      z = v;
    }
    GridVec& operator+=(const GridVec& _g) {
      x += _g.x;
      y += _g.y;
      z += _g.z;
      return *this;
    }
    GridVec& operator-=(const GridVec& _g) {
      x -= _g.x;
      y -= _g.y;
      z -= _g.z;
      return *this;
    }
    friend GridVec operator-(GridVec _lhs, const GridVec& _rhs) {
      _lhs -= _rhs;
      return _lhs;
    }
    friend GridVec operator+(GridVec _lhs, const GridVec& _rhs) {
      _lhs += _rhs;
      return _lhs;
    }
    friend GridVec operator*(int _sf, const GridVec& _v) {
      return {_sf * _v.x, _sf * _v.y, _sf * _v.z};
    }
    friend GridVec operator*(double _sf, const GridVec& _v) {
        return { _sf * _v.x, _sf * _v.y, _sf * _v.z };
    }
    friend GridVec operator/(const GridVec& _v, int _sf) {
      return {_sf / _v.x, _sf / _v.y, _sf / _v.z};
    }
    int64_t dot(GridVec& _g) const {
      return x * _g.x + y * _g.y + z * _g.z;
    }
    double length() const {
      return sqrt(static_cast<double>(x * x + y * y + z * z));
    }
    double length_cent() const {
      const auto xx = static_cast<double>(x) + 0.5;
      const auto yy = static_cast<double>(y)+ 0.5;
      const auto zz = static_cast<double>(z) + 0.5;
      return sqrt(xx * xx + yy * yy + zz * zz);
    }
    GridVec abs_elem() const {
      return {std::abs(x) + std::abs(y) + std::abs(z)};
    }
    int64_t abs() const{
      return std::abs(x) + std::abs(y) + std::abs(z);
    }
    friend std::ostream& operator << ( std::ostream& os, GridVec const& _g ) {
      os << _g.x <<"," << _g.y << "," << _g.z;
      return os;
    }
    friend bool operator==(const GridVec& _lhs, const GridVec& _rhs) {
      return (_lhs.x == _rhs.x) && (_lhs.y == _rhs.y) && (_lhs.z == _rhs.z);
    }
    int64_t x = 0, y = 0, z = 0;
  };
}