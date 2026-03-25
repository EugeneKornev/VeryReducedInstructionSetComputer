#include <cassert>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace Emulator {
enum Reg { R0, R1, R2, R3 };

struct EmulatorState {
  static const size_t regs_size = 4;
  static const size_t mem_size = 1024;

  std::vector<int> _registers{std::vector<int>(regs_size)};
  std::vector<int> _mem{std::vector<int>(mem_size)};

  size_t _pc = 0; // program counter register

  //EmulatorState() {}

  //~EmulatorState() {}
};

// TODO: implement all instructions listed in ISA. This class should be base
// class for all insturctions
class Instruction {
public:
  virtual void eval(EmulatorState &emul) = 0;

  virtual ~Instruction() {};

  void increment(EmulatorState &emul) { emul._pc++; }
};

class RegisterInstruction : public Instruction {
protected:
  unsigned char destRegisterIndex;

public:
  virtual void eval(EmulatorState &st) override {}

  RegisterInstruction(unsigned char idx) : destRegisterIndex(idx) {}
};

class ArithmeticInstructionReg : public RegisterInstruction {
protected:
  unsigned char srcRegisterIndex;

public:
  virtual void eval(EmulatorState &st) override {}

  ArithmeticInstructionReg(unsigned char src_idx, unsigned char dest_idx)
      : RegisterInstruction(dest_idx), srcRegisterIndex(src_idx) {}
};

class AddArithmeticInstructionReg : public ArithmeticInstructionReg {
public:
  AddArithmeticInstructionReg(unsigned char src_idx, unsigned char dest_idx)
      : ArithmeticInstructionReg(src_idx, dest_idx) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRegisterIndex && destRegisterIndex <= 3 &&
           0 <= srcRegisterIndex && srcRegisterIndex <= 3);
    st._registers[destRegisterIndex] += st._registers[srcRegisterIndex];
    this->increment(st);
  }
};

class SubArithmeticInstructionReg : public ArithmeticInstructionReg {
public:
  SubArithmeticInstructionReg(unsigned char src_idx, unsigned char dest_idx)
      : ArithmeticInstructionReg(src_idx, dest_idx) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRegisterIndex && destRegisterIndex <= 3 &&
           0 <= srcRegisterIndex && srcRegisterIndex <= 3);
    st._registers[destRegisterIndex] -= st._registers[srcRegisterIndex];
    this->increment(st);
  }
};

class MulArithmeticInstructionReg : public ArithmeticInstructionReg {
public:
  MulArithmeticInstructionReg(unsigned char src_idx, unsigned char dest_idx)
      : ArithmeticInstructionReg(src_idx, dest_idx) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRegisterIndex && destRegisterIndex <= 3 &&
           0 <= srcRegisterIndex && srcRegisterIndex <= 3);
    st._registers[destRegisterIndex] *= st._registers[srcRegisterIndex];
    this->increment(st);
  }
};

class DivArithmeticInstructionReg : public ArithmeticInstructionReg {
public:
  DivArithmeticInstructionReg(unsigned char src_idx, unsigned char dest_idx)
      : ArithmeticInstructionReg(src_idx, dest_idx) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRegisterIndex && destRegisterIndex <= 3 &&
           0 <= srcRegisterIndex && srcRegisterIndex <= 3);
    st._registers[destRegisterIndex] /= st._registers[srcRegisterIndex];
    this->increment(st);
  }
};

class ArithmeticInstructionImm : public RegisterInstruction {
protected:
  int imm;

public:
  ArithmeticInstructionImm(int immediate, unsigned char idx)
      : RegisterInstruction(idx), imm(immediate) {}

  virtual void eval(EmulatorState &st) override {}
};

class AddArithmeticInstructionImm : public ArithmeticInstructionImm {
public:
  AddArithmeticInstructionImm(unsigned short address, unsigned char idx)
      : ArithmeticInstructionImm(address, idx) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRegisterIndex && destRegisterIndex <= 3);
    st._registers[destRegisterIndex] += imm;
    this->increment(st);
  }
};

class SubArithmeticInstructionImm : public ArithmeticInstructionImm {
public:
  SubArithmeticInstructionImm(unsigned short address, unsigned char idx)
      : ArithmeticInstructionImm(address, idx) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRegisterIndex && destRegisterIndex <= 3);
    st._registers[destRegisterIndex] -= imm;
    this->increment(st);
  }
};

class MulArithmeticInstructionImm : public ArithmeticInstructionImm {
public:
  MulArithmeticInstructionImm(unsigned short address, unsigned char idx)
      : ArithmeticInstructionImm(address, idx) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRegisterIndex && destRegisterIndex <= 3);
    st._registers[destRegisterIndex] *= imm;
    this->increment(st);
  }
};

class DivArithmeticInstructionImm : public ArithmeticInstructionImm {
public:
  DivArithmeticInstructionImm(unsigned short address, unsigned char idx)
      : ArithmeticInstructionImm(address, idx) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRegisterIndex && destRegisterIndex <= 3);
    st._registers[destRegisterIndex] /= imm;
    this->increment(st);
  }
};

