#include "Player/ProfileProperty.h"

ProfileProperty::ProfileProperty(std::string name, std::string value, std::string signature) :
    name(std::move(name)),
    value(std::move(value)),
    signature(std::move(signature))
{
}
