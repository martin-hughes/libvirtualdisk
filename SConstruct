import os

env = Environment(ENV = os.environ)
env.AppendENVPath("INCLUDE", "#/src")
env["CXXFLAGS"] = "/EHac /Od /ZI /MTd"
env.SConscript("SConscript-Library", "env", variant_dir = "output", duplicate = 0)