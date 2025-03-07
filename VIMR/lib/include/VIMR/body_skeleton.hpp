#pragma once

#include "pose.hpp"
#include "vimr_api.hpp"

namespace VIMR {
  // This follows the Kinect2 joint naming convention
  // Repeated here to avoid requiring Kinect.h outside of the camera module
  enum JointType {
    JointType_SpineBase = 0,
    JointType_SpineMid  = 1,
    JointType_Neck  = 2,
    JointType_Head  = 3,
    JointType_ShoulderLeft  = 4,
    JointType_ElbowLeft = 5,
    JointType_WristLeft = 6,
    JointType_HandLeft  = 7,
    JointType_ShoulderRight = 8,
    JointType_ElbowRight  = 9,
    JointType_WristRight  = 10,
    JointType_HandRight = 11,
    JointType_HipLeft = 12,
    JointType_KneeLeft  = 13,
    JointType_AnkleLeft = 14,
    JointType_FootLeft  = 15,
    JointType_HipRight  = 16,
    JointType_KneeRight = 17,
    JointType_AnkleRight  = 18,
    JointType_FootRight = 19,
    JointType_SpineShoulder = 20,
    JointType_HandTipLeft = 21,
    JointType_ThumbLeft = 22,
    JointType_HandTipRight  = 23,
    JointType_ThumbRight  = 24,
    JointType_Count = ( JointType_ThumbRight + 1 )
  };

  // This follows the originla VBMR naming convention
  // Lower numbers override higher numbers when body part volumes overlap
  typedef enum BodyPartType {
    Head = 0,
    Neck = 1,
    LeftHand = 2,
    RightHand = 3,
    LeftFoot = 4,
    RightFoot = 5,
    LeftShoulder = 6,
    RightShoulder = 7,
    LeftUpperArm = 8,
    RightUpperArm = 9,
    LeftForearm = 10,
    RightForearm = 11,
    LeftHip = 12,
    RightHip = 13,
    LeftThigh = 14,
    RightThigh = 15,
    LeftCalf = 16,
    RightCalf = 17,
    UpperTorso = 18,
    LowerTorso = 19,
    // End of valid body parts
    BodyPartType_Count = 20,
    InvalidBodyPart = 255
  } BodyPartType;

  // Defining the skeleton
  struct Bone {
    JointType start;// 'parent' joint in VBMR, away from extremity
    JointType end;  // 'child' in VBMR, towards extremity
  };
  extern const Bone skeleton[BodyPartType_Count];

  const VIMR_INTERFACE Bone*  get_bone(int _bodyPartType);
  bool VIMR_INTERFACE load_body(const char* _file_in, Pose* _j);
  bool VIMR_INTERFACE save_body(const char* _file_out, Pose* _j, bool _recenter);
}
