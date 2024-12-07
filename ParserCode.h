#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace parser_code {
    class Node;
    class Document;

    using Array = std::vector<std::string>;
    using Dictionary = std::unordered_map<std::string, Node>;
    using OptDictionary = std::optional<std::unordered_map<std::string, Node>>;
    using OptExpansion = std::optional<std::vector<std::string>>;


    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };


    /*
        static inline void func(){}
        int value1; 
        int value2 = 0; 
        const* int* const value3 = 0;

        название:  func, value1, value2
        расширение слева от названия: [static inline void], [int], [int], [const* int* const]
        расширение справа от названия: [{}], [{}], [{}], [{}]
        вхождение справа от названия: [{}], [;] [ = 0; ],  [ = 0; ],
    
        поик проиходит при 
        [name] = [expression]
        
        m_opt_left_expansion m_name m_opt_right_expansion m_entry =>
        m_name: [ Globale ], m_entry: {
         left_expansion: [ #include ], m_name: [ <iostream> ],
         left_expansion: [ #include ], m_name: [ <algorithm> ] 
        }
    */

    class Node {
    private:
        std::string m_name;                     // название 
        OptDictionary m_entry;                  // вхождение справа от названия
        OptExpansion m_opt_left_expansion;      // расширение слева от названия  
        OptExpansion m_opt_right_expansion;     // расширение справа от названия  
    public:
        Node() = default;
        Node(const std::string& name, 
            const OptDictionary& entry = std::nullopt,
            const OptExpansion& opt_left_expansion = std::nullopt,
            const OptExpansion& opt_right_expansion = std::nullopt
        )
            :m_name(name),  m_entry(entry),
            m_opt_left_expansion(opt_left_expansion),
            m_opt_right_expansion(opt_right_expansion)
        {}

        // секция в {}
        void InsertEntryItem(const Node& node) {
            if (!m_entry) {
                m_entry = std::make_optional<Dictionary>(); 
            }
            m_entry.value()[node.m_name] = node;
        }
        // имя секции 
        void SetName(const std::string& name)
        {
            m_name = name;
        }
        // теги слева
        void SetLeftExpansion(const std::string& left_expansion) {
            if (!m_opt_left_expansion) {
                m_opt_left_expansion = std::make_optional<Array>();
            }
            m_opt_left_expansion.value().push_back(left_expansion);
        }
        // теги справа
        void SetRightExpansion(const std::string& right_expansion) {
            if (!m_opt_right_expansion) {
                m_opt_right_expansion = std::make_optional<Array>();
            }
            m_opt_right_expansion.value().push_back(right_expansion);
        }
    
        const std::string& GetName()const noexcept { return m_name; }
        const OptDictionary& GetEntry()const noexcept { return m_entry; }
        const OptExpansion& GetLeftExpansion()const noexcept { return m_opt_left_expansion; }
        const OptExpansion& GetRightExpansion()const noexcept { return m_opt_right_expansion; }
    };


    class Parsing {
        Node m_root;
        std::unordered_set<std::string> KeyWordsLanguage = {
            "int", "bool", "char", "double", "long", "void",
            "static", "const", "noexcept", "#inlude"
        };

    public:
        Parsing(std::istream& input) : m_root("") {
            auto code = CopyStreamToString(input);
            ProcessParsing(code);
        }

        const Node& Get() const noexcept { return m_root; }
    private:

        void ProcessParsing(const std::string& input_code) {
            auto supportive = input_code.begin();
            auto begin = input_code.begin();
            auto end = input_code.end();

            while (begin != end)
            {
                // std::cout << *begin;
                // препроцессы
                if (*begin == '#') {
                    size_t offset = ParsingPreprocess(begin, end);
                    // выходим так как обработали весь файл  
                    if (!IsValid(begin, end, offset)) { break; }
                    begin += offset;

                }
                else
                {
                    ++begin;
                }
            }



        }

        /*
            


        */


        template<typename iterator>
        size_t ParsingPreprocess(iterator begin, iterator end) {
            // Преобразуем первое слово (например, "#include" или "#define")
            auto word_preprocess = CompleteWord(begin, end, ' ');

            std::cout << "word preprocess: " << word_preprocess << "\n";

            // Обработка директивы #include
            if (word_preprocess == "#include") {
                // Получаем имя файла из <>
                auto include_target_str = CompleteWord(std::next(begin, word_preprocess.size()), end, '\n');

                Node include_target{ include_target_str };
                include_target.SetLeftExpansion(word_preprocess);
                m_root.InsertEntryItem(include_target);
                return word_preprocess.size() + include_target_str.size();
            }
            // Обработка директивы #define
            else if (word_preprocess == "#define") {
                // Получаем имя макроса
                auto macro_name = CompleteWord(std::next(begin, word_preprocess.size()), end, ' ');

                
            }

            return word_preprocess.size();
        }



        template <typename Iterator>
        bool IsValid(Iterator begin, Iterator end, size_t offset) {
            return std::distance(begin, end) >= static_cast<std::ptrdiff_t>(offset);
        }

        template <typename Iterator>
        std::string CompleteWord(Iterator begin, Iterator end, char delimeter = ' ') {
            auto delimeterPos = std::find(begin, end, delimeter);
            return std::string(begin, ((delimeterPos != end) ? delimeterPos : end)); 
        }



        std::string CopyStreamToString(std::istream& inputStream) {
            std::stringstream buffer;
            std::streampos originalPos = inputStream.tellg();
            buffer << inputStream.rdbuf();
            inputStream.seekg(originalPos);
            return buffer.str();
        }

 
    };


    class PrintContext {
        bool m_ast_forma;
    public:
        PrintContext(const Document& doc, std::ostream& output, bool ast_forma);
    private:
        void Print(const Node& doc, std::ostream& output, int indent_level = 0) const noexcept;

        void PrintLeftExpansion(const OptExpansion& OptExp, std::ostream& output, int indent_level) const noexcept;
        void PrintRightExpansion(const OptExpansion& OptExp, std::ostream& output, int indent_level) const noexcept;
        void PrintName(const std::string& name, std::ostream& output, int indent_level) const noexcept;
        void PrintEntry(const OptDictionary& name, std::ostream& output, int indent_level) const noexcept;
    };

      
    class Document {
    public:
        Node m_root;
        explicit Document(Node root) : m_root(root) {}
        const Node& Get() const noexcept { return m_root; }
    };
    Document Load(std::istream& input);
    void PrintAST(const Document& doc, std::ostream& output) noexcept;
}