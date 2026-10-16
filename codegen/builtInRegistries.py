import json
import os
import sys

JSON_PATH = sys.argv[1]
OUTPUT_HPP = "include/Registry/BuiltIn.h"

def sanitizeName(name):
    return name.replace(":", "_").replace("/", "_")

def main():
    if not os.path.exists(JSON_PATH):
        print(f"Error: file {JSON_PATH} not found.")
        return

    with open(JSON_PATH, "r", encoding="utf-8") as f:
        data = json.load(f)

    with open(OUTPUT_HPP, "w", encoding="utf-8") as out:
        generatedVars = []
        out.write("// ==========================================================\n")
        out.write("// file automatically generated from registries.json\n")
        out.write("// ==========================================================\n\n")
        out.write("#pragma once\n\n")
        out.write('#include "Registry/Registry.h"\n\n')
        out.write('namespace Registries {\n')

        for registryName, registryContent in data.items():
            entries = registryContent.get("entries", {})
            if not entries:
                continue

            varName = sanitizeName(registryName).upper()
            generatedVars.append(varName)
            sorted_entries = sorted(entries.items(), key=lambda item: item[1].get("protocol_id", 0))

            out.write(f"    inline const Registry {varName} = {{\n")
            out.write(f'        "{registryName}",\n')
            out.write(f"        Registry::Type::BUILT_IN,\n")
            out.write(f"        {{\n")

            for entryName, entryInfo in sorted_entries:
                protoId = entryInfo.get("protocol_id", 0)
                out.write(f'            {{ {protoId}, "{entryName}", {{}} }},\n')
            out.write("        }\n")
            out.write("    };\n\n")

        out.write(f"    inline const Registry* BUILT_IN_REGISTRIES[] = {{\n")
        for var in generatedVars:
            out.write(f"        &{var},\n")

        out.write(f"    }};\n\n")
        out.write(f"    inline const std::size_t BUILT_IN_REGISTRIES_COUNT = {len(generatedVars)};\n")
        out.write(f"}};\n")
    print(f"Successfully generated '{OUTPUT_HPP}'")

if __name__ == "__main__":
    main()
