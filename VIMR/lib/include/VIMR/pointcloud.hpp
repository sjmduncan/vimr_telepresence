// Copyright (C) 2021 Stuart Duncan
//
// This file is part of VIMR.
//
// VIMR is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// VIMR is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with VIMR.  If not, see <http://www.gnu.org/licenses/>.
#pragma once

#include "Eigen/Geometry"
#include "serialbuffer.hpp"
#include "vimr_api.hpp"
#include "serializable.hpp"
#include <sstream>
#include <fstream>
#include <vector>
#include <utility> // for std::pair

namespace VIMR {
  class VIMR_INTERFACE PointCloud : public Serializable {
   protected:
    bool encode(BufWriter* _dst) override{
      return append_to(_dst);
    }
    bool decode(BufReader* _src) override{
      return load_from(_src);
    }
   public:
    SerializableType serial_type() const override {
      return SerializableType::PointCloud;
    }
    using Point = std::pair<Eigen::Vector3d, std::vector<uint8_t>>;
    unsigned long long timestamp_ms{};

    PointCloud() = default;

    size_t size() const {
      return points.size();
    }

    void reset(unsigned long long int _t_ms) {
      points.clear();
      timestamp_ms = _t_ms;
    }

    void copy_from(const PointCloud& _ptcloud) {
      timestamp_ms = _ptcloud.timestamp_ms;
      points.clear();
      for (const auto& p : _ptcloud.points) points.emplace_back(p);
    }

    bool append_to(BufWriter* _b) {
      if (!_b->put(timestamp_ms)) return false;
      size_t size = points.size();
      if (!_b->put(size)) return false;
      for (auto p : points) {
        if (!_b->put((char*)p.first.data(), sizeof(double) * 3)) return false;
        if (!_b->put((char*)p.second.data(), 3)) return false;
      }
      return true;
    }

    bool load_from(BufReader* _b) {
      if (!_b->pop(timestamp_ms)) return false;
      size_t size = points.size();
      if (!_b->pop(size)) return false;
      while (points.size() < size) {
        points.emplace_back(Eigen::Vector3d(0, 0, 0), std::vector<uint8_t>{ 0, 0, 0 });
        if (!_b->pop((char*)points.back().first.data(), sizeof(double) * 3)) return false;
        if (!_b->pop((char*)points.back().second.data(), 3)) return false;
      }
      return true;
    }

    bool save_ply(const char* _file_path) const;

    bool load_ply(const char* _file_path);

    void append(double _x, double _y, double _z, uint8_t _r, uint8_t _g, uint8_t _b) {
      points.emplace_back(Eigen::Vector3d(_x, _y, _z), std::vector<uint8_t>{ _r, _g, _b });
    }
    std::vector<Point> points;
  };
}