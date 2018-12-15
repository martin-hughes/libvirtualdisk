/// @file
/// @brief Implements functionality common to all virtual disk formats.

// Copyright Martin Hughes 2018.

#include "virtualdisk/virtualdisk.h"
#include "virtualdisk/virt_disk_vdi.h"
#include "virtualdisk/virt_disk_vhd.h"

#include <functional>

namespace
{
  typedef std::function<virt_disk::virt_disk *(std::string)> disk_constructor;

  struct format_info
  {
    disk_constructor constructor_fn;
  };

  format_info known_types[] =
    {
      {
        [](std::string f) { return dynamic_cast<virt_disk::virt_disk *>(new virt_disk::vdi_disk(f)); }
      },
      {
        [](std::string f) { return dynamic_cast<virt_disk::virt_disk *>(new virt_disk::vhd_disk(f)); }
      },
    };

  const uint32_t NUM_FORMATS = sizeof(known_types) / sizeof(format_info);
}

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
    for (int i = 0; i < NUM_FORMATS; i++)
    {
      try
      {
        virt_disk *disk = known_types[i].constructor_fn(filename);
        if (disk != nullptr)
        {
          return disk;
        }
      }
      catch (std::fstream::failure &fault)
      {
        continue;
      }
    }

    throw std::fstream::failure("No valid format");
  }
};