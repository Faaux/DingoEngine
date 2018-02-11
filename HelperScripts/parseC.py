from pathlib import Path
from multiprocessing import Pool, freeze_support
import sys

import os

import datetime
import time

import clang.cindex
import asciitree

args = ["-xc++",
        "-D__CODE_GENERATOR__",
        "-std=c++17",
        "-IC:/Projects/DingoEngine/src",
        "-IC:/Projects/DingoEngine/src/misc",
        "-IC:/Projects/DingoEngine/src/graphics",
        "-IC:/Projects/DingoEngine/src/components",
        "-IC:/Projects/DingoEngine/ThirdParty/SDL-mirror/include",
        "-IC:/Projects/DingoEngine/ThirdParty/glm",
        "-IC:/Projects/DingoEngine/ThirdParty/glad/include",
        "-IC:/Projects/DingoEngine/ThirdParty/imgui",
        "-IC:/Projects/DingoEngine/ThirdParty/imguiGizmo",
        "-IC:/Projects/DingoEngine/ThirdParty/tinyGltf",
        "-IC:/Projects/DingoEngine/ThirdParty/freetype-2.9",
        "-IC:/Projects/DingoEngine/ThirdParty/stb",
        "-IC:/Projects/DingoEngine/ThirdParty/physx-3.4/Include",
        "-Ic:/Program Files/LLVM/include"]


def print_whole_ast():
    def node_children(node):
        base_path = Path(r"C:\Projects\DingoEngine\src")
        result = []
        for c in node.get_children():
            if c.location.file and base_path in Path(c.location.file.name).parents:
                result.append(c)

        return result

    def print_node(node):
        text = node.spelling or node.displayname
        kind = str(node.kind)[str(node.kind).index('.') + 1:]
        return '{} {}'.format(kind, text)

    index = clang.cindex.Index.create()
    translation_unit = index.parse(sys.argv[1], args)
    print(asciitree.draw_tree(translation_unit.cursor, node_children, print_node))


class Field:
    def __init__(self, cursor):
        self.name = cursor.spelling
        self.attributes = []
        self.type = cursor.type.spelling
        for c in cursor.get_children():
            if c.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
                self.attributes.append(c.spelling or c.displayname)

    def get_children(self):
        return []

    def __str__(self) -> str:
        result = "{}({})".format(self.name, self.type)

        if self.attributes:
            result += " -> {}".format(" ,".join(self.attributes))
        return result


class Class:
    def __init__(self, cursor):
        self.cursor = cursor
        self.name = cursor.spelling or cursor.displayname
        self.isTypeBase = False
        self.fields = []
        self.attributes = []
        self.base = []

        for c in cursor.get_children():
            if c.kind == clang.cindex.CursorKind.FIELD_DECL:
                f = Field(c)
                self.fields.append(f)
            elif c.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
                self.attributes.append(c.spelling or c.displayname)
            elif c.kind == clang.cindex.CursorKind.CXX_BASE_SPECIFIER:
                self.base.append(c.type.spelling)

        assert (len(self.base) <= 1)

    def get_children(self):
        return self.fields

    def __str__(self) -> str:
        result = "{}".format(self.name)
        if self.attributes:
            result += " -> {}".format(" ,".join(self.attributes))
        return result


class File:
    def __init__(self, filename):
        self.classes = []
        self.candidateClasses = {}
        self.filename = filename

        index = clang.cindex.Index.create()
        translation_unit = index.parse(str(filename), args)

        self.build_classes(translation_unit.cursor)

        # Clean classes, so that only those which inherit from TypeBase and are located in the given file are left

        # 1.) Filter out the classes by file

        candidates = [c for c in self.candidateClasses.values() if
                      filename.samefile(Path(c.cursor.location.file.name)) and len(c.base) == 1]

        for candidate in candidates:
            base = candidate
            while len(base.base) == 1:
                key = base.base[0]
                if key not in self.candidateClasses.keys():
                    break
                base = self.candidateClasses[key]

            if base.cursor.type.spelling == "DG::TypeBase":
                self.classes.append(candidate)

    def build_classes(self, cursor):
        for c in cursor.get_children():
            if c.kind == clang.cindex.CursorKind.NAMESPACE:
                self.build_classes(c)
            if c.location.file and path_to_src in Path(c.location.file.name).parents:
                if c.kind == clang.cindex.CursorKind.CLASS_DECL or c.kind == clang.cindex.CursorKind.STRUCT_DECL:
                    a_class = Class(c)
                    self.candidateClasses[c.type.spelling] = a_class

    def get_children(self):
        return self.classes

    def __str__(self) -> str:
        return "{}(File)".format(str(self.filename))


