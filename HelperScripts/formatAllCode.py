import os
from pathlib import Path
import subprocess

dir_path = os.path.dirname(os.path.realpath(__file__))
path_to_engine = Path(dir_path).parent
path_to_src = path_to_engine / "src"
path_to_shader = path_to_engine / "shaders"
path_to_cmake = path_to_engine / "CMakeLists.txt"

pathlist = list(Path(path_to_src).glob('**/*.cpp'))
pathlist = pathlist + list(Path(path_to_src).glob('**/*.h'))

arguments = [r"c:\Program Files\LLVM\bin\clang-format.exe", "-i", "-style=file"]
arguments.extend([str(p) for p in pathlist])

subprocess.Popen(arguments)
