/// @file
/// @brief Declaration of a class to handle Microsoft VHD format files

#pragma once

#include "virtualdisk.h"

#include <memory>
#include <boost/endian/conversion.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/endian/arithmetic.hpp>

using namespace boost::endian;

namespace virt_disk
{
#pragma pack (push, 1)

  /// @brief Constants that define the known types of VHD file.
  namespace vhd_disk_type
  {
    const uint32_t NONE = 0; ///< Invalid file format.
    const uint32_t FIXED = 2; ///< Fixed-size format.
    const uint32_t DYNAMIC = 3; ///< Dynamic-size format - not yet supported.
    const uint32_t DIFFERENCING = 4; ///< Differencing format - not yet supported.
  };

  /// @brief Structure of the footer for VHD files
  ///
  /// This is the main control structure for VHD files, and it lives right at the end of the file. This structure uses
  /// big-endian integers.
  struct vhd_footer
  {
    uint64_t cookie; ///< Always the string 'conectix'
    big_uint32_t features; ///< Must be set to 2 for our purposes.
    big_uint32_t format_version; ///< Must be set to 0x00010000
    big_uint64_t data_offset; ///< Offset to the control structure in non-fixed size files, ~0 in fixed sized files.
    big_uint32_t timestamp; ///< Creation date in the number of seconds since 12:00 1st Jan 2000.
    big_uint32_t creater_app; ///< Short string defining the creating application
    big_uint32_t creator_version; ///< Short string containing the creator's version number
    big_uint32_t creator_host_os; ///< Short string containing the creator's host OS.
    big_uint64_t original_size; ///< The original size of this file, in bytes.
    big_uint64_t current_size; ///< The current size of this file, in bytes.
    big_uint32_t disk_geometry; ///< The geometry of this disk stored as CChs
    big_uint32_t disk_type; ///< The storage type of this VHD file. Only 2 (FIXED size) is supported at present.
    big_uint32_t checksum; ///< One's complement of all the bytes in the header, except this field.
    uint8_t unique_id[16]; ///< A unique ID for this file,
    uint8_t saved_state; ///< Whether or not the disk is in the 'saved state'
    uint8_t reserved[427]; ///< Set to zero.
  };
  static_assert(sizeof(vhd_footer) == 512, "sizeof(vhd_footer) != 512, check compiler packing options");

  struct vhd_parent_locator
  {
    uint8_t string[24];
  };

  struct vhd_dynamic_header
  {
    char cookie[8];
    big_uint64_t data_offset;
    big_uint64_t table_offset;
    big_uint32_t header_version;
    big_uint32_t max_table_entries;
    big_uint32_t block_size;
    big_uint32_t checksum;
    uint8_t parent_unique_id[16];
    big_uint32_t parent_time_stamp;
    big_uint32_t reserved_1;
    uint8_t parent_unicode_name[512];
    vhd_parent_locator parent_locators[8];
    uint8_t reserved_2[256];
  };
  static_assert(sizeof(vhd_dynamic_header) == 1024, "Sizeof vhd_dynamic_header wrong");
#pragma pack (pop)

  /// The string that is always stored in vhd_footer::cookie.
  ///
  const uint8_t VHD_COOKIE[8] = {'c', 'o', 'n', 'e', 'c', 't', 'i', 'x'};

  /// The string that is stored in the vhd_dynamic_header::cookie.
  const uint8_t VHD_DYNAMIC_COOKIE[8] = {'c', 'x', 's', 'p', 'a', 'r', 's', 'e'};

  /// The only version of the VHD specification we support.
  ///
  const uint32_t VHD_SUPPORTED_VERSION = 0x00010000;

  /// @brief Represents a VHD format virtual hard disk.
  ///
  /// At present, only the fixed-size version of this format is supported.
  class vhd_disk : public virt_disk
  {
  public:
    vhd_disk(std::string &filename);
    ~vhd_disk() { };

    virtual void read(void *buffer, uint64_t start_posn, uint64_t length, uint64_t buffer_length) override;

    virtual uint64_t get_length() override;

  protected:
    std::fstream backing_file;
    vhd_footer footer_copy;
    uint64_t total_file_length;
    vhd_dynamic_header dynamic_header_copy;

    uint16_t data_block_bitmap_bytes;
    std::unique_ptr<boost::endian::big_uint32_t[]> block_allocation_table;

    static_assert(sizeof(boost::endian::big_uint32_t) == sizeof(uint32_t), "Wrong endian type size");

    virtual void read_block(void *buffer, uint64_t start_posn, uint64_t length);
  };
};