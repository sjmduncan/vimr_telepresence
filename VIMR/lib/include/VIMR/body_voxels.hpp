#pragma once

#include "body_cylinders.hpp"
#include "serializablemessage.hpp"
#include "octree.hpp"
#include "vimr_api.hpp"
#include "freq_estimation.hpp"
#include <map>

namespace VIMR {
  class VIMR_INTERFACE VoxelBody {
  public:
    VoxelBody();

    // LVTRP is left-voxels to right-pose. Voxels belonging to left limbs
    // are transformed so bone-to-bone angles of the (transformed) left limb
    // mirror the angles of the right limb.
    // RVTLP is right-voxels to left-pose (i.e. the opposite of LVTRP)
    typedef enum BoneAngleTransform {
      BAM_LVTRP = 2, BAM_RVTLP = 1, BAM_MIRROR = 2, BAM_NONE = 0, BAM_LABEL_ONLY = 4
    } BoneAngleTransform;

    bool update(Octree * _m, VoxelEncoding & _ve);

    bool label_only(Octree *_m, VoxelEncoding &_ve);

    // Label voxels and apply the Bone-Angle-Map transform in one pass
    bool label_and_bam(Octree *_m, VoxelEncoding & _vd);

    // Swap left and right body part across the sagittal plane.
    // Don't apply labels
    bool sagittal_swap(Octree *_m, VoxelEncoding & _ve);

    void set_transform_type(BoneAngleTransform _ang_src);
    BoneAngleTransform get_transform_type();
    void suppress_healthy(bool _s);

    // Set the 'hide' flag for a body part. This suppresses it during rendering
    // but the voxels remain in the Octree and will be visible when rendering
    // special voxels.
    // Only applies when one of the Label* functions is  called.
    void hide(BodyPartType _t) { parts_hidden[_t] = true; }

    // Un-set the 'hide' flag for a body part.
    void show(BodyPartType _t) { parts_hidden[_t] = false; }

  private:
    bool update_reflector(Vector3d *_v_1);

    bool parts_hidden[BodyPartType_Count] = {false};

    AvgBuffered<Vector3d, 4> joint_avgbuf[JointType_Count];

    static size_t update_skeleton(const Octree* _o, Vector3d *_joints, bool *_joint_valid, AvgBuffered<Vector3d, 4>* _joints_averaged, VoxelEncoding & _ve);

    void estimate_transforms(BodyCylinders _body_cylinders);

    struct part_map {
      BodyPartType ang; // body part label to derive target limb direction from
      BodyPartType vox; // Body part label to apply transforms to
    };

    void estimate_axial(part_map _start_dir, Eigen::Affine3d &_end_dir, Vector3d *_end_2);

    BoneAngleTransform transform_type = BAM_LVTRP;

    std::map<BodyPartType, Eigen::Affine3d> bam_trans;

    std::vector<part_map> limbs[2];

    bool do_transform(BodyPartType _t);

    bool transform_part[BodyPartType_Count]{};

    Eigen::Affine3d sagittal_pos_reflector[2];
    Eigen::Affine3d sagittal_direction_reflector[2];
  };
}
