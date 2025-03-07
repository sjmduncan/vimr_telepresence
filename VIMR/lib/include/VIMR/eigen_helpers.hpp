#pragma once

#include "Eigen/Geometry"
#include <fstream>
#include <string>
#include <vector>
#include "pose.hpp"
#include "vimr_api.hpp"

namespace VIMR {
  template<typename M>
  M load_mat_csv(const char* _path) {
    std::ifstream data_in;
    data_in.open(_path);
    if (!data_in.is_open()) {
      throw std::exception();
    }
    std::string line;
    std::vector<double> values;
    unsigned int rows = 0;
    while (std::getline(data_in, line)) {
      std::stringstream line_stream(line);
      std::string cell;
      while (std::getline(line_stream, cell, ',')) {
        values.push_back(std::stod(cell));
      }
      ++rows;
    }
    if (rows == 0) throw std::exception();
    data_in.close();
    return Eigen::Map<const Eigen::Matrix<typename M::Scalar, M::RowsAtCompileTime, M::ColsAtCompileTime, Eigen::RowMajor>>(
      values.data(), rows, values.size() / rows);
  }

  /*
   * Creates a transform which reflects points through a plane defined by a normal vector _n and a point _p
   */
  Eigen::Affine3d VIMR_INTERFACE make_reflector(Eigen::Vector3d _n, const Eigen::Vector3d& _p);
  void VIMR_INTERFACE from(unsigned long long _t_ms, const Eigen::Affine3d& _tx, VIMR::Pose & _out);
  void VIMR_INTERFACE from(unsigned long long _t_ms, const Eigen::Matrix4d& _tx, VIMR::Pose & _out);
  Eigen::Affine3d VIMR_INTERFACE from(const Pose& _p);
  std::string to_csv(const Pose &_p, int _precision);
  extern Eigen::IOFormat full_precision;
}
