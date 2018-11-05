/// @file
/// @brief Declares the main class for accessing virtual disk files.

// Copyright Martin Hughes 2018.

#pragma once

#include <stdint.h>
#include <string>
#include <fstream>

namespace virt_disk
{
  /// @brief The current library version.
  ///
  /// This has the format MMMMmmpp, where:
  /// - MMMM is the major version.
  /// - mm is the minor version.
  /// - pp is the patch level.
  const uint32_t VERSION = 0x00000000;

  /// @brief A class representing a generic virtual disk file.
  ///
  class virt_disk
  {
  protected:
    virt_disk() = default;

  public:
    static virt_disk *create_virtual_disk(std::string &filename);
    virtual ~virt_disk() = default;

    /// @brief Reads from the virtual machine disk into a provided buffer.
    ///
    /// @param buffer The buffer to read in to.
    ///
    /// @param start_posn The number of bytes into the virtual disk to begin reading.
    ///
    /// @param length The number of bytes to read from the disk.
    ///
    /// @param buffer_length The maximum length of buffer, in bytes.
    virtual void read(void *buffer, uint64_t start_posn, uint64_t length, uint64_t buffer_length) = 0;

    /// @brief Get the size of the virtual disk, in bytes
    ///
    /// @return The size of the virtual disk, in bytes.
    virtual uint64_t get_length() = 0;
  };
}

// Try to link to the library automatically on Windows.
#ifdef _WIN32
#pragma comment(lib, "libvirtualdisk.lib")
#endif