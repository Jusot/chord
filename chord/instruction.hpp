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
        SelfBoot, // self-boot
        Print, // print
    };

    static std::optional<Instruction>
    parse(const std::string &ins);

    Type type() const;
    const std::string &value() const;

  private:
    Instruction(Type type, std::string value);

  private:
    Type type_;
    std::string value_;
};
} // namespace chord

#endif
