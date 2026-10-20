#pragma once

#include <string>
#include <optional>
#include <vector>

class ProfileProperty {
public:
    ProfileProperty(std::string name, std::string value, std::string signature);

    std::string name;
    std::string value;
    std::optional<std::string> signature;
};
