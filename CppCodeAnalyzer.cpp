#include <tuple>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>

// Список ключевых слов и типов
static const std::unordered_set<std::string> KEYWORDS = {
   "bool", "int", "float", "double", "char", "const", "unsigned", "signed", "long", "short", "void", "static"
};

static bool isContine(std::string& value, char ch) {
    auto it = value.find(ch);
    return it != std::string::npos;
}

static bool isReturn(std::string& value) {
    auto it = value.find("return");
    return it != std::string::npos;
}

static bool isFunc(std::string& value) {
    auto it_close_break1 = value.find(')');  
    auto it_close_break2 = value.find('}');
    auto it_close_break3 = value.find(';');
    return (it_close_break1 != std::string::npos) 
        && (it_close_break2 == std::string::npos)
        && (it_close_break3 != std::string::npos);
}

static std::pair<bool, size_t> isAssignment(const std::string& value) {
    auto it = value.find('=');
    if (it != std::string::npos) {
        return { true, it }; 
    }
    return { false, std::string::npos }; 
}

static std::vector<std::string> SplitNameArgsNoAssignment(const std::string& value) {
    std::string copu_value(value.begin(), value.end());
    std::reverse(copu_value.begin(), copu_value.end());
    std::vector<std::string> rez{};
    std::string buffer{};

    for (auto ch : copu_value)
    {
        if (ch == ' ' || ch == ',') {
            std::reverse(buffer.begin(), buffer.end());
            rez.push_back(buffer);
            buffer = "";
        }
        if (ch == ';') {
            continue;
        }
        else {
            buffer += ch;
        }

      
    }
   
    return rez;
}

static std::string RemoveKeywordsAndTypes(const std::string& value) {
    std::stringstream input(value);
    std::stringstream output;
    std::string word;

    while (input >> word) {
        // Если слово не ключевое, добавляем его в выходную строку
        if (KEYWORDS.find(word) == KEYWORDS.end()) {
            output << word << " ";
        }
    }

    std::string result = output.str();
    result.erase(std::remove(result.begin(), result.end(), ' '), result.end()); // Удаляем лишние пробелы
    return result;
}

static std::vector<std::pair<std::string, bool>> SplitNameArgsAssignment(const std::string& value) {
    // Убираем ключевые слова и типы
    std::string filtered_value = RemoveKeywordsAndTypes(value);

    // Переворачиваем строку
    std::string reversed_value(filtered_value.rbegin(), filtered_value.rend());
    std::vector<std::pair<std::string, bool>> result{};
    std::string buffer{};

    // Лямбда для обработки накопленного текста
    auto processBuffer = [&]() {
        if (!buffer.empty()) {
            std::reverse(buffer.begin(), buffer.end()); // Переворачиваем обратно для анализа
            auto eq_pos = buffer.find('='); // Проверяем наличие '='
            if (eq_pos == std::string::npos) {
                result.emplace_back(buffer, false); // Если '=' нет, просто добавляем имя
            }
            else {
                result.emplace_back(buffer.substr(0, eq_pos), true); // Добавляем имя и флаг наличия присвоения
            }
            buffer.clear(); // Очищаем буфер
        }
        };

    for (char ch : reversed_value) {
        if (ch == ',' || ch == ';') { // Разделители
            processBuffer();
        }
        else if (!std::isspace(ch)) { // Пропускаем пробелы
            buffer += ch;
        }
    }
    processBuffer(); // Обрабатываем остатки

    return result;
}

static std::string KeepOnlyTypes(const std::string& value) {
    std::stringstream input(value);
    std::stringstream output;
    std::string word;

    while (input >> word) {
        // Если слово не ключевое, добавляем его в выходную строку
        if (KEYWORDS.find(word) != KEYWORDS.end()) {
            output << word << " ";
        }
    }

    return output.str();
}

