import os
import platform

sys_name = platform.system()
if sys_name == 'Linux':
  linux_build = True
elif sys_name == 'Windows':
  linux_build = False
else:
  print("Unknown build platform")
  exit(0)

# Setup the help text
Help("""libvirtualdisk build tool.

Targets:
  - Default target: Build the library, but don't install
  - install: Install the library, building if necessary.

Options:
  - install_prefix: Prefix for the installation path. On Linux this is commonly
    /usr. On Windows you should configure something sensible.

Build options are saved for future builds - they do not need to be specified
each time, only if they have changed since the last build.
""")

# Load the install prefix path.
var = Variables([ "variables.cache" ], ARGUMENTS)
var.AddVariables(
      PathVariable("install_prefix",
                   "Path prefix to install library under.",
                   "/usr",
                   PathVariable.PathAccept))
cfg = Environment(variables = var)
var.Save("variables.cache", cfg)
install_prefix = cfg['install_prefix']

# Construct the build environment from the system environment.
env = Environment(ENV = os.environ)

if linux_build:
  env['CC'] = 'clang'
  env['CXX'] = 'clang++'
  env['LINK'] = 'clang++'
  env['CXXFLAGS'] = '-std=c++17'

  env.AppendENVPath('CPATH', '#/src')
else:
  env["CXXFLAGS"] = "/EHac /Od /ZI /MTd /Fdoutput\\libvirtualdisk.pdb /std:c++17 /Zc:__cplusplus /permissive-"
  env.AppendENVPath("INCLUDE", "#/src")

# Main library build script.
main_lib = env.SConscript("SConscript-Library", "env", variant_dir = "output", duplicate = 0)

if not linux_build:
  env.SideEffect("output\\libvirtualdisk.idb", main_lib)
  env.SideEffect("output\\libvirtualdisk.pdb", main_lib)

# Add install target.
lib_dir = os.path.join(install_prefix, "lib")
include_dir = os.path.join(install_prefix, "include", "virtualdisk")
lib_install = env.Install(lib_dir, main_lib)
header_install = env.Install(include_dir, Glob("src/virtualdisk/*.h"))
env.Alias("install", [lib_install, header_install])
