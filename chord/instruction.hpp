#ifndef __CHORD_INSTRUCTION_HPP__
#define __CHORD_INSTRUCTION_HPP__

#include <string>
#include <optional>

namespace chord
{
class Instruction
{
  public:
    enum Type
    {
        Join, // join dst_ip:port
        Get,  // get filename
        Put,  // put filepath
        Quit, // quit
    };

    static std::optional<Instruction> parse(const std::string &ins)
    {
        std::size_t pos = ins.find(' ');

        Type type;
        auto type_str = ins.substr(0, pos);
        auto value = ins.substr(pos + 1);

        if (type_str == "join")
        {
            type = Join;
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
        else
        {
            return {};
        }

        return Instruction(type, std::move(value));
    }

    Type type() const { return type_; }
    const std::string &value() { return value_; }

  private:
    Instruction(Type type, std::string value)
      : type_(type), value_(std::move(value))
    {
        // ...
    }

  private:
    Type type_;
    std::string value_;
};
} // namespace chord

#endif
