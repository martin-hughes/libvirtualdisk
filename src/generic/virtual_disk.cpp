/// @file
/// @brief Implements functionality common to all virtual disk formats.

// Copyright Martin Hughes 2018.

#include "virtualdisk/virtualdisk.h"
#include "virtualdisk/virt_disk_vdi.h"

namespace virt_disk
{
  /// @brief Creates a virtual disk image object from the provided filename.
  ///
  /// In future, this function will look at the image to determine its type, before returning an object of a suitable
  /// type. At the moment, however, it assumes the image is of VDI format and returns an object to handle that.
  ///
  /// @param filename The filename of the disk image to open.
  ///
  /// @return An object that can be used to access that virtual disk.
  virt_disk * virt_disk::create_virtual_disk(std::string &filename)
  {
    return new vdi_disk(filename);
  }
};