from pathlib import Path
from collections import defaultdict

SRC_DIR = Path("src").resolve()
files_by_group = defaultdict(list)

for file in SRC_DIR.rglob("*"):
    if file.suffix in [".cpp", ".h", ".c"]:
        rel_path = file.relative_to(SRC_DIR)
        group = str(rel_path.parent).replace("\\", "/")
        files_by_group[group].append(file)

of = open("cmake_files.cmake", "w")

all_vars = []
dont_compile = [
    "physics/ode/array.cpp",
    "physics/ode/array.h",
    "physics/ode/collision_trimesh.cpp",
    "physics/ode/collision_trimesh_ccylinder.cpp",
    "physics/ode/collision_trimesh_distance.cpp",
    "physics/ode/collision_trimesh_ray.cpp",
    "physics/ode/collision_trimesh_sphere.cpp",
    "physics/ode/collision_trimesh_trimesh.cpp",
    "physics/ode/lcp.cpp",
    "physics/ode/lcp.h",
    "physics/ode/mat.cpp",
    "physics/ode/mat.h",
    "physics/ode/memory.cpp",
    "physics/ode/obstack.cpp",
    "physics/ode/odeext.h",
    "physics/ode/odemisc.cpp",
    "physics/ode/stack.cpp",
    "physics/ode/step.cpp",
    "physics/ode/step.h",
    "physics/ode/stepfast.cpp",
    "physics/ode/stepfast.h",
    "physics/ode/testing.cpp",
    "physics/ode/testing.h",
    "script/scr_yacc.cpp",
    "script/scr_compiler.cpp",
    "binklib/dx9rad3d.cpp",
]

for group, files in sorted(files_by_group.items()):
    var_name = group.upper().replace("/", "_").replace("-", "_")
    if var_name == ".":
        var_name = "SRCROOT"
    print(f"set({var_name}",file=of)
    for f in files:
        fp = f.as_posix()
        rel = fp.find("src/") + 4
        file_rel = fp[rel:]
        if file_rel in dont_compile:
            continue
        print(f"    \"${{SRC_DIR}}/{file_rel}\"",file=of)
    print(")",file=of)
    print(f"source_group(\"{group}\" FILES ${{{var_name}}})\n",file=of)
    all_vars.append(var_name)
print(f"add_executable( ${{BIN_NAME}} {' '.join(f'${{{v}}}' for v in all_vars)})", file=of)