import os

env = Environment(ENV = os.environ)
env.AppendENVPath("INCLUDE", "#/src")
env["CXXFLAGS"] = "/EHac /Od /ZI /MTd /Fdoutput\\libvirtualdisk.pdb"
env.SConscript("SConscript-Library", "env", variant_dir = "output", duplicate = 0)

test_script_env.SideEffect("output\\libvirtualdisk.idb", tests_obj)
test_script_env.SideEffect("output\\libvirtualdisk.pdb", tests_obj)