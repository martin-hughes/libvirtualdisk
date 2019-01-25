/// @file
/// @brief Implements a container for the VirtualBox VDI format.

// Copyright Martin Hughes 2018.

#include "virtualdisk/virtualdisk.h"
#include "virtualdisk/virt_disk_vdi.h"

namespace virt_disk
{
  /// @brief Constructs a vdi_disk object.
  ///
  /// This object parses VirtualBox .VDI format disk images.
  ///
  /// @param filename The filename of the disk image to open.
  vdi_disk::vdi_disk(std::string &filename) :
    backing_file{filename, std::fstream::binary | std::fstream::in | std::fstream::out},
    is_ok{false}
  {
    if (!backing_file)
    {
      throw std::fstream::failure("Failed to open backing file");
    }

    backing_file.exceptions(std::fstream::failbit | std::fstream::eofbit | std::fstream::badbit);

    backing_file.seekg(0);
    backing_file.read(reinterpret_cast<char *>(&file_header), sizeof(vdi_header));

    if ((file_header.magic_number == VDI_MAGIC_NUM) &&
        (file_header.version_major == 1) &&
        (file_header.version_minor == 1) &&
        (file_header.image_block_extra_size == 0) &&
        ((file_header.file_type == VDI_TYPE_NORMAL) ||
          (file_header.file_type == VDI_TYPE_FIXED_SIZE)))
    {
      is_ok = true;
    }

    if (!is_ok)
    {
      throw std::fstream::failure("Failed to construct disk image object");
    }

    block_map = std::unique_ptr<uint32_t[]>(new uint32_t[file_header.number_blocks_allocated]);
    uint64_t block_map_bytes = file_header.number_blocks_allocated * sizeof(uint32_t);
    backing_file.seekg(file_header.block_data_offset);
    backing_file.read(reinterpret_cast<char *>(block_map.get()), block_map_bytes);

    if (!backing_file)
    {
      throw std::fstream::failure("Image file fstream failed");
    }
  }

  void vdi_disk::read(void *buffer, uint64_t start_posn, uint64_t length, uint64_t buffer_length)
  {
    uint64_t amount_read = 0;

    if (!is_ok || !backing_file)
    {
      throw std::fstream::failure("Disk image format not OK");
    }

    // Make it easier for us to manipulate our position within 'buffer'
    uint8_t *buffer_uint = reinterpret_cast<uint8_t *>(buffer);

    // Ensure that we don't try to write beyond the length of buffer.
    if (length > buffer_length)
    {
      length = buffer_length;
    }

    // Compute a start block and offset. Note that at the moment we simply ignore "image_block_extra_size".
    uint64_t start_block = start_posn / this->file_header.image_block_size;
    uint64_t block_offset = start_posn % this->file_header.image_block_size;
    uint64_t bytes_this_block = this->file_header.image_block_size - block_offset;
    if (bytes_this_block > length)
    {
      bytes_this_block = length;
    }

    while(amount_read < length)
    {
      read_one_block(start_block, block_offset, bytes_this_block, buffer_uint);

      amount_read += bytes_this_block;

      // Update offsets and lengths for the next block.
      block_offset = 0;
      start_block++;
      buffer_uint += bytes_this_block;
      bytes_this_block = this->file_header.image_block_size;

      if ((amount_read + bytes_this_block) > length)
      {
        bytes_this_block = length - amount_read;
      }
    }
  }

  void vdi_disk::write(const void *buffer, uint64_t start_posn, uint64_t length, uint64_t buffer_length)
  {
    throw std::fstream::failure("Not implemented");
  }

  /// @brief Read a complete or partial block from the disk.
  ///
  /// VDI files are stored on disk in "blocks" that may be out-of-order, which means it is easiest to just attempt to
  /// read one block at a time - which is what this function does.
  ///
  /// @param start_block The logical number of the block to read.
  ///
  /// @param block_offset The offset within the block to begin reading from, in bytes.
  ///
  /// @param bytes_this_block How many bytes to read from this block.
  ///
  /// @param buffer The buffer to read in to. No checks are performed to ensure the buffer is a suitable size.
  ///
  void vdi_disk::read_one_block(uint64_t start_block,
                                uint32_t block_offset,
                                uint32_t bytes_this_block,
                                uint8_t *buffer)
  {
    // Which block within the file represents this block on disk?
    uint32_t block_on_disk_number = this->block_map[start_block];

    // For now, only support already extant blocks
    if ((block_on_disk_number == ~0) || (block_on_disk_number == ((~0) - 1)))
    {
      throw std::fstream::failure("Non-existent block read attempted");
    }

    uint64_t file_offset = (block_on_disk_number * this->file_header.image_block_size) +
                           block_offset +
                           this->file_header.image_data_offset;

    backing_file.seekg(file_offset);
    backing_file.read(reinterpret_cast<char *>(buffer), bytes_this_block);
  }

  uint64_t vdi_disk::get_length()
  {
    return this->file_header.disk_size;
  }
} // namespace virt_disk.
