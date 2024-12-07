#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <unordered_map>
#include "ParserCode.h"

static std::string path_dir = "D:\\Dev\\temp_project\\UNIVER_LABS\\CSBP\\CppCodeAnalyzer\\CppCodeAnalyzer\\test";


int main() {
    int main = 1;
    {
        bool main_ = true;
    }
    


    std::fstream input(path_dir + "\\main.cpp", std::ios::in);

    if (input.is_open()) {

        auto document = parser_code::Load(input);

        parser_code::PrintAST(document, std::cout);

        std::cout << "File analyzed and cleaned successfully.\n";
    }
    else {
        std::cerr << "Failed to open input file.\n";
    }

    return 0;

}

