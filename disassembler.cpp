// convert Hack Machine Code into Assembly Language
#include "iobuffer.h"
#include "symbols.h"
#include "tokeniser-d.h"
#include <vector>
#include <iostream>
#include <unordered_map>
#include <cstdio>
#include <map>
#include <sstream>
// to make out programs a bit neater
using namespace std ;

using namespace CS_IO_Buffers ;
using namespace CS_Symbol_Tables ;
using namespace Hack_Disassembler ;

// This program must disassemble Hack Machine Code and output the equivalent Hack Assembly Language.

// A-instructions must be translated into symbolic form using the following rules:

// 1. IF symbols is false
//    THEN
//     - the A-instruction is output in numeric form.

// 2. IF an A-instruction is followed by a C-instruction and the jump bits of the C-instruction are not "000"
//    THEN
//     - IF the value of the A-instruction is the address an instruction in the program
//       THEN
//        - the instruction must be allocated a label and the A-instruction must use this label
//        - the first labelled location in memory must be labelled L0, the next L1 and so on

// 3. IF an A-instruction is followed by a C-instruction that reads or writes memory
//    THEN
//     - IF the address has a predefined name
//       THEN
//        - the A-instruction must use the predefined name
//        - where there is a choice, use SP, LCL, ARG, THIS and THAT in preference to R0, R1, R2 R3 and R4
//    - IF the address is the address of an existing variable location
//      THEN
//       - the A-instructon must use the variable's name
//    - IF the address is the address of the next free variable location
//      THEN
//       - the A-instructon must use the next free variable's name and
//       - the address of the next free variable must be incremented by 1
//       - the first variable name allocated will be for address 16 and will be called v_0
//       - the second variable name allocated will be for address 17 and will be called v_1
//       - and so on

// 4. ELSE the A-instruction is output in numeric form.

// C-instructions must be translated using the following rules:

// 1. IF the destination bits are "000"
//    THEN
//     - do not output a destination component for the C-instruction

// 2. IF the ALU operation is not recognised
//   THEN
//    - replace the ALU operation with: "< ** UNDEFINED ALU OPERATION ** >"

// Do not output a jump component for a C-instruction if it the jump bits are "000"
// 3. IF the jump bits are "000"
//    THEN
//     - do not output a jump componentfor the C-instruction


// Hack Machine Code:
// program  ::= (instr tk_eol?)* tk_eoi
// instr    ::= tk_a_instr | tk_c_instr


#define SPACE   "        "
#define EQU     "="
#define NOTCOMP "< ** UNDEFINED ALU OPERATION ** >"
#define NULL_TOKEN "000"
#define LABEL_HEAD "L"
#define EMPTY_STR ""
#define NEW_LINE "\n"
#define LEFT "("
#define RIGHT ")"
unordered_map<string,string> 
dest =  {
    {"000" ,   "NULL"},
    {"001"    ,   "M"},
    {"010"    ,   "D"},
    {"011"   ,   "MD"},
    {"100"    ,   "A"},
    {"101"   ,   "AM"},
    {"110"   ,   "AD"},
    {"111" ,   "AMD" }
};
unordered_map<string,string> 
jump =  {
    {"000" ,   "NULL"},
    {"001"  ,   "JGT"},
    {"010"  ,   "JEQ"},
    {"011"  ,   "JGE"},
    {"100"  ,   "JLT"},
    {"101"  ,   "JNE"},
    {"110"  ,   "JLE"},
    {"111"  ,   "JMP"}
};
unordered_map<string,string> 
comp =  {
    {"0101010"    ,   "0"}  ,   {"0111111"   ,   "1"},
    {"0111010"   ,   "-1"}  ,   {"0001100"    ,  "D" },
    {"0110000"    ,   "A"}  ,   {"0001101"   ,  "!D"},
    {"0110001"   ,   "!A"}  ,   {"0001111"   ,   "-D"},
    {"0110011"   ,   "-A"}  ,   {"0011111"  ,   "D+1"},
    { "0110111" ,   "A+1"}  ,   {"0001110"  ,   "D-1"},
    {"0110010"  ,   "A-1"}  ,   {"0000010" ,   "D+A"},
    {"0010011"  ,  "D-A" }  ,   {"0000111" ,  "A-D" },
    {"0000000"  ,  "D&A" }  ,   { "0010101" ,  "D|A" },
    {"1110000"    ,  "M" }  ,   {"1110001"   ,   "!M"},
    {"1110011"   ,   "-M"}  ,   {"1110111"  ,  "M+1" },
    {"1110010"  ,   "M-1"}  ,   {"1000010"  ,   "D+M"},
    {"1010011"  ,   "D-M"}  ,   {"1000111"  ,  "M-D" },
    {"1000000"  ,   "D&M"}  ,   {"1010101"  ,   "D|M"}
};

