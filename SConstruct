import os

env = Environment(ENV = os.environ)
env.AppendENVPath("INCLUDE", "#/src")
env["CXXFLAGS"] = "/EHac /Od /ZI /MTd /Fdoutput\\libvirtualdisk.pdb /std:c++17 /Zc:__cplusplus /permissive-"
lib = env.SConscript("SConscript-Library", "env", variant_dir = "output", duplicate = 0)

env.SideEffect("output\\libvirtualdisk.idb", lib)
env.SideEffect("output\\libvirtualdisk.pdb", lib)