#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

typedef unsigned char uchar;
const int MAX_SIZE = 16;

void CreateDict(std::unordered_map<std::string, int>& dict) {
  for (int i = 1; i <= (1 << 8); ++i) {
    std::string symbol(1, static_cast<uchar>(i - 1));
    dict[symbol] = i;
  }
  return;
}

void CreateDictReverse(std::unordered_map<int, std::string>& dict) {
  for (int i = 1; i <= (1 << 8); ++i) {
    std::string symbol(1, static_cast<uchar>(i - 1));
    dict[i] = symbol;
  }
  return;
}

bool CheckDict(std::unordered_map<int, std::string>& dict, int& length) {
  if (dict.size() == (1 << length) - 1) {
    if (length == MAX_SIZE) {
      dict.clear();
      length = 9;
      CreateDictReverse(dict);
      return true;
    } else {
      ++length;
    }
  }
  return false;
}

void GammaCode(int val, std::vector<bool>& vec) {
  std::vector<bool> temp;
  while (val != 0) {
    temp.push_back(val & 1);
    val >>= 1;
  }
  for (int i = 0; i < temp.size() - 1; ++i) {
    vec.push_back(0);
  }
  for (int i = temp.size() - 1; i >= 0; --i) {
    vec.push_back(temp[i]);
  }
  return;
}

void CodeOutput(int val, int length, std::vector<bool>& vec) {
  std::vector<bool> temp(length);
  int pos = temp.size() - 1;
  while (val != 0) {
    temp[pos] = (val & 1);
    val >>= 1;
    --pos;
  }
  for (int i = 0; i < temp.size(); ++i) {
    vec.push_back(temp[i]);
  }
  return;
}

std::vector<bool> Encode_Unlimited(std::vector<uchar>& bytes) {
  int it = 0;
  std::unordered_map<std::string, int> dict;
  for (int i = 1; i <= (1 << 8); ++i) {
    std::string symbol(1, static_cast<uchar>(i - 1));
    dict[symbol] = i;
  }
  std::vector<bool> bin;
  std::string cur = "";
  std::string prev = "";
  while (it < bytes.size()) {
    cur += bytes[it];
    while ((dict.count(cur) != 0 || cur.length() == 0) && it < bytes.size()) {
      ++it;
      prev = cur;
      cur += bytes[it];
    }
    GammaCode(dict[prev], bin);
    int value = dict.size() + 1;
    dict[cur] = value;
    cur.clear();
    prev.clear();
  }
  return bin;
}

std::vector<uchar> Decode_Unlimited(std::vector<uchar>& bytes) {
  std::vector<uchar> output;
  std::unordered_map<int, std::string> dict;
  for (int i = 1; i <= (1 << 8); ++i) {
    std::string symbol(1, static_cast<uchar>(i - 1));
    dict[i] = symbol;
  }
  std::vector<int> code;
  int it = 0;
  std::vector<bool> bits;
  for (int i = 0; i < bytes.size(); ++i) {
    char byte = bytes[i];
    for (int i = 7; i >= 0; --i) {
      bits.push_back(byte & (1 << i));
    }
  }
  while (it < bits.size()) {
    int zero = 0;
    int num = 0;
    while (!bits[it] && it < bits.size()) {
      ++zero;
      ++it;
    }
    int pos = zero;
    for (int i = 0; i <= zero && it < bits.size(); ++i) {
      num |= bits[it] << pos;
      --pos;
      ++it;
    }
    if (pos == -1) code.push_back(num);
  }
  std::string prev = "";
  std::string cur = "";
  for (int i = 0; i < code.size(); ++i) {
    if (dict.count(code[i])) {
      cur = dict[code[i]];
      int value = dict.size() + 1;
      if (prev.length() != 0) dict[value] = prev + cur[0];
    } else {
      cur = prev + prev[0];
      int value = dict.size() + 1;
      dict[value] = cur;
    }
    prev = cur;
    for (int j = 0; j < cur.length(); ++j) {
      output.push_back(static_cast<uchar>(cur[j]));
    }
  }
  return output;
}

