import pathlib
import os

dir_path = os.path.dirname(os.path.realpath(__file__))

path_to_engine = pathlib.Path(dir_path).parent

print("Assumed path to engine: {}".format(str(path_to_engine)))
print()
path_to_src = path_to_engine / "src"
path_to_shader = path_to_engine / "shaders"
path_to_cmake = path_to_engine / "CMakeLists.txt"


def get_input_as_int():
    succeeded = False
    result = -1
    while not succeeded:
        try:
            result = int(input("Please choose: \n"))
            succeeded = True
        except Exception:
            pass
    return result


def output_header(name):
    with open(str(path_to_src / (name + ".h")), "w") as file:
        file.write(
            "#pragma once\n"
            '#include "DG_Include.h"\n'
            "\n"
            "namespace DG\n"
            "{\n"
            "}  // namespace DG\n"
        )


def output_source(name):
    with open(str(path_to_src / (name + ".cpp")), "w") as file:
        file.write('#include "{}"\n'.format((name + ".h")))
        file.write("\n"
                   "namespace DG\n"
                   "{\n"
                   "}  // namespace DG\n"
                   )


def output_shaders(name):
    pass


try:
    while True:
        print("Header and Source Files")
        print("    1) Header File")
        print("    2) Source File")
        print("    3) Header and Source")
        print()

        print("Shaders")
        print("    4) Shaders (not implemented)")
        print()

        print("Miscellaneous")
        print("    0) Touch CMakeLists.txt")
        print("")

        choice = get_input_as_int()
        if choice != 0:
            filename = input("Please input the name (without extension):\n")
        if choice == 1 or choice == 3:
            output_header(filename)
        if choice == 2 or choice == 3:
            output_source(filename)
        if choice == 4:
            output_shaders(filename)

        # Touch CMake to make it regenerate
        path_to_cmake.touch()

        print("----------Success---------")
        print("")
        print("")
except KeyboardInterrupt:
    pass