class MovInstructionReg : public RegisterInstruction {
protected:
  unsigned char srcRegisterIndex;

public:
  MovInstructionReg(unsigned char src_idx, unsigned char dest_idx)
      : RegisterInstruction(dest_idx), srcRegisterIndex(src_idx) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRegisterIndex && destRegisterIndex <= 3 &&
           0 <= srcRegisterIndex && srcRegisterIndex <= 3);
    st._registers[destRegisterIndex] = st._registers[srcRegisterIndex];
    this->increment(st);
  }
};

class MovInstructionImm : public RegisterInstruction {
protected:
  int imm;

public:
  MovInstructionImm(int immediate, unsigned char dest_idx)
      : RegisterInstruction(dest_idx), imm(immediate) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRegisterIndex && destRegisterIndex <= 3);
    st._registers[destRegisterIndex] = imm;
    this->increment(st);
  }
};

class LoadInstruction : public RegisterInstruction {
protected:
  unsigned short srcRAMAddress;

public:
  LoadInstruction(unsigned char dest_idx, unsigned short address)
      : RegisterInstruction(dest_idx), srcRAMAddress(address) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRegisterIndex && destRegisterIndex <= 3 &&
           0 <= srcRAMAddress && srcRAMAddress <= 1023);
    st._registers[destRegisterIndex] = st._mem[srcRAMAddress];
    this->increment(st);
  }
};

class StoreInstruction : public Instruction {
protected:
  unsigned short destRAMAddress;
  unsigned char srcRegisterIndex;

public:
  StoreInstruction(unsigned short address, unsigned char idx)
      : destRAMAddress(address), srcRegisterIndex(idx) {}

  void eval(EmulatorState &st) override {
    assert(0 <= destRAMAddress && destRAMAddress <= 1023 &&
           0 <= srcRegisterIndex && srcRegisterIndex <= 3);
    st._mem[destRAMAddress] = st._registers[srcRegisterIndex];
    this->increment(st);
  }
};

class JmpInstruction : public Instruction {
protected:
  unsigned int address;

public:
  JmpInstruction(unsigned int adr) : address(adr) {}

  //~JmpInstruction() {}

  virtual void eval(EmulatorState &st) override { st._pc = address; }
};

class JmpCondInstruction : public JmpInstruction {
public:
  JmpCondInstruction(unsigned int address) : JmpInstruction(address) {}

  //~JmpCondInstruction() {}

  void eval(EmulatorState &st) override {
    if (st._registers[0] != 0) {
      this->increment(st);
    } else {
      st._pc = address;
    }
  }
};

Instruction *buildMoveInstr(std::smatch match) {
  if (match[3].matched) {
    return new MovInstructionReg(std::stoi(match[3]), std::stoi(match[1]));
  } else {
    return new MovInstructionImm(std::stoi(match[4]), std::stoi(match[1]));
  }
}

Instruction *buildArithmeticInstr(std::smatch match) {
  if (match[2].matched) {
    if (match[8].matched) {
      return new AddArithmeticInstructionReg(std::stoi(match[8]),
                                             std::stoi(match[6]));
    } else {
      return new AddArithmeticInstructionImm(std::stoi(match[9]),
                                             std::stoi(match[6]));
    }
  } else if (match[3].matched) {
    if (match[8].matched) {
      return new SubArithmeticInstructionReg(std::stoi(match[8]),
                                             std::stoi(match[6]));
    } else {
      return new SubArithmeticInstructionImm(std::stoi(match[9]),
                                             std::stoi(match[6]));
    }
  } else if (match[4].matched) {
    if (match[8].matched) {
      return new MulArithmeticInstructionReg(std::stoi(match[8]),
                                             std::stoi(match[6]));
    } else {
      return new MulArithmeticInstructionImm(std::stoi(match[9]),
                                             std::stoi(match[6]));
    }
  } else {
    if (match[8].matched) {
      return new DivArithmeticInstructionReg(std::stoi(match[8]),
                                             std::stoi(match[6]));
    } else {
      return new DivArithmeticInstructionImm(std::stoi(match[9]),
                                             std::stoi(match[6]));
    }
  }
}

Instruction *builldMemInstr(std::smatch match) {
  if (match[2].matched) {
    return new LoadInstruction(std::stoi(match[4]), std::stoi(match[5]));
  } else {
    return new StoreInstruction(std::stoi(match[5]), std::stoi(match[4]));
  }
}

Instruction *buildJmpInstr(std::smatch match) {
  if (match[2].matched) {
    return new JmpInstruction(std::stoi(match[4]));
  } else {
    return new JmpCondInstruction(std::stoi(match[4]));
  }
}

const std::regex regexes[] = {
    std::regex("Mov R(\\d) (R(\\d)|(\\d+))"),
    std::regex("((Add)|(Sub)|(Mul)|(Div)) R(\\d+) (R(\\d)|(\\d+))"),
    std::regex("((Load)|(Store)) R(\\d) (\\d+)"),
    std::regex("((Jmp)|(Jmpz)) (\\d+)")};

