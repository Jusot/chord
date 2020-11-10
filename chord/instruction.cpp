#include "instruction.hpp"

namespace chord
{
std::optional<Instruction> Instruction::parse(const std::string &ins)
{
    std::size_t pos = ins.find(' ');

    Type type;
    auto type_str = ins.substr(0, pos);
    auto value = ins.substr(pos + 1);

    if (type_str == "join")
    {
        type = Join;
        /**
         * TODO: catch join-ins with wrong value
        */
    }
    else if (type_str == "get")
    {
        type = Get;
    }
    else if (type_str == "put")
    {
        type = Put;
    }
    else if (type_str == "quit")
    {
        type = Quit;
    }
    else if (type_str == "self-boot")
    {
        type = SelfBoot;
    }
    else if (type_str == "print")
    {
        type = Print;
    }
    else
    {
        return {};
    }

    return Instruction(type, std::move(value));
}

Instruction::Type Instruction::type() const
{
    return type_;
}

const std::string &Instruction::value() const
{
    return value_;
}

Instruction::Instruction(Type type, std::string value)
  : type_(type), value_(std::move(value))
{
    // ...
}
} // namespace chord
