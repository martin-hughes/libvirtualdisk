/// @file
/// @brief Declares a class implementing a VirtualBox VDI format container.

// Copyright Martin Hughes 2018.

#include "virtualdisk.h"

#include <memory>

namespace virt_disk
{
#pragma pack ( push , 1 )

  /// @brief Header of a v1.1 VDI file.
  ///
  /// This structure is the header of a VDI file of format version 1.1. Other versions are not yet supported. The first
  /// 4 fields are the "pre-header" that should be version-independent. After this header in the file is a bunch of
  /// UUID and other data that I have little interest in.
  struct vdi_header
  {
    /// Text to describe the file format - usually "<<< Oracle VM VirtualBox Disc Image >>>\n" - although we don't
    /// actually care about this.
    char info_test[64];

    /// Confirms the file type - should be 0xbeda107f.
    uint32_t magic_number;

    /// Should be 1.
    unsigned short version_minor;

    /// Should be 1.
    unsigned short version_major;

    /// Size of the header - excluding the pre-header...
    uint32_t header_len;

    /// The type of the file - dynamic, static, etc. We support 1 (normal) and 2 (fixed). Others are not supported.
    uint32_t file_type;

    /// Image flags - No idea what flags are valid, always seems to be zero...
    uint32_t image_flags;

    /// Image comment - optional.
    char comment[256];

    /// Byte-offset of the blocks table from the beginning of the image file.
    uint32_t block_data_offset;

    /// Byte-offset of the image data from the beginning of the image file.
    uint32_t image_data_offset;

    /// Disk geometry data. Number of cylinders - ignored by the library.
    uint32_t geo_cylinders;

    /// Disk geometry data. Number of heads - ignored by the library.
    uint32_t geo_heads;

    /// Disk geometry data. Number of sectors - ignored by the library.
    uint32_t geo_sectors;

    /// Sector size in bytes.
    uint32_t sector_size;

    /// Ignored.
    uint32_t unused_1;

    /// Total size of disk, in bytes.
    uint64_t disk_size;

    /// Size of a block in this file, in bytes.
    uint32_t image_block_size;

    /// Additional data pre-pended to each block, in bytes (must be power of two). Only zero is supported at the moment.
    uint32_t image_block_extra_size;

    /// Number of blocks in the simulated disk.
    uint32_t number_blocks;

    /// Number of blocks allocated in this image.
    uint32_t number_blocks_allocated;
  };
#pragma pack ( pop )

  /// The magic number of .VDI files.
  const uint32_t VDI_MAGIC_NUM = 0xbeda107f;

  /// Constant representing variably sized .VDI files.
  const uint32_t VDI_TYPE_NORMAL = 1;

  /// Constant representing fixed-size .VDI files.
  const uint32_t VDI_TYPE_FIXED_SIZE = 2;

  /// @brief Represents a VirtualBox VDI format disk image.
  ///
  /// This class is not directly exposed by including "virtualdisk.h", but can be instantiated directly if
  /// "virt_disk_vdi.h" is used.
  class vdi_disk : public virt_disk
  {
  public:
    vdi_disk(std::string &filename);
    ~vdi_disk() { };

    virtual void read(void *buffer, uint64_t start_posn, uint64_t length, uint64_t buffer_length) override;
    virtual void write(const void *buffer, uint64_t start_posn, uint64_t length, uint64_t buffer_length) override;

    virtual uint64_t get_length() override;

  protected:

    /// The file object representing the actual file we're treating as a virtual machine hard disk.
    std::fstream backing_file;

    /// A buffered copy of the header of the .VDI file.
    vdi_header file_header;

    /// A buffered copy of the block-to-disk map in the .VDI file.
    std::unique_ptr<uint32_t[]> block_map;

    /// Whether or not this object is constructed and operating correctly.
    bool is_ok;

    // Magic numbers.

    // Member functions.

    void read_one_block(uint64_t start_block, uint32_t block_offset, uint32_t bytes_this_block, uint8_t *buffer);
  };
};