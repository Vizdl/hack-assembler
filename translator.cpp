// convert an abstract syntax tree for Hack Assembly language into machine code
#include "iobuffer.h"
#include "symbols.h"
#include "tokeniser.h"
#include "abstract-syntax-tree.h"
#include <unordered_map>
#include <cstdio>

// to simplify the code
using namespace std ;
using namespace CS_IO_Buffers ;
using namespace CS_Symbol_Tables ;
using namespace Hack_Assembler ;

#define APERFIX "0"
#define CPERFIX "111"
#define NS "\n"
unordered_map<string,string> 
dest =  {
    {"NULL" ,   "000"},
    {"M"    ,   "001"},
    {"D"    ,   "010"},
    {"MD"   ,   "011"},
    {"A"    ,   "100"},
    {"AM"   ,   "101"},
    {"AD"   ,   "110"},
    {"AMD"  ,   "111"}
};
unordered_map<string,string> 
jump =  {
    {"NULL" ,   "000"},
    {"JGT"  ,   "001"},
    {"JEQ"  ,   "010"},
    {"JGE"  ,   "011"},
    {"JLT"  ,   "100"},
    {"JNE"  ,   "101"},
    {"JLE"  ,   "110"},
    {"JMP"  ,   "111"}
};
unordered_map<string,string> 
comp =  {
    {"0"    ,   "0101010"}  ,   {"1"    ,   "0111111"},
    {"-1"   ,   "0111010"}  ,   {"D"    ,   "0001100"},
    {"A"    ,   "0110000"}  ,   {"!D"   ,   "0001101"},
    {"!A"   ,   "0110001"}  ,   {"-D"   ,   "0001111"},
    {"-A"   ,   "0110011"}  ,   {"D+1"  ,   "0011111"},
    {"A+1"  ,   "0110111"}  ,   {"D-1"  ,   "0001110"},
    {"A-1"  ,   "0110010"}  ,   {"D+A"  ,   "0000010"},
    {"D-A"  ,   "0010011"}  ,   {"A-D"  ,   "0000111"},
    {"D&A"  ,   "0000000"}  ,   {"D|A"  ,   "0010101"},
    {"M"    ,   "1110000"}  ,   {"!M"   ,   "1110001"},
    {"-M"   ,   "1110011"}  ,   {"M+1"  ,   "1110111"},
    {"M-1"  ,   "1110010"}  ,   {"D+M"  ,   "1000010"},
    {"D-M"  ,   "1010011"}  ,   {"M-D"  ,   "1000111"},
    {"D&M"  ,   "1000000"}  ,   {"D|M"  ,   "1010101"}
};


unordered_map<string, string> 
name = {
    {"R0"       ,   "000000000000000"},
    {"SP"       ,   "000000000000000"},
    {"R1"       ,   "000000000000001"},
    {"LCL"      ,   "000000000000001"},
    {"R2"       ,   "000000000000010"},
    {"ARG"      ,   "000000000000010"},
    {"R3"       ,   "000000000000011"},
    {"THIS"     ,   "000000000000011"},
    {"R4"       ,   "000000000000100"},
    {"THAT"     ,   "000000000000100"},
    {"R5"       ,   "000000000000101"},
    {"R6"       ,   "000000000000110"},
    {"R7"       ,   "000000000000111"},
    {"R8"       ,   "000000000001000"},
    {"R9"       ,   "000000000001001"},
    {"R10"      ,   "000000000001010"},
    {"R11"      ,   "000000000001011"},
    {"R12"      ,   "000000000001100"},
    {"R13"      ,   "000000000001101"},
    {"R14"      ,   "000000000001110"},
    {"R15"      ,   "000000000001111"},
    {"SCREEN"   ,   "100000000000000"},
    {"KBD"      ,   "110000000000000"}
};

unordered_map<string, string> label;
unordered_map<string, string> variable;
int address = 16;

string spiltlow15bit(int num){
    string str = "";
    for (int i = 0; i < 15; i++){
        if (num & (1 << i)){
            str = "1" + str;
        }else {
            str = "0" + str;
        }
    }
    return str;
}

void walk_a_name (ast instruction){
    string name_str = get_a_name_unresolved(instruction);
    if (name.find(name_str) != name.end()){
        write_to_output(APERFIX + name[name_str] + NS);
        return;
    }

    if (label.find(name_str) != label.end()){
        write_to_output(APERFIX + label[name_str] + NS);
        return;
    }
    // fatal_error(0,"// error name");
    if (variable.find(name_str) == variable.end()){
        variable[name_str] = spiltlow15bit(address++); 
    }
    write_to_output(APERFIX + variable[name_str] + NS);
}

void walk_a_instruction (ast instruction){
    int value = get_a_instruction_value(instruction);
    write_to_output(APERFIX + spiltlow15bit(value) + NS);
}

void walk_c_instruction (ast instruction){
    write_to_output(CPERFIX + comp[get_c_instruction_alu(instruction)]
     + dest[get_c_instruction_dest(instruction)] + jump[get_c_instruction_jump(instruction)] + NS);
}


// this function provides an example of how to walk an abstract syntax tree constructed by ast_parse_xml()
void asm_translator(ast the_program)
{
    // the_program contains a vector of instructions
    int ninstructions = size_of_program(the_program) ;
    int label_counter = 0;
    for ( int i = 0 ; i < ninstructions ; i++ )
    {
        ast instruction = get_program(the_program,i) ;
        if (ast_node_kind(instruction) == ast_label){
            string label_name = get_label_name(instruction);
            label[label_name] = spiltlow15bit(i - label_counter++);
        }
    }
    for ( int i = 0 ; i < ninstructions ; i++ )
    {
        ast instruction = get_program(the_program,i) ;
        switch(ast_node_kind(instruction))
        {
        case ast_label:
            break ;
        case ast_a_name:
            walk_a_name(instruction);
            break ;
        case ast_a_instruction:
            walk_a_instruction(instruction);
            break ;
        case ast_c_instruction:
            walk_c_instruction(instruction);
            break ;
        default:
            fatal_error(0,"// bad node - expected ast_label,ast_a_name,ast_a_instruction or ast_c_instruction\n") ;
            break ;
        }
    }
}
// main program
int main(int argc,char **argv)
{
    // error messages
    config_errors(iob_buffer) ;

    // parse abstract syntax tree and pass to the translator
    asm_translator(ast_parse_xml()) ;

    // flush output and errors
    print_output() ;
    print_errors() ;
}
