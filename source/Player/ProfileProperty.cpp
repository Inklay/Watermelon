#include "Player/ProfileProperty.h"

namespace Player {
    ProfileProperty::ProfileProperty(std::string name, std::string value, std::optional<std::string> signature) :
        name(std::move(name)),
        value(std::move(value)),
        signature(std::move(signature))
    {}
}
