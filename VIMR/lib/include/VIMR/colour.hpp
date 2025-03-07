#pragma once

#include "octree.hpp"

namespace VIMR {
  /*
   * A class for compressing colours from 24 bits to 8 or fewer
   */
  class VIMR_INTERFACE Colour {
   public:
    static unsigned char quantize(const unsigned char* _triple);
    static void unquantize(unsigned char _c, unsigned char* _color_triple);
    static void unquantize(unsigned char _c, unsigned char& _r, unsigned char& _g, unsigned char& _b);

    static unsigned char togray(const unsigned char* _triple);
    static void fromgray(unsigned char _c, unsigned char* _color_triple);

    static void copy_triple(unsigned char* _dst, const unsigned char* _src);
    static void copy_triple_reverse(unsigned char * _dst, const unsigned char * _src);
  };

  class VIMR_INTERFACE ColourPalette
  {
  public:
    static constexpr size_t n_colors = 256;
    unsigned char palette[n_colors * 3]{};
    void build_palette(const unsigned char* _colours);
    unsigned char reduce(const unsigned char* _rgb) const;
    void lookup(unsigned char _d, unsigned char* _rgb) const;
    void copy_from(const ColourPalette & _other);
  protected:
    static unsigned int color_to_morton(const unsigned char* _c);
    unsigned int palette_mortons[n_colors]{};
  };
  
  class VIMR_INTERFACE ColourTree : public Octree {
    void reduce(size_t _palette_size);
   public:
    ColourTree();
    void create(Octree& _o_colours, size_t _palette_size = 256);
    void create(const unsigned char* _colours, size_t _stride, size_t _n_size, size_t _palette_size = 256);
    unsigned char reduce(const unsigned char* _color_triple) const;
    void lookup(unsigned char _c, unsigned char* _color_triple) const;
    size_t palette_size() const;
    SerializableType serial_type() const override;
    std::vector<std::vector<unsigned char>> get_palette();
  };
  
  extern const VIMR_INTERFACE uint8_t palette[27][3];
}

