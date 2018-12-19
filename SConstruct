import os
import platform

dir_prefix = "/usr"

sys_name = platform.system()
if sys_name == 'Linux':
  linux_build = True
elif sys_name == 'Windows':
  linux_build = False
else:
  print("Unknown build platform")
  exit(0)

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

main_lib = env.SConscript("SConscript-Library", "env", variant_dir = "output", duplicate = 0)

if not linux_build:
  env.SideEffect("output\\libvirtualdisk.idb", main_lib)
  env.SideEffect("output\\libvirtualdisk.pdb", main_lib)

if linux_build:
  # Offer the option to install on a Linux machine.
  lib_dir = os.path.join(dir_prefix, "lib")
  include_dir = os.path.join(dir_prefix, "include", "virtualdisk")

  lib_install = env.Install(lib_dir, main_lib)
  header_install = env.Install(include_dir, Glob("src/virtualdisk/*.h"))
  env.Alias("install", [lib_install, header_install])