unordered_map<int, string>
predefine = {
        {0  , "@SP\n"},
        {1  , "@LCL\n"},
        {2  , "@ARG\n"},
        {3  , "@THIS\n"},
        {4  , "@THAT\n"},
        {5  , "@R5\n"},
        {6  , "@R6\n"},
        {7  , "@R7\n"},
        {8  , "@R8\n"},
        {9  , "@R9\n"},
        {10 , "@R10\n"},
        {11 , "@R11\n"},
        {12 , "@R12\n"},
        {13 , "@R13\n"},
        {14 , "@R14\n"},
        {15 , "@R15\n"},
        {16384  , "@SCREEN\n"},
        {24576, "@KBD\n"}
};


map<int, string> label;

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




// parse Hack Machine Code and return a vector of tokens, one per instruction
vector<Token> parse_program()
{
    vector<Token> instructions ;

    while ( have(tk_instruction) )
    {
        instructions.push_back(mustbe(tk_instruction)) ;
        if ( have(tk_eol) ) next_token() ;
    }
    mustbe(tk_eoi) ;

    return instructions ;
}

/*****************   REPLACE THE FOLLOWING CODE  ******************/


void output_c_instr(string dest, string comp, string jump){
    if (dest != "NULL"){
        write_to_output(dest + EQU);
    }
    write_to_output(comp); 
    
    if (jump != "NULL"){
        write_to_output(";" + jump);
    }
    write_to_output("\n");
}

void out_a_instr(int value){
    char a_instr[1024];
    sprintf(a_instr, "@%d\n",value);
    write_to_output(string(a_instr));
}

// disassemble a Hack Machine Code program without using any symbols
static void disassemble_no_symbols(vector<Token> instructions)
{
    for (int i = 0; i < instructions.size(); i++)
    {
        string comp_str;
        write_to_output(SPACE);
        Token instruction = instructions[i];
        if (tk_eol == token_kind(instruction))
            continue;
        if (token_kind(instruction) == tk_c_instr){
            string dest_bit = token_dest(instruction);
            string jump_bit = token_jump(instruction);
            string comp_bit = token_a_bit(instruction) + token_alu_op(instruction);
            if (dest.find(dest_bit) == dest.end()){
                fatal_error(0,"// error dest\n") ;
            }
            string dest_str = dest[token_dest(instruction)];

            if (comp.find(comp_bit) != dest.end()){
                comp_str = comp[comp_bit];
            }else {
                comp_str = NOTCOMP;
            }

            if (jump.find(jump_bit) == jump.end()){
                fatal_error(0,"// error jump\n") ;
            }
            string jump_str = jump[jump_bit];
            output_c_instr(dest_str, comp_str, jump_str);
        }else {
            int value = token_ivalue(instruction);
            out_a_instr(value);
        }
    }
}