static std::vector<std::tuple<std::string, std::string, std::string>> SplitNameArgsAssignmentSaveType(const std::string& value) {
    // Убираем ключевые слова и типы
    std::string filtered_value = RemoveKeywordsAndTypes(value);
    std::string type_value = KeepOnlyTypes(value);

    std::vector<std::tuple<std::string, std::string, std::string>>  result{};

    auto split = [](std::string filtered_value) {
        std::vector<std::string> rez{};
        size_t start = 0;
        size_t end = filtered_value.find(',');

        while (end != std::string::npos) {
            rez.push_back(filtered_value.substr(start, end - start));
            start = end + 1;
            end = filtered_value.find(',', start);
        }

        // Добавляем последнюю часть строки, если она есть
        if (start < filtered_value.length()) {
            rez.push_back(filtered_value.substr(start));
        }

        return rez;
    };

    auto split_condishen = split(filtered_value);
  
    for (auto& argc : split_condishen) {
        auto it = argc.find('=');
        if (it == std::string::npos) {
            continue;
        }
        it = argc.find(';');
        if (it != std::string::npos) {
            argc.erase(it, 1);
        }
        size_t start = 0;
        size_t end = argc.find('=');
        std::string agrc_name{}, agrc_condish{};

        if (end != std::string::npos) {
            // Часть до знака равенства
            agrc_name = argc.substr(start, end - start);
            // Часть после знака равенства
            agrc_condish = argc.substr(end);  
        }

        

        result.emplace_back(type_value, agrc_name, agrc_condish);
    }

    return result;
}

static void load(std::istream& input, std::ostream& output) {
    std::unordered_map<std::string, bool> decl_or_used_args{};
    std::vector<std::string> copy_input_vec{};
    {
        std::string line, type;
        while (std::getline(input, line))
        {
            copy_input_vec.push_back(line);

            if (!isContine(line, ';') || isReturn(line) || isFunc(line)) {
                continue;
            }

            // в строке нет = 
            if (auto [status, offset] = isAssignment(line); !status) {
                auto split_name_argc = SplitNameArgsNoAssignment(line);
                for (auto& argc : split_name_argc) {
                    decl_or_used_args[argc] = false;
                }
            }
            // в строке есть = 
            else {
                auto split_name_argc = SplitNameArgsAssignment(line);
                for (auto& [argc, used] : split_name_argc) {
                    decl_or_used_args[argc] = used;
                }
               

            }
        }
        for (auto& [argc, used] : decl_or_used_args) {
            std::cout << "name - [ " << argc << " ] used [ " << (used ? "true" : "false") << " ]\n";
        }
    }


    for (auto& line : copy_input_vec) {
        if (!isContine(line, ';') || isReturn(line) || isFunc(line)) {
            output << line << "\n";
            continue;
        }

        // в строке нет = 
        if (auto [status, offset] = isAssignment(line); !status) {
            auto split_name_argc = SplitNameArgsNoAssignment(line);
            for (auto& argc : split_name_argc) {
                if (decl_or_used_args[argc]) {
                    output << line << "\n";
                }
            }
        }
        // в строке есть = 
        else {
            auto split_name_argc = SplitNameArgsAssignmentSaveType(line);
            for (auto& [type, argc, condsh] : split_name_argc) {
                if (decl_or_used_args[argc]) {
                    output << type << " " << argc << condsh << ";" << "\n";
                }
            }
        }
    }
}

static void show_start_menu() {
    namespace fs = std::filesystem;
    std::cout << "Welcome, this is a C++ code analyzer, "
        << "it filters C++ code based on the use of variables, clearing it of unused parts.\n"
        << "To get started, enter the path to the input file: ";

    std::string input_file;
    std::string output_file;

    // Ввод имени входного файла
    std::cin >> input_file;

    std::cout << "Do you want to specify an output file? (Yes/No): ";
    std::string apply_to_him;
    std::cin >> apply_to_him;

    if (apply_to_him == "Yes") {
        std::cout << "Enter the path for the output file: ";
        std::cin >> output_file;
    }
    else {
        // Если "No", используем имя входного файла с добавлением суффикса "_cleaned"
        fs::path input_path(input_file);
        output_file = (input_path.stem().string() + "_cleaned" + input_path.extension().string());
    }

    std::fstream input(input_file, std::ios::in);
    std::fstream output(output_file, std::ios::out);

    if (input.is_open() && output.is_open()) {
        // Предположим, что функция load уже реализована
        load(input, output);
        std::cout << "File analyzed and cleaned successfully.\n";
    }
    else {
        std::cerr << "Failed to open input or output file.\n";
    }
}

int main() {
    show_start_menu();

    return 0;
}
