import os
import pathlib

_dir_path = os.path.dirname(os.path.realpath(__file__))
path_to_engine = pathlib.Path(_dir_path).parent
path_to_src = path_to_engine / "src"
path_to_shader = path_to_engine / "shaders"
path_to_cmake = path_to_engine / "CMakeLists.txt"
path_to_res = path_to_engine / "res"
path_to_components = path_to_src / "components"
path_to_gameobjects = path_to_src / "gameobjects"
