# Getting Started with libvirtualdisk

Libvirtualdisk will, eventually, allow you to access and modify the contents of virtual machine hard disk files as
though they were normal files. You might do this to be able to interact directly with filesystem structures, or to see
the layout of files on disk, for example.

## Contents

- Installing
- Including the library in a project

## Installing

To install the library on your machine, follow the instructions in [Installing.md](./Installing.md).

## Including the library in a project

On Windows, the linker will attempt to search for the library automatically. On Linux it will not - you will need to
add it to the build command line yourself.
