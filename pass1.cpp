#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>

using namespace std;

// Define condition codes and register codes
map<string, int> condition_codes = {
 {"LT", 1},
 {"LE", 2},
 {"EQ", 3},
 {"GT", 4},
 {"GE", 5},
 {"ANY", 6}
};

map<string, int> register_codes = {
 {"AREG", 1},
 {"BREG", 2},
 {"CREG", 3},
 {"DREG", 4}
};

map<string, pair<string, int>> mnemonics = {
 {"STOP", {"IS", 0}},
 {"ADD", {"IS", 1}},
 {"SUB", {"IS", 2}},
 {"MULT", {"IS", 3}},
 {"MOVER", {"IS", 4}},
 {"MOVEM", {"IS", 5}},
 {"COMP", {"IS", 6}},
 {"BC", {"IS", 7}},
 {"DIV", {"IS", 8}},
 {"READ", {"IS", 9}},
 {"PRINT", {"IS", 10}},
 {"START", {"AD", 1}},
 {"END", {"AD", 2}},
 {"ORIGIN", {"AD", 3}},
 {"EQU", {"AD", 4}},
 {"LTORG", {"AD", 5}},
 {"DC", {"DL", 1}},
 {"DS", {"DL", 2}}
};

// Initialize data structures
vector<pair<string, int>> symtab; // Symbol Table
vector<pair<string, int>> lit_tab(10, {"", -1}); // Literal Table
vector<int> pool_tab(10, -1); // Pool Table
int lit_tab_ptr = 0; // Literal Table pointer
int pool_tab_ptr = 0; // Pool Table pointer
int location_cntr = 1; // Location Counter
vector<string> ic_lines; // Intermediate Code Lines

// Function to find a symbol in the symbol table
int find_symbol(const string &symbol_name, int &addr) {
 for (int i = 0; i < symtab.size(); ++i) {
 if (symtab[i].first == symbol_name) {
 addr = symtab[i].second;
 return i;
 }
 }
 return -1;
}

// Function to update or add a symbol in the symbol table
int update_symbol(const string &symbol_name, int addr) {
 int index;
 if (find_symbol(symbol_name, index) != -1) {
 if (addr != -1) {
 symtab[index].second = addr;
 }
 return index;
 }
 symtab.push_back({symbol_name, addr});
 return symtab.size() - 1;
}

// Function to initialize literals in the literal table
void init_literals() {
 for (int i = pool_tab[pool_tab_ptr]; i < lit_tab_ptr; ++i) {
 lit_tab[i].second = location_cntr;
 ++location_cntr;
 }
 ++pool_tab_ptr;
 pool_tab[pool_tab_ptr] = lit_tab_ptr;
}

