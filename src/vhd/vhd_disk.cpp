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
    backing_file{filename, std::fstream::binary | std::fstream::in | std::fstream::out}
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
      (footer_copy.format_version != VHD_SUPPORTED_VERSION) ||
      (footer_copy.data_offset != 0xFFFFFFFFFFFFFFFF) ||
      (footer_copy.disk_type != vhd_disk_type::FIXED))
  {
    throw std::fstream::failure("File is wrong format");
  }
  if (footer_copy.features != 2)
  {
    throw std::fstream::failure("No feature flags supported");
  }

  if (footer_copy.current_size > (total_file_length - sizeof(vhd_footer)))
  {
    throw std::fstream::failure("Disk size mismatch");
  }
}

void vhd_disk::read(void *buffer, uint64_t start_posn, uint64_t length, uint64_t buffer_length)
{
  if (length > buffer_length)
  {
    length = buffer_length;
  }

  if ((start_posn + length) > footer_copy.current_size)
  {
    throw std::fstream::failure("Too long");
  }

  backing_file.seekg(start_posn);
  backing_file.read(reinterpret_cast<char *>(buffer), length);
}

uint64_t vhd_disk::get_length()
{
  return footer_copy.current_size;
}

/// @brief Is the given file a VHD format file?
///
/// @param filename The name of the file to check
///
/// @return True if this a file in the VHD format, false otherwise.
bool vhd_disk::is_vhd_format_file(std::string &filename)
{
  std::fstream test_file(filename, std::fstream::binary | std::fstream::in | std::fstream::out);
  uint64_t test_file_length;
  vhd_footer test_footer;
  if (!test_file)
  {
    return false;
  }

  try
  {
    test_file.exceptions(std::fstream::failbit | std::fstream::eofbit | std::fstream::badbit);
    test_file.seekg(0, std::fstream::end);
    test_file_length = test_file.tellg();
    test_file.seekg(test_file_length - sizeof(vhd_footer), std::fstream::beg);
    test_file.read(reinterpret_cast<char *>(&test_footer), sizeof(footer_copy));
  }
  catch (std::fstream::failure &f)
  {
    return false;
  }

  if ((memcmp(&test_footer.cookie, &VHD_COOKIE, sizeof(test_footer.cookie)) != 0) ||
      (test_footer.format_version != VHD_SUPPORTED_VERSION) ||
      (test_footer.data_offset != 0xFFFFFFFFFFFFFFFF) ||
      (test_footer.disk_type != vhd_disk_type::FIXED))
  {
    return false;
  }
  if (test_footer.features != 2)
  {
    return false;
  }
  if (test_footer.current_size > (test_file_length - sizeof(vhd_footer)))
  {
    return false;
  }

  return true;
}