def node_children(node):
    return node.get_children()


def print_node(node):
    return str(node)


def output_file(file, with_impl, filename, astFile):
    file.write("/**\n"
               "*  @file    {}\n"
               "*  @author  Generated by DingoGenerator (written by Faaux)\n"
               "*  @date    {}\n"
               "*  This file was generated, do not edit!"
               "*/\n"
               "\n".format(filename, datetime.datetime.now().strftime("%d %B %Y")))
    file.write(
        "#pragma once\n"
    )

    if with_impl:
        file.write('#include "Serialize.h"\n')
        file.write('#include "{}"\n'.format(filename.replace(".cpp", ".h")))

    file.write(
        '#include "../{}"\n'
        "\n".format(astFile.filename.name)
    )

    file.write(
        "namespace DG\n"
        "{\n"
    )

    # Output all functions here
    for c in astFile.get_children():
        parsedClass: Class = c
        file.write(
            'void Serialize{}(const {}* item, nlohmann::json& json)'.format(parsedClass.name,
                                                                            parsedClass.name))
        if with_impl:
            file.write(
                '\n'
                '{\n'
            )

            # Find all attributes in files and export them here!
            for f in parsedClass.get_children():
                field: Field = f
                if field.attributes:
                    if field.attributes[0] == "DPROPERTY":
                        file.write(
                            '    json["{}"] = Serialize(item->{});\n'.format(field.name, field.name)
                        )

            file.write(
                '}\n'
            )
        else:
            file.write(";\n")
    file.write(
        "}  // namespace DG\n"
    )


def generate_for_path(path):
    if not path.is_file():
        print("Error: Given path was not a file: {}".format(str(path)))
        return

    # Parse Into file structure
    astFile = File(path)

    # Output to file
    filename_h = path.name.replace(".h", ".generated.h")
    filename_cpp = path.name.replace(".h", ".generated.cpp")
    path = path.parent / "generated"

    if len(astFile.get_children()) > 0:
        # Debug output of AST we are considering
        print(asciitree.draw_tree(astFile, node_children, print_node))
        # Make sure folder exists
        if not os.path.exists(str(path)):
            os.makedirs(str(path))

        with open(str(path / filename_h), "w") as file:
            output_file(file, False, filename_h, astFile)

        with open(str(path / filename_cpp), "w") as file:
            output_file(file, True, filename_cpp, astFile)


dir_path = os.path.dirname(os.path.realpath(__file__))
path_to_engine = Path(dir_path).parent
path_to_src = path_to_engine / "src"

if __name__ == "__main__":
    print("Assumed path to engine: {}".format(str(path_to_engine)))
    print()

    path_to_shader = path_to_engine / "shaders"
    path_to_cmake = path_to_engine / "CMakeLists.txt"
    path_to_components = path_to_src / "components"
    path_to_gameobjects = path_to_src / "gameobjects"
    freeze_support()
    pathlist = list(Path(path_to_components).glob('**/*.h'))
    pathlist = pathlist + list(Path(path_to_gameobjects).glob('**/*.h'))
    pathlist = [p for p in pathlist
                if not str(p).endswith(".generated.h")
                and not str(p).endswith("Actor.h")
                and not str(p).endswith("BaseComponent.h")]

    p = Pool(4)
    p.map(generate_for_path, pathlist)
    path_to_cmake.touch()