// convert Hack Machine Code into Assembly Language
// use predefined names, generic label names and generic variable names
static void disassemble_symbols(vector<Token> instructions)
{
    int size = instructions.size();
    for (int i = 0; i < size; i++){
        Token instruction = instructions[i];
        int value = token_ivalue(instruction);
        if (token_kind(instruction) == tk_a_instr && i + 1 < size && 
        token_kind(instructions[i + 1]) == tk_c_instr){
            Token next_instruction = instructions[i + 1];
            string jump_token = token_jump(next_instruction);
        // cout << jump_token << endl;
            if (NULL_TOKEN != jump_token && value < size - 1){
                label[value] = EMPTY_STR;
            }
        }
    }
    int count = 0;
    map<int, string>::iterator it,next;
    for (it = label.begin(); it != label.end(); it++){
        stringstream s;
        s << LABEL_HEAD << count ++ ;
        (*it).second = s.str();
    }
    it = label.begin();
    int addr = 16;
    for (int i = 0; i < size; i++)
    {
        // cout << (it != label.begin()) << endl;
        if (it != label.end() && i == (*it).first){
            write_to_output( LEFT + (*it).second + RIGHT + "\n");
            it++;
        }

        write_to_output(SPACE);
        Token instruction = instructions[i];
        if (tk_eol == token_kind(instruction))
            continue;
        if (token_kind(instruction) == tk_c_instr){
            string comp_str = "";
            string dest_bit = token_dest(instruction);
            string jump_bit = token_jump(instruction);
            string comp_bit = token_a_bit(instruction) + token_alu_op(instruction);
            if (dest.find(dest_bit) == dest.end()){
                fatal_error(0,"// error dest\n") ;
            }
            string dest_str = dest[token_dest(instruction)];

            if (comp.find(comp_bit) != comp.end()){
                comp_str = comp[comp_bit];
            }else {
                comp_str = NOTCOMP;
            }

            if (jump.find(jump_bit) == jump.end()){
                fatal_error(0,"// error jump\n") ;
            }
            string jump_str = jump[jump_bit];
            output_c_instr(dest_str, comp_str, jump_str);
        }else {
            int value = token_ivalue(instruction);
            if (i + 1 < size && token_kind(instructions[i + 1]) == tk_c_instr){
                Token next_instruction = instructions[i + 1];
                string jump_token = token_jump(next_instruction);
                string jump_str = jump[jump_token];
                string comp_bit = token_a_bit(next_instruction) + token_alu_op(next_instruction);
                // string comp_str = comp[comp_bit];
                string dest_str = dest[token_dest(next_instruction)];
                if (NULL_TOKEN != jump_token && label.find(value) != label.end()){
                    write_to_output("@" + label[value] + "\n");
                    continue;
                }
                // cout << comp_bit << endl;
                if (comp.find(comp_bit) == comp.end() || 
                comp[comp_bit].find("M") != comp[comp_bit].npos || dest_str.find("M") != dest_str.npos ){
                    // cout <<"dwokjd" <<endl;
                    if (predefine.find(value) != predefine.end()){
                        write_to_output(predefine[value]);
                        continue;
                    }
                    if (addr == value)
                    {
                        stringstream s ;
                        s << "@v_" << (addr - 16) << endl;
                        write_to_output(s.str());
                        addr = addr + 1;
                        continue;
                    }
                    if (addr > value && value > 15)
                    {
                        stringstream s ;
                        s << "@v_" << (value - 16) << endl;
                        write_to_output(s.str());
                        continue;
                    }  
                } 
            }
            out_a_instr(value);
        }
    }
}

/*****************        DOWN TO HERE         ******************/


// main program
int main(int argc,char **argv)
{
    // initialise tokeniser
    next_token() ;
    // is the first command line argument "N", ie Numeric Output ?
    if ( argc > 1 && string(argv[1]) == "N" )
    {
        // YES, output all A-instructions in numeric form
        disassemble_no_symbols(parse_program()) ;
    }
    else
    {
        // NO, output A-instructions in symbolic form where possible
        disassemble_symbols(parse_program()) ;
    }

    // flush the output and any errors
    print_output() ;
    print_errors() ;
}

