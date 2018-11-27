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
#pragma pack (pop)

  /// The string that is always stored in vhd_footer::cookie.
  ///
  const uint8_t VHD_COOKIE[8] = {'c', 'o', 'n', 'e', 'c', 't', 'i', 'x'};

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

    static bool is_vhd_format_file(std::string &filename);

  protected:
    std::fstream backing_file;
    vhd_footer footer_copy;
    uint64_t total_file_length;
  };
};