std::optional<std::string> trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t\n");
  size_t end = str.find_last_not_of(" \t\n");

  if (start == std::string::npos) {
    return std::nullopt;
  }

  return std::optional<std::string>(str.substr(start, end - start + 1));
}

std::vector<std::string> split(const std::string &s,
                               const std::string &delimiter) {
  std::vector<std::string> tokens;
  size_t last = 0;
  size_t next = 0;
  while ((next = s.find(delimiter, last)) != std::string::npos) {
    tokens.push_back(s.substr(last, next - last));
    last = next + 1;
  }
  tokens.push_back(s.substr(last));
  return tokens;
}

/* This function accepts the program written in the vrisc assembly
 * If the input program is correct, returns sequence of insturction,
 * corresponding to the input program. If the input text is incorrect, the
 * behaviour is undefined
 */
std::vector<Instruction *> parse(const std::string &program) {
  // TODO: implement it!
  // feel free to change signature of this function
  std::vector<Instruction *> compiled_program;
  std::vector<std::string> splitted = split(program, "\n");
  std::vector<std::string> trimmed;
  for (auto i : splitted) {
    std::optional<std::string> t = trim(i);
    if (t.has_value()) {
      trimmed.push_back(t.value());
    }
  }
  std::smatch sm;
  for (auto j : trimmed) {
    unsigned char regexType = 4;
    for (int k = 0; k < 4; k++) {
      if (std::regex_match(j, sm, regexes[k])) {
        regexType = k;
        break;
      }
    }
    switch (regexType) {
    case 0:
      compiled_program.push_back(buildMoveInstr(sm));
      break;
    case 1:
      compiled_program.push_back(buildArithmeticInstr(sm));
      break;
    case 2:
      compiled_program.push_back(builldMemInstr(sm));
      break;
    case 3:
      compiled_program.push_back(buildJmpInstr(sm));
      break;
    case 4:
      throw std::invalid_argument("Invalid instruction parsed");
      break;
    }
  }
  return compiled_program;
}

/* Emulate receive a program, written in the vrisc assembly,
 * in case of the correct program, emulate returns R0 value at the end of the
 * emulation. If the program is incorrect, that is, either its text is not vrisc
 * assembly language or it contains UB(endless cycles), the behaviour of emulate
 * if also undefined. Handle these cases in any way.
 */
int emulate(const std::string &program_text) {
  // Feel free to change code of this function
  std::vector<Instruction *> program = parse(program_text);
  EmulatorState state;
  while (state._pc < program.size()) {
    program[state._pc]->eval(state);
  }
  for (size_t i = 0; i < program.size(); i++) {
    delete program[i];
  }
  return state._registers[R0];
}
} // namespace Emulator

// Simple helper for file as single line. Feel free to change it or delete it
// completely
std::optional<std::string> readStringFromFile(const std::string &filename) {
  std::ifstream file{filename};

  if (!file) {
    return std::nullopt;
  }

  std::stringstream buf;
  buf << file.rdbuf();

  return buf.str();
}

TEST(Emulator, MovToRegister) {
  std::string program = R"(
      Mov R0 12
  )";
  int result = Emulator::emulate(program);
  ASSERT_EQ(result, 12);
}

TEST(Emulator, DefaultFactorial) {
  std::string program = R"(
    Mov R0 5
    Mov R1 1  
    Jmpz 6
                      
    Mul R1 R0
    Sub R0 1  
    Jmp 2       

    Mov R0 R1
  )";
  int result = Emulator::emulate(program);
  ASSERT_EQ(result, 120);
}

TEST(Emulator, Fibonacci) {
  std::string program = R"(
    Mov R0 18
    Mov R1 0
    Mov R2 1

    Mov R3 R2
    Add R3 R1
    Mov R1 R2
    Mov R2 R3
    Sub R0 1

    Jmpz 10
    Jmp 3

    Mov R0 R2  
  )";
  int result = Emulator::emulate(program);
  ASSERT_EQ(result, 4181);
}

TEST(Emulator, MemoryAccess) {
  std::string program = R"(
    Mov R0 12
    Store R0 0
    Load R1 0
    Sub R0 R1
    Jmpz 6
    Jmp 5
    Add R2 R3
  )";
  int result = Emulator::emulate(program);
  ASSERT_EQ(result, 0);
}

TEST(Emulator, Jump) {
  std::string program = R"(
    Mov R0 12
    Jmp 4
    Mov R0 13
    Mov R1 11    
  )";
  int result = Emulator::emulate(program);
  ASSERT_EQ(result, 12);
}

TEST(Emulator, FileFactorial) {
  std::string filename = "factorial.vrisc";
  std::optional<std::string> program = readStringFromFile("factorial.vrisc");
  int result = Emulator::emulate(program.value());
  if (!program) {
    throw std::runtime_error("Can't open file");
  }
  ASSERT_EQ(result, 120);
}

int main() {
  testing::InitGoogleTest();
  bool succesful = RUN_ALL_TESTS();
  return 0;
}