std::vector<bool> Encode_Limited(std::vector<uchar>& bytes) {
  std::vector<bool> bin;
  int length = 9;

  std::unordered_map<std::string, int> dict;
  CreateDict(dict);
  std::string cur = "";
  std::string prev = "";
  int it = 0;
  while (it < bytes.size()) {
    cur += bytes[it];
    while ((dict.count(cur) != 0 || cur.length() == 0) && it < bytes.size()) {
      ++it;
      prev = cur;
      cur += bytes[it];
    }
    CodeOutput(dict[prev], length, bin);
    int value = dict.size() + 1;
    dict[cur] = value;
    if (dict.size() == (1 << length)) {
      if (length != MAX_SIZE)
        ++length;
      else {
        dict.clear();
        length = 9;
        CreateDict(dict);
      }
    }
    cur.clear();
    prev.clear();
  }
  return bin;
}

std::vector<uchar> Decode_Limited(std::vector<uchar>& bytes) {
  int length = 9;
  std::vector<uchar> output;
  std::unordered_map<int, std::string> dict;
  CreateDictReverse(dict);

  int it = 0;
  std::vector<bool> bits;
  for (int i = 0; i < bytes.size(); ++i) {
    char byte = bytes[i];
    for (int i = 7; i >= 0; --i) {
      bits.push_back(byte & (1 << i));
    }
  }
  std::string prev = "";
  std::string cur = "";
  while (it < bits.size()) {
    int code = 0;
    int pos = length - 1;
    for (int i = 0; i < length && it < bits.size(); ++i) {
      code |= bits[it] << pos;
      --pos;
      ++it;
    }
    if (pos == -1) {
      bool f = false;
      if (dict.count(code)) {
        cur = dict[code];
        int value = dict.size() + 1;
        if (prev.length() != 0) {
          dict[value] = prev + cur[0];
          f = CheckDict(dict, length);
        }
      } else {
        cur = prev + prev[0];
        int value = dict.size() + 1;
        dict[value] = cur;
        f = CheckDict(dict, length);
      }
      if (f)
        prev = "";
      else
        prev = cur;
      for (int j = 0; j < cur.length(); ++j) {
        output.push_back(static_cast<uchar>(cur[j]));
        //std::cout << static_cast<uchar>(cur[j]) << '\n';
      }
    }
  }
  return output;
}

void PrintBin(std::string file_output, std::vector<bool>& bin) {
  std::ofstream out(file_output, std::ios::binary);
  char byte = 0;
  int it = 0;
  while (it < (int)bin.size()) {
    for (int i = 7; i >= 0 && it < (int)bin.size(); --i) {
      byte |= bin[it] << i;
      ++it;
    }
    out.write(&byte, sizeof(byte));
    byte = 0;
  }
  return;
}

void PrintRes(std::string file_output, std::vector<uchar>& output) {
  std::ofstream out(file_output, std::ios::binary);
  for (int i = 0; i < output.size(); ++i) {
    out.write((char*)&output[i], sizeof(char));
  }
  return;
}

void GetData(std::string file_input, std::vector<uchar>& bytes) {
  std::ifstream in(file_input, std::ios::binary);
  char byte;
  while (in.read(&byte, sizeof(byte))) {
    bytes.push_back(static_cast<uchar>(byte));
  }
  return;
}

int main() {
  while (true) {
    std::cout << "\033[1;36m Enter file name to input: \033[0m";
    std::string file_input, file_output;
    std::cin >> file_input;
    std::cout << "\n\033[1;36m Enter file name to output: \033[0m";
    std::cin >> file_output;

    std::vector<uchar> bytes;
    GetData(file_input, bytes);

    std::cout << "\n\033[1;36m Enter the command number:\n1)Encode "
                 "file\n2)Decode file\n\033[0m";
    int command;
    std::cin >> command;
    std::cout << "\n\033[1;36m Enter the dictionary type "
                 "number:\n1)Limited\n2)Unlimited\n\033[0m";
    int type;
    std::cin >> type;

    if (command == 1) {
      std::vector<bool> bin;
      if (type == 1) {
        bin = Encode_Limited(bytes);
      } else if (type == 2) {
        bin = Encode_Unlimited(bytes);
      }
      PrintBin(file_output, bin);
    } else if (command == 2) {
      std::vector<uchar> output;
      if (type == 1) {
        output = Decode_Limited(bytes);
      } else if (type == 2) {
        output = Decode_Unlimited(bytes);
      }
      PrintRes(file_output, output);
    }
    std::cout << "\n\033[1;32m DONE!\n\033[0m";
  }
  return 0;
}
