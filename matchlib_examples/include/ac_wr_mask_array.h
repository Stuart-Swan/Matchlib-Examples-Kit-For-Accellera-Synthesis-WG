#pragma once

// Author: Stuart Swan, Platform Architect, Siemens EDA
// 10 December 2024

// ac_wr_mask_array_1D is a 1-dimensional memory that includes "ByteEnable" support.
// The NumSlices parameter specifies the number of independent slices of each element
// of the memory. The slices do not need to be exactly 8 bits wide.
// 
// Usage:
//   ac_wr_mask_array<uint32, 0x1000, 4> mem;
//   mem[0x100] = 0x123;  // write all slices at address 0x100
//   mem[0x100].mask(3) = 0xffff; // write 2 low-order bytes at address 0x100
//   uint32 val = mem[0x100];  // read all slices at address 0x100

#include <mem_array.h>
#include <ac_assert.h>
#include "ac_bank_array.h"


template <typename T, int Dim1, int NumSlices=1>
class ac_wr_mask_array_1D {
  public:

  static const unsigned num_slices = NumSlices;
  static const unsigned dim1 = Dim1;
  typedef ac_int<NumSlices> WriteMask;
  typedef Wrapped<T> WData_t;
  static const unsigned int WordWidth = WData_t::width;
  static const unsigned int SliceWidth = WordWidth/NumSlices;
  typedef sc_lv<WordWidth> Data_t;
  typedef sc_lv<SliceWidth> Slice_t;
  ac_bank_array_2D<Slice_t, NumSlices, Dim1> mem;


  struct elem_proxy {
    ac_wr_mask_array_1D& array;
    unsigned idx;
    ac_wr_mask_array_1D::WriteMask mask_val{~0};

    elem_proxy(ac_wr_mask_array_1D& _array, unsigned _idx, ac_wr_mask_array_1D::WriteMask _mask=~0) 
     : array(_array), idx(_idx), mask_val(_mask) {} 

    operator T () { 
      assert(idx < ac_wr_mask_array_1D::dim1);
      Data_t read_data = TypeToBits<NVUINTW(ac_wr_mask_array_1D::WordWidth)>(0);
      #pragma hls_unroll yes
      for (unsigned i = 0; i < ac_wr_mask_array_1D::num_slices; i++) {
        read_data.range((i+1)*SliceWidth-1, i*SliceWidth) = array.mem[i][idx];
      }

      CMOD_ASSERT_MSG(read_data.xor_reduce()!=sc_logic('X'), "Read data is X");
      return BitsToType<T>(read_data);
    }

    void operator=(const T& val) { 
      assert(idx < ac_wr_mask_array_1D::dim1);
      Data_t write_data = TypeToBits<T>(val);
      #pragma hls_unroll yes
      for (unsigned i = 0; i < ac_wr_mask_array_1D::num_slices; i++) {
        if (mask_val[i] == 1)
          array.mem[i][idx] = write_data.range((i+1)*SliceWidth-1, i*SliceWidth);
      }
    }

    elem_proxy mask(ac_wr_mask_array_1D::WriteMask _mask) {
      return elem_proxy(array, idx, _mask);
    }
  };

  elem_proxy operator[](unsigned idx) { return elem_proxy(*this, idx); }
  const elem_proxy operator[](unsigned idx) const { return elem_proxy(*this, idx); }
};
