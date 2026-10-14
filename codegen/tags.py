import json
import os
import sys

DATA_PATH = sys.argv[1] + "/minecraft/tags"
JSON_PATH = sys.argv[2]
OUTPUT_HPP = "include/Tag/Tags.h"

REGISTRY_MAPPING = {
    "banner_pattern": "minecraft:banner_pattern",
    "block": "minecraft:block",
    "damage_type": "minecraft:damage_type",
    "dialog": "minecraft:dialog",
    "enchantment": "minecraft:enchantment",
    "entity_type": "minecraft:entity_type",
    "fluid": "minecraft:fluid",
    "game_event": "minecraft:game_event",
    "instrument": "minecraft:instrument",
    "item": "minecraft:item",
    "painting_variant": "minecraft:painting_variant",
    "point_of_interest_type": "minecraft:point_of_interest_type",
    "potion": "minecraft:potion",
    "timeline": "minecraft:timeline",
    #"villager_trade/armorer": "minecraft:villager_trade",
    #"villager_trade/butcher": "minecraft:villager_trade",
    #"villager_trade/cartographer": "minecraft:villager_trade",
    #"villager_trade/cleric": "minecraft:villager_trade",
    #"villager_trade/common_smith": "minecraft:villager_trade",
    #"villager_trade/farmer": "minecraft:villager_trade",
    #"villager_trade/fisherman": "minecraft:villager_trade",
    #"villager_trade/fletcher": "minecraft:villager_trade",
    #"villager_trade/leatherworker": "minecraft:villager_trade",
    #"villager_trade/librarian": "minecraft:villager_trade",
    #"villager_trade/mason": "minecraft:villager_trade",
    #"villager_trade/shepherd": "minecraft:villager_trade",
    #"villager_trade/toolsmith": "minecraft:villager_trade",
    #"villager_trade/wandering_trader": "minecraft:villager_trade",
    #"villager_trade/weaponsmith": "minecraft:villager_trade",
    "worldgen/biome": "minecraft:worldgen/biome",
    #"worldgen/feature": "minecraft:worldgen/feature",
    #"worldgen/flat_level_generator_preset": "minecraft:worldgen/flat_level_generator_preset",
    #"worldgen/structure": "minecraft:worldgen/structure",
    #"worldgen/world_preset": "minecraft:worldgen/world_preset",
}

rawTagsContent = {}
resolvedTags = {}

def sanitizeName(name):
    return name.replace(":", "_").replace("/", "_")

def getMapping():
    with open(JSON_PATH, "r", encoding="utf-8") as f:
        dataBuiltIn = json.load(f)

    with open("protocol_mapping.json", "r", encoding="utf-8") as f:
        dataMapped = json.load(f)

    for registryName, registryContent in dataBuiltIn.items():
        if registryName not in dataMapped:
            dataMapped[registryName] = {"entries": {}}
            builtinEntries = registryContent.get("entries", {})
    
            for entryName, entryInfo in builtinEntries.items():
                dataMapped[registryName]["entries"][entryName] = {
                    "protocol_id": entryInfo.get("protocol_id", 0)
                }
    return dataMapped

def findRegistryPrefix(dirName):
    parts = dirName.split("/")
    for i in range(len(parts), 0, -1):
        prefix = "/".join(parts[:i])
        if prefix in REGISTRY_MAPPING:
            return prefix
    return None

def resolveTagValues(registryName, currentTagId, dataMapped, visited = None):
    if visited is None:
        visited = set()
        
    if currentTagId in visited:
        return []
    visited.add(currentTagId)

    tagRegistry = rawTagsContent.get(registryName, {})
    rawValues = tagRegistry.get(currentTagId, [])
    
    finalIds = []
    
    for val in rawValues:
        if isinstance(val, dict):
            val = val.get("id", "")
            
        if not val:
            continue
            
        if str(val).startswith("#"):
            subTagId = val[1:]
            finalIds.extend(resolveTagValues(registryName, subTagId, dataMapped, visited))
        else:
            registryEntries = dataMapped.get(registryName, {}).get("entries", {})
            if val in registryEntries:
                protoId = registryEntries[val]["protocol_id"]
                if (val, protoId) not in finalIds:
                    finalIds.append((val, protoId))
                    
    return finalIds

def main():
    if not os.path.exists(DATA_PATH):
        print(f"Error: directory {DATA_PATH} not found.")
        return

    dataMapped = getMapping()

    for root, _, files in os.walk(DATA_PATH):
        dirName = os.path.relpath(root, DATA_PATH).replace("\\", "/")
        if dirName == ".":
            continue

        prefix = findRegistryPrefix(dirName)
        if prefix is None:
            continue

        registryName = REGISTRY_MAPPING[prefix]
        subPath = dirName[len(prefix):].lstrip("/")

        if registryName not in rawTagsContent:
            rawTagsContent[registryName] = {}
            resolvedTags[registryName] = []

        for file in sorted(files):
            if not file.endswith(".json"):
                continue

            baseName = os.path.splitext(file)[0]
            tagName = f"{subPath}/{baseName}" if subPath else baseName
            tagTextId = f"minecraft:{tagName}"

            with open(os.path.join(root, file), "r", encoding="utf-8") as f:
                jsonContent = json.load(f)
                rawTagsContent[registryName][tagTextId] = jsonContent.get("values", [])

    for registryName, tagDict in rawTagsContent.items():
        for tagTextId in tagDict.keys():
            flattenedProtoIds = resolveTagValues(registryName, tagTextId, dataMapped)
            flattenedProtoIds.sort(key=lambda x: x[1])
            resolvedTags[registryName].append((tagTextId, flattenedProtoIds))

    with open(OUTPUT_HPP, "w", encoding="utf-8") as out:
        generatedVars = []
        out.write("// ==========================================================\n")
        out.write("// file automatically generated from tags directory\n")
        out.write("// ==========================================================\n\n")
        out.write("#pragma once\n\n")
        out.write('#include "Tag/RegistryTag.h"\n\n')
        out.write('namespace Tags {\n')

        for registryName, tagList in resolvedTags.items():
            if not tagList:
                continue

            varName = sanitizeName(registryName).upper()
            generatedVars.append(varName)

            out.write(f"    inline const RegistryTag {varName} = {{\n")
            out.write(f'        "{registryName}",\n')
            out.write(f"        {{\n")

            for tagTextId, entries in tagList:
                out.write(f"            {{\n")
                out.write(f'                "{tagTextId}",\n')
                out.write(f"                {{\n")
                for name, protoId in entries:
                    out.write(f'                    {protoId},\n')
                out.write(f"                }}\n")
                out.write(f"            }},\n")
            out.write(f"        }}\n")
            out.write(f"    }};\n\n")

        out.write(f"    inline const RegistryTag* REGISTRY_TAGS[] = {{\n")
        for var in generatedVars:
            out.write(f"        &{var},\n")
        out.write(f"    }};\n")
        out.write(f"    inline const std::size_t REGISTRY_TAGS_COUNT = {len(generatedVars)};\n")
        out.write(f"}};\n")
    print(f"Successfully generated '{OUTPUT_HPP}'")

if __name__ == "__main__":
    main()
