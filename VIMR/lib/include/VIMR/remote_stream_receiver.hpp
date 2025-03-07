#pragma once

#include "vimr_component.hpp"
#include "netstream.hpp"
#include "config.hpp"
#include "async.hpp"

namespace VIMR
{
  class VoxStreamReceiver
  {
    Network::VNetStream<SerialMessage>* receiver{};
    BufferProcessor<SerialMessage>* deserializer{};
    BufferProcessor<VoxelMessage>* voxbuffer{};
    VoxelCallback callback{};
    ConfigFile config;
  public:
    VoxStreamReceiver();
    bool init(const char* _inst_config_file);
    bool init_vnet_stream(const char* _owner_inst_id, const char* _inst_id, const char* _vnet_addr, bool _is_lan);

    int get_instance_id(char** id_out);

    bool set_vox_sink(const VoxelCallback& _callback);
  };
}