// Function to process each line of the assembly code
void process_line(const vector<string> &line_tokens) {
 static int lit_tab_ptr = 0; // Ensure lit_tab_ptr persists between calls

 if (line_tokens.empty()) return;

 string label = "", mnemonic_str = "", operand1 = "", operand2 = "";
 if (mnemonics.find(line_tokens[0]) != mnemonics.end()) {
 mnemonic_str = line_tokens[0];
 if (line_tokens.size() == 2) {
 operand1 = line_tokens[1];
 } else if (line_tokens.size() == 3) {
 operand1 = line_tokens[1];
 operand2 = line_tokens[2];
 }
 } else {
 label = line_tokens[0];
 mnemonic_str = line_tokens[1];
 if (line_tokens.size() == 3) {
 operand1 = line_tokens[2];
 } else if (line_tokens.size() == 4) {
 operand1 = line_tokens[2];
 operand2 = line_tokens[3];
 }
 }

 auto it = mnemonics.find(mnemonic_str);
 if (it == mnemonics.end()) {
 cout << "Error: Unknown mnemonic " << mnemonic_str << endl;
 return;
 }

 string mnemonic_class = it->second.first;
 int mnemonic_opcode = it->second.second;
 stringstream ic_line;
 ic_line << "(" << mnemonic_class << "," << mnemonic_opcode << ") ";

 if (mnemonic_class != "AD" && !label.empty()) {
 update_symbol(label, location_cntr);
 }

 if (mnemonic_str == "LTORG") {
 init_literals();
 }

 if (mnemonic_str == "START") {
 location_cntr = stoi(operand1);
 ic_line << "(C," << location_cntr << ")";
 } else if (mnemonic_str == "ORIGIN") {
 if (operand1.find("+") != string::npos || operand1.find("-") != string::npos) {
 string symbol_name, constant;
 istringstream ss(operand1);
 ss >> symbol_name >> constant;
 int symbol_addr;
 find_symbol(symbol_name, symbol_addr);
 location_cntr = stoi(operand1.substr(0, operand1.find(symbol_name))) + symbol_addr;
 ic_line << "(C," << location_cntr << ")";
 } else {
 int symbol_addr;
 int symbol_index = find_symbol(operand1, symbol_addr);
 if (symbol_index == -1) {
 location_cntr = stoi(operand1);
 ic_line << "(C," << location_cntr << ")";
 } else {
 location_cntr = symbol_addr;
 ic_line << "(S," << symbol_index << ")";
 }
 }
 } else if (mnemonic_str == "EQU") {
 if (operand1.find("+") != string::npos || operand1.find("-") != string::npos) {
 string symbol_name, constant;
 istringstream ss(operand1);
 ss >> symbol_name >> constant;
 int symbol_addr;
 find_symbol(symbol_name, symbol_addr);
 int updated_val = stoi(operand1.substr(0, operand1.find(symbol_name))) + symbol_addr;
 ic_line << "(S," << update_symbol(label, updated_val) << ") (C," << updated_val << ")";
 } else {
 int symbol_addr;
 int symbol_index = find_symbol(operand1, symbol_addr);
 int updated_val = (symbol_index == -1) ? 0 : symbol_addr;
 update_symbol(label, updated_val);
 ic_line << "(S," << symbol_index << ") (C," << updated_val << ")";
 }
 } else if (mnemonic_str == "DC" || mnemonic_str == "DS") {
 int symbol_index = update_symbol(label, location_cntr);
 if (mnemonic_str == "DC") {
 location_cntr += 1;
 operand1 = operand1.substr(1, operand1.size() - 2);
 } else if (mnemonic_str == "DS") {
 location_cntr += stoi(operand1);
 }
 ic_line << "(S," << symbol_index << ") (C," << operand1 << ")";
 } else if (mnemonic_class == "IS") {
 if (mnemonic_str == "READ" || mnemonic_str == "PRINT") {
 int symbol_index = update_symbol(operand1, -1);
 ic_line << "(S," << symbol_index << ")";
 } else if (mnemonic_str == "BC") {
 string condition = operand1.substr(0, operand1.find(','));
 int symbol_index = update_symbol(operand2, -1);
 ic_line << "(" << condition_codes[condition] << ") ";
 ic_line << "(S," << symbol_index << ")";
 } else if (mnemonic_str == "STOP") {
 // Nothing to append for STOP
 } else {
 if (operand2.find('=') != string::npos) {
 string literal_val = operand2.substr(operand2.find('=') + 1);
 ic_line << "(" << register_codes[operand1.substr(0, operand1.find(','))] << ") (L," << lit_tab_ptr << ")";
 lit_tab[lit_tab_ptr++] = {literal_val, -1};
 } else {
 int symbol_index = update_symbol(operand2, -1);
 ic_line << "(" << register_codes[operand1.substr(0, operand1.find(','))] << ") (S," << symbol_index << ")";
 }
 }
 ++location_cntr;
 }

 ic_lines.push_back(ic_line.str());
}

int main() {
 string source_file_path;
 cout << "Enter source code file path: ";
 getline(cin, source_file_path);

 ifstream file(source_file_path);
 if (!file.is_open()) {
 cerr << "Error: Unable to open file " << source_file_path << endl;
 return 1;
 }

 string line;
 while (getline(file, line)) {
 stringstream ss(line);
 vector<string> line_tokens;
 string token;
 while (ss >> token) {
 line_tokens.push_back(token);
 }
 process_line(line_tokens);
 }
 file.close();

 init_literals();

 // Print intermediate code, symbol table, literal table, and pool table
 cout << "-------------- INTERMEDIATE CODE----------------" << endl;
 for (const auto &line : ic_lines) {
 cout << line << endl;
 }

 cout << "--------- SYMBOL TABLE -------------" << endl;
 for (const auto &entry : symtab) {
 cout << entry.first << " " << entry.second << endl;
 }

 cout << "--------- LITERAL TABLE -------------" << endl;
 for (int i = 0; i < lit_tab_ptr; ++i) {
 cout << lit_tab[i].first << " " << lit_tab[i].second << endl;
 }

 cout << "---------- POOL TABLE --------------- " << endl;
 for (int i = 0; i <= pool_tab_ptr; ++i) {
 cout << pool_tab[i] + 1 << endl;
 }

 return 0;
}