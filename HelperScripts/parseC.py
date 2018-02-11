from pathlib import Path
import sys

import os

import datetime

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
                base = self.candidateClasses[base.base[0]]

            if base.cursor.type.spelling == "DG::TypeBase":
                self.classes.append(candidate)

    def build_classes(self, cursor):
        for c in cursor.get_children():
            if c.kind == clang.cindex.CursorKind.NAMESPACE:
                self.build_classes(c)
            if c.location.file and base_path in Path(c.location.file.name).parents:
                if c.kind == clang.cindex.CursorKind.CLASS_DECL or c.kind == clang.cindex.CursorKind.STRUCT_DECL:
                    a_class = Class(c)
                    assert (c.type.spelling not in self.candidateClasses.keys())
                    self.candidateClasses[c.type.spelling] = a_class

    def get_children(self):
        return self.classes

    def __str__(self) -> str:
        return "{}(File)".format(str(self.filename))


def node_children(node):
    return node.get_children()


def print_node(node):
    return str(node)


if len(sys.argv) != 3:
    print("Usage: python parseC.py [header file name] [base include path]")
    sys.exit()

path = Path(sys.argv[1])
base_path = Path(sys.argv[2])

if not path.is_file():
    print("Error: Given path was not a file")
    sys.exit()

assert(base_path in path.parents)

# Parse Into file structure
astFile = File(path)

# Debug output of AST we are considering
print(asciitree.draw_tree(astFile, node_children, print_node))

# Output to file
filename = path.name.replace(".h", ".generated.h")
path = path.parent / "generated"

# Make sure folder exists
if not os.path.exists(str(path)):
    os.makedirs(str(path))

with open(str(path / filename), "w") as file:
    file.write("/**\n"
               "*  @file    {}\n"
               "*  @author  Generated by DingoGenerator (written by Faaux)\n"
               "*  @date    {}\n"
               "*  This file was generated, do not edit!"
               "*/\n"
               "\n".format(filename, datetime.datetime.now().strftime("%d %B %Y")))
    file.write(
        "#pragma once\n"
        '#include "Serialize.h"\n'
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
        if not any([len(f.attributes) > 0 for f in parsedClass.get_children()]):
            print("Skipping class '{}' because no properties were found".format(parsedClass.name))
            continue

        file.write(
            'SDL_FORCE_INLINE nlohmann::json Serialize{}(const {}* item)\n'.format(parsedClass.name, parsedClass.name))
        file.write(
            '{\n'
            '    nlohmann::json jt;\n'
            '    jt["type"] = *item->GetInstanceType();\n'
        )

        # Find all attributes in files and export them here!
        for f in parsedClass.get_children():
            field: Field = f
            assert (field.attributes[0] == "DPROPERTY")
            file.write(
                '    jt["{}"] = Serialize(item->{});\n'.format(field.name, field.name)
            )

        file.write(
            '    return jt;\n'
            '}\n'
        )
    file.write(
        "}  // namespace DG\n"
    )
