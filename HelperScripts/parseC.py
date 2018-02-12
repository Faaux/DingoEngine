from pathlib import Path
from multiprocessing import Pool, freeze_support
import os
import datetime
import clang.cindex
from paths import path_to_components, path_to_gameobjects, path_to_cmake, path_to_src

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


class Field:
    def __init__(self, cursor):
        self.name = cursor.spelling
        self.attributes = []
        self.type = cursor.type.spelling
        for c in cursor.get_children():
            if c.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
                self.attributes.append(c.spelling or c.displayname)

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

    def __str__(self) -> str:
        result = "{}".format(self.name)
        if self.attributes:
            result += " -> {}".format(" ,".join(self.attributes))
        return result


class File:
    def __init__(self, filename):
        self.classes = []
        self.candidateClasses = {}
        self.filename: Path = filename
        self.output_filename_h: Path = filename.name.replace(".h", ".generated.h")
        self.output_filename_cpp: Path = filename.name.replace(".h", ".generated.cpp")

    def needs_update(self):
        path = self.filename.parent / "generated"
        path_gen_header = (path / self.output_filename_h)
        path_gen_cpp = (path / self.output_filename_cpp)

        if path_gen_header.is_file() and path_gen_cpp.is_file():
            modify_generated_h = path_gen_header.lstat().st_mtime
            modify_generated_cpp = path_gen_cpp.lstat().st_mtime
            modify_header = self.filename.lstat().st_mtime
            if modify_header < modify_generated_h and modify_header < modify_generated_cpp:
                return False

        return True

    def generate(self):
        # Output to file
        index = clang.cindex.Index.create()
        translation_unit = index.parse(str(self.filename), args)

        self.build_classes(translation_unit.cursor)

        candidates = [c for c in self.candidateClasses.values() if
                      self.filename.samefile(Path(c.cursor.location.file.name)) and len(c.base) == 1]

        for candidate in candidates:
            base = candidate
            while len(base.base) == 1:
                key = base.base[0]
                if key not in self.candidateClasses.keys():
                    break
                base = self.candidateClasses[key]

            if base.cursor.type.spelling == "DG::TypeBase":
                self.classes.append(candidate)

        path = self.filename.parent / "generated"
        if len(self.classes) > 0:
            self.print()
            # Make sure folder exists
            if not os.path.exists(str(path)):
                os.makedirs(str(path))

            with open(str(path / self.output_filename_h), "w") as file:
                output_file(file, False, self.output_filename_h, self)

            with open(str(path / self.output_filename_cpp), "w") as file:
                output_file(file, True, self.output_filename_cpp, self)

    def build_classes(self, cursor):
        for c in cursor.get_children():
            if c.kind == clang.cindex.CursorKind.NAMESPACE:
                self.build_classes(c)
            if c.location.file and path_to_src in Path(c.location.file.name).parents:
                if c.kind == clang.cindex.CursorKind.CLASS_DECL or c.kind == clang.cindex.CursorKind.STRUCT_DECL:
                    a_class = Class(c)
                    self.candidateClasses[c.type.spelling] = a_class

    def print(self):
        print(self.filename)
        indent_size = 4
        for c in self.classes:
            indent = " " * indent_size
            print(indent + "+-- " + c.name)
            indent = indent + "|   "
            for f in c.fields:
                print(indent + "+-- " + f.name)

    def __str__(self) -> str:
        return "{}(File)".format(str(self.filename))


def output_file(file, with_impl, filename, ast_file):
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
        file.write('#include "engine/Serialize.h"\n')
        file.write('#include "{}"\n'.format(filename.replace(".cpp", ".h")))

    file.write(
        '#include "../{}"\n'
        "\n".format(ast_file.filename.name)
    )

    file.write(
        "namespace DG\n"
        "{\n"
    )

    # Output all functions here
    for c in ast_file.classes:
        parsed_class: Class = c
        file.write(
            'void Serialize{}(const {}* item, nlohmann::json& json)'.format(parsed_class.name,
                                                                            parsed_class.name))
        if with_impl:
            file.write(
                '\n'
                '{\n'
            )

            # Find all attributes in files and export them here!
            for f in parsed_class.fields:
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


def generate_for_file(file):
    file.generate()


def main():
    freeze_support()
    pathlist = list(Path(path_to_components).glob('**/*.h'))
    pathlist = pathlist + list(Path(path_to_gameobjects).glob('**/*.h'))
    filelist = [File(p) for p in pathlist
                if not str(p).endswith(".generated.h")
                and not str(p).endswith("Actor.h")
                and not str(p).endswith("BaseComponent.h")]

    filelist = [f for f in filelist if f.needs_update()]
    if len(filelist) > 0:
        if len(filelist) >= 4:
            p = Pool(4)
            p.map(generate_for_file, filelist)
        else:
            for file in filelist:
                file.generate()

        path_to_cmake.touch()


if __name__ == "__main__":
    main()
