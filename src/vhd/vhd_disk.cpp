/// @file
///

#include "virtualdisk/virt_disk_vhd.h"

using namespace virt_disk;

/// @brief Constructs a vhd_disk object.
///
/// This object parses Microsoft Virtual Hard Disk (VHD) format disk images.
///
/// @param filename The filename of the disk image to open.
vhd_disk::vhd_disk(std::string &filename) :
    backing_file{filename, std::fstream::binary | std::fstream::in | std::fstream::out},
    data_block_bitmap_bytes{0}
{
  if (!backing_file)
  {
    throw std::fstream::failure("Failed to open backing file");
  }

  backing_file.exceptions(std::fstream::failbit | std::fstream::eofbit | std::fstream::badbit);
  backing_file.seekg(0, std::fstream::end);
  total_file_length = backing_file.tellg();
  backing_file.seekg(total_file_length - sizeof(vhd_footer), std::fstream::beg);
  backing_file.read(reinterpret_cast<char *>(&footer_copy), sizeof(footer_copy));

  if ((memcmp(&footer_copy.cookie, &VHD_COOKIE, sizeof(footer_copy.cookie)) != 0) ||
      (footer_copy.format_version != VHD_SUPPORTED_VERSION))
  {
    throw std::fstream::failure("File is wrong format");
  }

  if ((footer_copy.disk_type != vhd_disk_type::FIXED) &&
      (footer_copy.disk_type != vhd_disk_type::DYNAMIC))
  {
    throw std::fstream::failure("Only FIXED and DYNAMIC disks supported");
  }

  if (footer_copy.features != 2)
  {
    throw std::fstream::failure("No feature flags supported");
  }

  if (footer_copy.disk_type == vhd_disk_type::FIXED)
  {
    if (footer_copy.data_offset != 0xFFFFFFFFFFFFFFFF)
    {
      throw std::fstream::failure("Fixed length file has wrong data offset.");
    }

    if (footer_copy.current_size > (total_file_length - sizeof(vhd_footer)))
    {
      throw std::fstream::failure("Disk size mismatch");
    }
  }

  if (footer_copy.disk_type == vhd_disk_type::DYNAMIC)
  {
    backing_file.seekg(footer_copy.data_offset, std::fstream::beg);
    backing_file.read(reinterpret_cast<char *>(&dynamic_header_copy), sizeof(dynamic_header_copy));

    if (dynamic_header_copy.header_version != VHD_SUPPORTED_VERSION)
    {
      throw std::fstream::failure("Wrong dynamic disk version");
    }

    if ((memcmp(&dynamic_header_copy.cookie, VHD_DYNAMIC_COOKIE, sizeof(VHD_DYNAMIC_COOKIE)) != 0) ||
        (dynamic_header_copy.data_offset != 0xFFFFFFFFFFFFFFFF) ||
        (dynamic_header_copy.table_offset > total_file_length))
    {
      throw std::fstream::failure("Dynamic disk structure not correct");
    }

    data_block_bitmap_bytes = (((dynamic_header_copy.block_size - 1) / 512) + 1) / 8;
    // Round this up to the next 512 byte boundary.
    data_block_bitmap_bytes = (((data_block_bitmap_bytes - 1) / 512) + 1) * 512;

    block_allocation_table = std::unique_ptr<boost::endian::big_uint32_t[]>(
        new boost::endian::big_uint32_t[dynamic_header_copy.max_table_entries]);
    backing_file.seekg(dynamic_header_copy.table_offset, std::fstream::beg);
    backing_file.read(reinterpret_cast<char *>(block_allocation_table.get()),
                      dynamic_header_copy.max_table_entries * 4);
  }
}

void vhd_disk::read(void *buffer, uint64_t start_posn, uint64_t length, uint64_t buffer_length)
{
  if (length > buffer_length)
  {
    length = buffer_length;
  }

  if (this->footer_copy.disk_type == vhd_disk_type::FIXED)
  {
    this->read_block(buffer, start_posn, length);
  }
  else
  {
    uint64_t cur_posn = start_posn;
    uint64_t bytes_to_go = length;
    uint64_t bytes_read = 0;
    uint64_t block_number;
    uint64_t offset_in_block;
    uint64_t bytes_to_read_this_block;
    uint64_t disk_offset;
    uint32_t block_ptr;
    char *write_ptr = reinterpret_cast<char *>(buffer);

    while (bytes_to_go > 0)
    {
      block_number = cur_posn / dynamic_header_copy.block_size;
      offset_in_block = cur_posn % dynamic_header_copy.block_size;

      bytes_to_read_this_block = bytes_to_go;
      if ((offset_in_block + bytes_to_go) > dynamic_header_copy.block_size)
      {
        bytes_to_read_this_block = dynamic_header_copy.block_size - offset_in_block;
      }

      block_ptr = block_allocation_table[block_number];
      if (block_ptr == 0xFFFFFFFF)
      {
        memset(write_ptr, 0, bytes_to_read_this_block);
      }
      else
      {
        disk_offset = (block_ptr * 512) + data_block_bitmap_bytes + offset_in_block;
        backing_file.seekg(disk_offset, std::fstream::beg);
        backing_file.read(write_ptr, bytes_to_read_this_block);
      }

      write_ptr += bytes_to_read_this_block;
      offset_in_block = 0;
      cur_posn += bytes_to_read_this_block;
      bytes_to_go -= bytes_to_read_this_block;
    }
  }
}

uint64_t vhd_disk::get_length()
{
  return footer_copy.current_size;
}

void vhd_disk::read_block(void *buffer, uint64_t start_posn, uint64_t length)
{
  if ((start_posn + length) > footer_copy.current_size)
  {
    throw std::fstream::failure("Too long");
  }

  backing_file.seekg(start_posn);
  backing_file.read(reinterpret_cast<char *>(buffer), length);
}