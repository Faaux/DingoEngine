import subprocess
from paths import path_to_src

pathlist = list(path_to_src.glob('**/*.cpp'))
pathlist = pathlist + list(path_to_src.glob('**/*.h'))

arguments = [r"c:\Program Files\LLVM\bin\clang-format.exe", "-i", "-style=file"]
arguments.extend([str(p) for p in pathlist])

subprocess.Popen(arguments)
