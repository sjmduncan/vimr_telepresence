#pragma once

#include "Eigen/Geometry"
#include "vimr_api.hpp"
#include "body_skeleton.hpp"
#include "octree.hpp"

// This is a re-implementation of the cylinder-volume-classifier from VBMR.
// It's a little more complete than the original (Head and Neck voxels are
// actually labeled, for example) and there are a few bugfixes.

using Eigen::Vector3d;
namespace VIMR {
  class VIMR_INTERFACE CylVolume {
    double invlsq{};
    double lsq{};
    double rsq{};

   public:
    double length{};
    double rad{};
    Eigen::Vector3d pos{};
    Eigen::Vector3d dir{};
    CylVolume(double _r) {
      set_radius(_r);
    }
    void set_radius(double _r) {
      rsq = _r * _r;
    }
    void update(const Vector3d& _start, const Vector3d& _end) {
      dir = _end - _start;
      pos = _start;
      length = dir.norm();
      lsq = length * length;
      invlsq = 1.0 / lsq;
    }

    bool contains(Vector3d _p) const {
      // The VBMR cylinder check was taken from here:
      // https://flipcode.com/archives/Fast_Point-In-Cylinder_Test.shtml
      // This implementation is the same, but uses Vector3d
      Vector3d pd = _p - pos;

      double dot = pd.dot(dir);

      if (dot < 0 || dot > lsq) {
        return false;
      }
      else {
        double dist_to_ax_sq = pd.dot(pd) - dot * dot * invlsq;
        if (dist_to_ax_sq > rsq) {
          return false;
        }
      }
      return true;
    }
  };

    class VIMR_INTERFACE BodyCylinders {
   protected:
    double radii_m[BodyPartType_Count]{};
    double radii_vox[BodyPartType_Count]{};
    double m_to_vox = -1;
    CylVolume* part_cyls[BodyPartType_Count]{};
    void update_vox_scale(double _m_to_vox) {
      if (m_to_vox != _m_to_vox) {
        m_to_vox = _m_to_vox;
        for (int b = 0; b < BodyPartType_Count; b++) {
          radii_vox[b] = m_to_vox * radii_m[b];
          part_cyls[b]->set_radius(radii_vox[b]);
        }
      }
    }

   public:
    BodyCylinders() {
      // Body part radii are in meters, call UpdateVoxScale to make sure the VoxScale matches
      // whatever voxel grid the joints came from.
      radii_m[LeftHand] = 0.12 / 2.00;
      radii_m[RightHand] = 0.12 / 2.00;
      radii_m[LeftFoot] = 0.125 / 2.00;
      radii_m[RightFoot] = 0.125 / 2.00;
      radii_m[Head] = 0.6 / 2.00;
      radii_m[Neck] = 0.1 / 2.00;
      radii_m[LeftShoulder] = 0.2 / 2.00;
      radii_m[RightShoulder] = 0.2 / 2.00;
      radii_m[LeftUpperArm] = 0.2 / 2.00;
      radii_m[RightUpperArm] = 0.2 / 2.00;
      radii_m[LeftForearm] = 0.15 / 2.00;
      radii_m[RightForearm] = 0.15 / 2.00;
      radii_m[UpperTorso] = 0.6 / 2.00;
      radii_m[LowerTorso] = 0.6 / 2.00;
      radii_m[LeftHip] = 0.4 / 2.00;
      radii_m[RightHip] = 0.4 / 2.00;
      radii_m[LeftThigh] = 0.25 / 2.00;
      radii_m[RightThigh] = 0.25 / 2.00;
      radii_m[LeftCalf] = 0.2 / 2.00;
      radii_m[RightCalf] = 0.2 / 2.00;

      for (int b = 0; b < BodyPartType_Count; b++) {
        radii_vox[b] = 1000 * radii_m[b];
        part_cyls[b] = new CylVolume(radii_vox[b]);
      }
    }
    void
    update(Vector3d* _joints, double _m_to_vox) {
      update_vox_scale(_m_to_vox);
      for (int i = 0; i < BodyPartType_Count; i++) {
        if (i == BodyPartType::Head || i == BodyPartType::LeftHand || i == BodyPartType::RightHand) {
          Vector3d extn = _joints[skeleton[i].end] - _joints[skeleton[i].start];
          part_cyls[i]->update(_joints[skeleton[i].start], _joints[skeleton[i].end] + extn);
        }
        else {
          part_cyls[i]->update(_joints[skeleton[i].start], _joints[skeleton[i].end]);
        }
      }
    }
    /*
     * Return the non-normalized direction vector from the play to the end of the body part (the bone vector)
     */
    Vector3d get_dir(BodyPartType _t) {
      return part_cyls[_t]->dir;
    }
    Vector3d get_start_pos(BodyPartType _t) {
      return part_cyls[_t]->pos;
    }

    BodyPartType classify(Vector3d _pt) {
      BodyPartType t = InvalidBodyPart;
      for (int i = 0; i < BodyPartType_Count; i++) {
        if (i == LeftHip || i == RightHip);
        else if (part_cyls[i]->contains(_pt)) {
          t = static_cast<BodyPartType>(i);
          break;
        }
      }
      return t;
    }
  };
}
