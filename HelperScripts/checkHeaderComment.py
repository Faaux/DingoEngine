from pathlib import Path

import os

import datetime

dir_path = os.path.dirname(os.path.realpath(__file__))
path_to_engine = Path(dir_path).parent

print("Assumed path to engine: {}".format(str(path_to_engine)))
print()
path_to_src = path_to_engine / "src"
path_to_shader = path_to_engine / "shaders"
path_to_cmake = path_to_engine / "CMakeLists.txt"

pathlist = list(Path(path_to_src).glob('**/*.cpp'))
pathlist = pathlist + list(Path(path_to_src).glob('**/*.h'))
for path in pathlist:
    path_in_str = str(path)
    if "imgui" in path_in_str:
        continue

    with open(path_in_str, 'r') as original:
        data = original.read()
    if data.startswith("/**\n"):
        continue
    with open(path_in_str, 'w') as modified:
        modified.write("/**\n"
                       "*  @file    {}\n"
                       "*  @author  Faaux (github.com/Faaux)\n"
                       "*  @date    {}\n"
                       "*/\n"
                       "\n".format(path.name, datetime.datetime.now().strftime("%d %B %Y")))
        modified.write(data)
        print("Modified: " + path_in_str)
