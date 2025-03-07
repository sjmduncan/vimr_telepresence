#pragma once
#include <json.hpp>
#include "vimr_api.hpp"
#include "voxelvideo.hpp"

using nlohmann::json;

namespace VIMR
{
  VIMR_INTERFACE void from_json(const nlohmann::json& j, VoxelVideo::AudioStream& p);
  VIMR_INTERFACE void to_json(nlohmann::json& j, const VoxelVideo::AudioStream& p);
  VIMR_INTERFACE void from_json(const nlohmann::json& j, VoxelVideo::Metadata& p);
  VIMR_INTERFACE void to_json(nlohmann::json& j, const VoxelVideo::Metadata& p);
}
