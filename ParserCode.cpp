#include "ParserCode.h"
#include <algorithm>
#include <iterator>


namespace parser_code {
    static void PrintIndent(std::ostream& output, int indent_level) {
        for (int i = 0; i < indent_level; ++i) {
            output << "  "; // two spaces per indent level
        }
    }

    PrintContext::PrintContext(const Document& doc, std::ostream& output, bool ast_forma)
        : m_ast_forma(ast_forma) {
        Print(doc.Get(), output);
    }

    void PrintContext::Print(const Node& node, std::ostream& output, int indent_level) const noexcept {
       
       
        if (m_ast_forma) {
            PrintIndent(output, indent_level);
        }
        PrintLeftExpansion(node.GetLeftExpansion(), output, indent_level);
        if (m_ast_forma) {
            PrintIndent(output, indent_level);
        }
        PrintName(node.GetName(), output, indent_level);
        if (m_ast_forma) {
            PrintIndent(output, indent_level);
        }
        auto& RightExpansion = node.GetRightExpansion();
        if (!RightExpansion) { output << "\n"; }
        PrintRightExpansion(RightExpansion, output, indent_level);
        if (m_ast_forma) {
            PrintIndent(output, indent_level);
        }
        auto& Entry = node.GetEntry();
        if (!Entry && RightExpansion) { output << "\n"; }
        PrintEntry(node.GetEntry(), output, indent_level);
    }

    void PrintContext::PrintLeftExpansion(const OptExpansion& OptExp, std::ostream& output, int indent_level) const noexcept {
        if (!OptExp) { return; }
        
        if (m_ast_forma) {
            PrintIndent(output, indent_level);
            output << "Left expansion: [ ";
        }
        for (const auto& item : OptExp.value()) {
            output << item << " ";
        }
        if (m_ast_forma) {
            output << "], ";
        }
    }

    void PrintContext::PrintName(const std::string& name, std::ostream& output, int indent_level) const noexcept {
        if (m_ast_forma) {
            PrintIndent(output, indent_level);
            output << "Name: [ ";
        }

        output << name;

        if (m_ast_forma) {
            output << " ], ";
        }
    }

    void PrintContext::PrintRightExpansion(const OptExpansion& OptExp, std::ostream& output, int indent_level) const noexcept {
        if (!OptExp) { return; }
       
        if (m_ast_forma) {
            PrintIndent(output, indent_level);
            output << "Right expansion: [ ";
        }
        for (const auto& item : OptExp.value()) {
            output << item << " ";
        }
        if (m_ast_forma) {
            output << "], ";
        }
    }

    void PrintContext::PrintEntry(const OptDictionary& Entry, std::ostream& output, int indent_level) const noexcept {
        if (!Entry) { return; }
       
        if (m_ast_forma) {
            PrintIndent(output, indent_level);
            output << "Entry: { \n";
        }

        for (const auto& [name, entry_node] : Entry.value()) {
            if (m_ast_forma) {
                PrintIndent(output, indent_level + 1); 
            }
            Print(entry_node, output, indent_level + 1);
        }

        if (m_ast_forma) {
            output << "}";
        }
    }




   
    Document Load(std::istream& input) {
        Parsing parser{ input };
        return Document{ parser.Get() };
    }

    void PrintAST(const Document& doc, std::ostream& output) noexcept {
        PrintContext(doc, output, false);
    }
}
