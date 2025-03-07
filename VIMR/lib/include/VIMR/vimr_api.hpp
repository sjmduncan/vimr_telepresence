#pragma once

#ifdef _MSC_VER
  #ifdef VIMR_INTERFACE_INTERNAL
    #define VIMR_INTERFACE
  #else
    #ifdef VIMR_INTERFACE_EXPORT
      #define VIMR_INTERFACE __declspec(dllexport)
    #else
      #define VIMR_INTERFACE __declspec(dllimport)
    #endif
  #endif
#else
  #ifdef VIMR_INTERFACE_EXPORT
    #define VIMR_INTERFACE __attribute__((visibility("default")))
  #else
    #define VIMR_INTERFACE
  #endif
#endif
