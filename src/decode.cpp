#include <bitset>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <cstdint>

const uint8_t opcode{0b11111100};
const uint8_t d{0b00000010};
const uint8_t w{0b00000001};
const uint8_t mod{0b11000000};
const uint8_t reg{0b00111000};
const uint8_t r_m{0b00000111};

const uint8_t mov{0b10001000};

std::unordered_map<uint8_t, std::string> opcodes{ {mov, "mov "} };

const char* W0[8] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
const char* W1[8] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};

int main(int argc, const char* argv[]) {
  FILE* file = fopen(argv[1], "rb");
  if (file == nullptr) {
    std::printf("Error reading file %s.\n", argv[1]);
    return -1;
  }

  std::fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(fileSize+1);
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  buffer[bytesRead] = '\0';

  std::cout << "; " << argv[1] << '\n';
  std::cout << "bits 16\n\n";

  for (int i = 0; i < bytesRead-1; i+=2) {
    uint8_t a(buffer[i]);
    uint8_t b(buffer[i+1]);
#ifdef DEBUG
    std::cout << "Byte " << i << ": " << a << '\n';
    std::cout << "Byte " << i+1 << ": " << b << '\n';
#endif

    auto my_opcode = (a & opcode);
    auto my_d = (a & d) >> 1;
    auto my_w = (a & w);
    auto my_mod = (b & mod);
    auto my_reg = (b & reg) >> 3;
    auto my_r_m  = (b & r_m);
    char buf1[3];
    char buf2[3];

    std::cout << opcodes[my_opcode];
    if (my_w == 0) {
      // W0
      std::strncpy(buf1, W0[my_reg],2);
      std::strncpy(buf2, W0[my_r_m],2);
    } else {
      // W1
      std::strncpy(buf1, W1[my_reg],3);
      std::strncpy(buf2, W1[my_r_m],3);
    }

    if (my_d == 0) {
      std::cout << buf2 << ", " << buf1 << '\n';
    } else {
      std::cout << buf1 << ", " << buf2 << '\n';
    }

  }

  fclose(file);
  return 0;
}
