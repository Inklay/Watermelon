import json
import os
import sys
import io
from nbtlib import serialize_tag, Compound, String, Int, Byte, Short, Long, Float, Double, List, File

DATA_PATH = sys.argv[1] + "/minecraft/"
OUTPUT_HPP = "include/Registry/Synchronised.h"

REGISTRY_MAPPING = {
    "banner_pattern": "minecraft:banner_pattern",
    "chat_type": "minecraft:chat_type",
    "damage_type": "minecraft:damage_type",
    "dialog": "minecraft:dialog",
    "dimension_type": "minecraft:dimension_type",
    "enchantment": "minecraft:enchantment",
    "instrument": "minecraft:instrument",
    "jukebox_song": "minecraft:jukebox_song",
    "painting_variant": "minecraft:painting_variant",
    "sulfur_cube_archetype": "minecraft:sulfur_cube_archetype",
    "test_environment": "minecraft:test_environment",
    "test_instance": "minecraft:test_instance",
    "timeline": "minecraft:timeline",
    "trim_material": "minecraft:trim_material",
    "trim_pattern": "minecraft:trim_pattern",
    "worldgen/biome": "minecraft:worldgen/biome",
    "world_clock": "minecraft:world_clock",
    "cat_variant": "minecraft:cat_variant",
    "cat_sound_variant": "minecraft:cat_sound_variant",
    "chicken_variant": "minecraft:chicken_variant",
    "chicken_sound_variant": "minecraft:chicken_sound_variant",
    "cow_variant": "minecraft:cow_variant",
    "cow_sound_variant": "minecraft:cow_sound_variant",
    "frog_variant": "minecraft:frog_variant",
    "pig_variant": "minecraft:pig_variant",
    "pig_sound_variant": "minecraft:pig_sound_variant",
    "wolf_variant": "minecraft:wolf_variant",
    "wolf_sound_variant": "minecraft:wolf_sound_variant",
    "zombie_nautilus_variant": "minecraft:zombie_nautilus_variant"
}

registries = {}

def sanitizeName(name):
    return name.replace(":", "_").replace("/", "_")

def jsonToNetworkNbt(jsonData):
    def castToNbt(value):
        if isinstance(value, dict):
            return Compound({k: castToNbt(v) for k, v in value.items()})
        elif isinstance(value, list):
            return List([castToNbt(v) for v in value])
        elif isinstance(value, str):
            return String(value)
        elif isinstance(value, bool):
            return Byte(1 if value else 0)
        elif isinstance(value, int):
            if -2147483648 <= value <= 2147483647:
                return Int(value)
            return Long(value)
        elif isinstance(value, float):
            return Double(value)
        return value

    nbtObj = castToNbt(jsonData)
    nbtFile = File(nbtObj, gzipped=False)
    output = io.BytesIO()
    nbtFile.write(output)
    return list(output.getvalue())

def main():
    if not os.path.exists(DATA_PATH):
        print(f"Error: directory {DATA_PATH} not found.")
        return

    for root, _, files in os.walk(DATA_PATH):
        dirName = os.path.relpath(root, DATA_PATH).replace("\\", "/")
        if dirName in REGISTRY_MAPPING:
            registryName = REGISTRY_MAPPING[dirName]
            if registryName not in registries:
                registries[registryName] = []

            for file in sorted(files):
                if file.endswith(".json"):
                    filePath = os.path.join(root, file)
                    entryName = os.path.splitext(file)[0]
                    textId = f"minecraft:{entryName}"

                    with open(filePath, "r", encoding="utf-8") as f:
                        try:
                            jsonContent = json.load(f)
                            nbtBytes = jsonToNetworkNbt(jsonContent)
                            registries[registryName].append((textId, nbtBytes))
                        except Exception as e:
                            print(f"Error while parsing {filePath}: {e}")
            
    with open(OUTPUT_HPP, "w", encoding="utf-8") as out:
        generatedVars = []
        out.write("// ==========================================================\n")
        out.write("// file automatically generated from registries.json\n")
        out.write("// ==========================================================\n\n")
        out.write("#pragma once\n\n")
        out.write('#include "Registry/Registry.h"\n\n')
        out.write('namespace Registries {\n')

        for registryName, entries in registries.items():
            if not entries:
                continue

            varName = sanitizeName(registryName).upper()
            generatedVars.append(varName)

            out.write(f"    inline const Registry {varName} = {{\n")
            out.write(f'        "{registryName}",\n')
            out.write(f"        Registry::Type::SYNCHRONISED,\n")
            out.write(f"        {{\n")

            for protoId, (textId, nbtBytes) in enumerate(entries):
                bytesStr = ", ".join(f"0x{b:02x}" for b in nbtBytes)
                out.write(f'            {{ {protoId}, "{textId}", {{ {bytesStr} }} }},\n')

            out.write("        }\n")
            out.write("    };\n\n")

        out.write(f"inline const Registry* SYNCHRONIZED_REGISTRIES[] = {{\n")
        for var in generatedVars:
            out.write(f"    &{var},\n")
        out.write(f"    }};\n")
        out.write(f"    inline const std::size_t SYNCHRONIZED_REGISTRIES_COUNT = {len(generatedVars)};\n")
        out.write(f"}};\n")
    print(f"Successfully generated '{OUTPUT_HPP}'")

if __name__ == "__main__":
    main()
