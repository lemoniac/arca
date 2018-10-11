#include <stdio.h>
#include <regex>
#include <iostream>
#include <deque>
#include <map>
#include "opcodes.h"
#include "struct.h"

enum Segment {Data, Text};


#define ALIGN(n) (n + 1) & ~1

class Parser {

    #define NUMBER "-?\\d+|\\'.\\'"

    const std::regex label_re = std::regex("(\\S+):");
    const std::regex add_re = std::regex("r(\\d+) = r(\\d+) \\+ r(\\d+)");
    const std::regex addi_re = std::regex("r(\\d+) = r(\\d+) \\+ (" NUMBER ")");
    const std::regex sub_re = std::regex("r(\\d+) = r(\\d+) \\- r(\\d+)");
    const std::regex subi_re = std::regex("r(\\d+) = r(\\d+) \\- (" NUMBER ")");
    const std::regex mul_re = std::regex("r(\\d+) = r(\\d+) \\* r(\\d+)");
    const std::regex muli_re = std::regex("r(\\d+) = r(\\d+) \\* (" NUMBER ")");
    const std::regex shri_re = std::regex("r(\\d+) = r(\\d+) >> (\\d+)");
    const std::regex shli_re = std::regex("r(\\d+) = r(\\d+) << (" NUMBER ")");
    const std::regex and_re = std::regex("r(\\d+) = r(\\d+) & r(\\d+)");
    const std::regex andi_re = std::regex("r(\\d+) = r(\\d+) & (" NUMBER ")");
    const std::regex or_re = std::regex("r(\\d+) = r(\\d+) \\| r(\\d+)");
    const std::regex ori_re = std::regex("r(\\d+) = r(\\d+) \\| (" NUMBER ")");
    const std::regex inc_re = std::regex("r(\\d+) \\+= (\\d+)");
    const std::regex assign_imm_re = std::regex("r(\\d+) = (" NUMBER ")");
    const std::regex assign_reg_re = std::regex("r(\\d+) = r(\\d+)");
    const std::regex assign_ref_re = std::regex("r(\\d+) = &(\\w+)");
    const std::regex loadr_re = std::regex("r(\\d+) ([w,h,b])?= \\*r(\\d+)([\\+|-]\\d+)?");
    const std::regex loadr_struct_re = std::regex("r(\\d+) ([w,h,b])?= \\*r(\\d+)\\.([\\w|\\.]+)");
    const std::regex store_reg_re = std::regex("\\*r(\\d+)([\\+|-]\\d+)? ([w,h,b])?= r(\\d+)");
    const std::regex storer_struct_re = std::regex("\\*r(\\d+)\\.([\\w|\\.]+) ([w,h,b])?= r(\\d+)");
    const std::regex jmp_reg_re = std::regex("jmp r(\\d+)");
    const std::regex jmp_re = std::regex("jmp(\\..+)? (\\S+)");
    const std::regex jal_reg_re = std::regex("jal r(\\d+) r(\\d+)(\\+d+)?");
    const std::regex jal_re = std::regex("jal r(\\d+) (\\S+)");
    const std::regex int_re = std::regex("int (\\d+)");
    const std::regex call_re = std::regex("call (\\w+)");

    const std::regex push_re = std::regex("push r(\\d+)");
    const std::regex pop_re = std::regex("pop r(\\d+)");

    const std::regex set_cr_re = std::regex("cr(\\d+) = r(\\d+)");
    const std::regex read_cr_re = std::regex("r(\\d+) = cr(\\d+)");

    const std::regex include_re = std::regex("\\.include ([\\w\\./]+)");
    const std::regex org_re = std::regex("\\.org (\\d+)");

    const std::regex struct_re = std::regex("\\.struct (\\w+)");
    const std::regex def_int_re = std::regex("int (\\w+)");
    const std::regex def_char_re = std::regex("char (\\w+)(\\[(\\d+)\\])?");
    const std::regex def_uint16_re = std::regex("uint16 (\\w+)");

    const std::regex data_int_re = std::regex("int (\\w+) = (\\d+)");
    const std::regex data_char_re = std::regex("char (\\w+) = \"(.*)\"");
    const std::regex data_array_re = std::regex("char (\\w+)\\[(\\d+)\\]");
    const std::regex struct_def_re = std::regex("struct (\\w+) (\\w+)");

    struct Symbol {
        Symbol(unsigned address, Type type, const std::string &structName):
            address(address), type(type), structName(structName)
        {}

        Symbol() {}
    
        unsigned address;
        Type type;
        std::string structName;
    };

    std::map<std::string, Symbol> labels;
    //std::map<std::string, unsigned> symbols; // name, address

    unsigned PC = 0;
    //unsigned DP = 0; // data pointer

    uint8_t code[16 * 1024];
    uint8_t data[16 * 1024];

    unsigned base_address = 0;

    Segment segment = Data;

    std::map<std::string, Struct> structs;

    struct Label {
        std::string name;
        unsigned address;
        unsigned align;
    };

    std::vector<Label> unk_labels;

    std::deque<FILE *> fileStack;
    FILE *file;

    bool useShortInstructions = true;

public:
    void parse(const std::string &filename)
    {
        file = fopen(filename.c_str(), "rt");
        if(!file)
        {
            std::cerr << "error: file not found: " << filename << std::endl;
            return;
        }
        fileStack.push_back(file);

        char *line = NULL;
        size_t len;

        while(!feof(file))
        {
            int res = getline(&line, &len, file);
            if(res != -1)
                parseLine(line);
        }

        fclose(file);
        fileStack.pop_back();
        if(!fileStack.empty())
            file = fileStack.back();
    }

    void parseLine(const char *line)
    {
        std::string l = cleanLine(line);
        if(l.empty())
            return;
/*
        if(l == ".data")
        {
            segment = Data;
            return;
        }

        if(l == ".text")
        {
            segment = Text;
            return;
        }

        if(segment == Data)
            parseDataSegment(l);
        else
            parseTextSegment(l);
            */

        if(!parseDataSegment(l))
        {
            if(!parseTextSegment(l))
            {
                std::cout << "???" << std::endl;
                std::cout << l << std::endl;
            }
        }
    }

    void resolveUnknownLabels()
    {
        for(const auto &label: unk_labels)
        {
            if(labels.find(label.name) == labels.end())
            {
                std::cerr << "error: unknown label " << label.name << std::endl;
                continue;
            }

            *(unsigned *)(code + label.address) |= labels[label.name].address << (12 - label.align);
        }
    }

    void dump(const char *filename)
    {
        FILE *file = fopen(filename, "wb");
        //fwrite(&PC, 4, 1, file);
        //fwrite(&DP, 4, 1, file);

        fwrite(code, PC, 1, file);
        //fwrite(data, DP, 1, file);
    }

protected:
    std::string cleanLine(const char *line)
    {
        std::string str = line;

        auto start = str.begin();
        while(start != str.end())
        {
            if(*start != ' ')
                break;
            start++;
        }
        std::string l = std::string(start, str.end() - 1);
        if(l.size() == 0 || l[0] == '#')
            return "";

        return l;
    }


    bool parseDataSegment(const std::string &line)
    {
        std::smatch match;
        if(std::regex_match(line, match, data_int_re) && match.size() > 1)
            createIntVariable(match.str(1), std::stoi(match.str(2)));
        else if(std::regex_match(line, match, data_char_re) && match.size() > 1)
            createCharVariable(match.str(1), match.str(2));
        else if(std::regex_match(line, match, data_array_re) && match.size() > 1)
            createArrayVariable(match.str(1), std::stoi(match.str(2)));
        else if(std::regex_match(line, match, include_re) && match.size() > 1)
            include(match.str(1));
        else if(std::regex_match(line, match, struct_def_re) && match.size() > 1)
            createStruct(match.str(1), match.str(2));
        //else if(std::regex_match(line, match, label_re) && match.size() > 1)
        //    symbols[match.str(1)] = DP;
        else
            return false;

        return true;
    }

    void createIntVariable(const std::string &name, int value)
    {
        *(int *)(code + PC) = value;
        addLabel(Type::Int, name);
        PC += 4;
    }

    void createArrayVariable(const std::string &name, int size)
    {
        addLabel(Type::Char, name);
        PC += size;
    }

    void createStruct(const std::string &structName, const std::string &name)
    {
        addLabel(Type::Struct, name, structName);
        PC += structs[structName].getSize();
    }

    void createCharVariable(const std::string &name, const std::string &value)
    {
        addLabel(Type::Char, name);
        for(unsigned i = 0; i < value.size(); i++)
        {
            *(int *)(code + PC) = value[i];
            PC++;
        }

        code[PC] = 0;
        PC++;
    }

    bool parseTextSegment(const std::string &line)
    {
        std::smatch match;
        if(std::regex_match(line, match, label_re) && match.size() > 1)
        {
            PC = ALIGN(PC);
            addLabel(Type::Address, match.str(1));
            return true;
        }

        std::cout << (PC + base_address) << "   ";

        if(std::regex_match(line, match, add_re) && match.size() > 1)
            arith(ALU_ADD, "+", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, addi_re) && match.size() > 1)
            arithi(ADDI, "+", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, sub_re) && match.size() > 1)
            arith(ALU_SUB, "-", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, subi_re) && match.size() > 1)
            arithi(SUBI, "-", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, mul_re) && match.size() > 1)
            arith(ALU_MUL, "*", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, muli_re) && match.size() > 1)
            arithi(MULI, "*", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, shri_re) && match.size() > 1)
            arithi(SHRI, ">>", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, shli_re) && match.size() > 1)
            arithi(SHLI, "<<", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, and_re) && match.size() > 1)
            arith(ALU_AND, "&", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, andi_re) && match.size() > 1)
            arithi(ANDI, "&", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, or_re) && match.size() > 1)
            arith(ALU_OR, "|", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, ori_re) && match.size() > 1)
            arithi(ORI, "|", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, inc_re) && match.size() > 1)
            inc(match.str(1), match.str(2));
        else if(std::regex_match(line, match, assign_reg_re) && match.size() > 1)
            assign_reg(match.str(1), match.str(2));
        else if(std::regex_match(line, match, assign_imm_re) && match.size() > 1)
            assign_imm(match.str(1), match.str(2));
        else if(std::regex_match(line, match, assign_ref_re) && match.size() > 1)
            assign_ref(match.str(1), match.str(2));
        else if(std::regex_match(line, match, loadr_re) && match.size() > 1)
            loadr(match.str(1), match.str(2), match.str(3), match.str(4));
        else if(std::regex_match(line, match, set_cr_re) && match.size() > 1)
            system(SYSTEM_CR_SET, match.str(2), match.str(1));
        else if(std::regex_match(line, match, read_cr_re) && match.size() > 1)
            system(SYSTEM_CR_READ, match.str(2), match.str(1));
        else if(std::regex_match(line, match, loadr_struct_re) && match.size() > 1)
            loadr_struct(match.str(1), match.str(2), match.str(3), match.str(4));
        else if(std::regex_match(line, match, store_reg_re) && match.size() > 1)
            storer(match.str(1), match.str(2), match.str(3), match.str(4));
        else if(std::regex_match(line, match, storer_struct_re) && match.size() > 1)
            storer_struct(match.str(1), match.str(2), match.str(3), match.str(4));
        else if(std::regex_match(line, match, jmp_reg_re) && match.size() > 1)
            jmp_reg(match.str(1));
        else if(std::regex_match(line, match, jmp_re) && match.size() > 1)
            jmp(match.str(1), match.str(2));
        else if(std::regex_match(line, match, jal_re) && match.size() > 1)
            jal(match.str(1), match.str(2));
        else if(std::regex_match(line, match, jal_reg_re) && match.size() > 1)
            jalr(match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, call_re) && match.size() > 1)
            jal("15", match.str(1));
        else if(std::regex_match(line, match, int_re) && match.size() > 1)
            interrupt(match.str(1));
        else if(line == "ret")
            jmp_reg("15");
        else if(std::regex_match(line, match, push_re) && match.size() > 1)
            push(match.str(1));
        else if(std::regex_match(line, match, pop_re) && match.size() > 1)
            pop(match.str(1));
        else if(line == "syscall")
            system(SYSTEM_CALL, "0", "0");
        else if(line == "sysret")
            system(SYSTEM_RETURN, "0", "0");
        else if(std::regex_match(line, match, include_re) && match.size() > 1)
            include(match.str(1));
        else if(std::regex_match(line, match, org_re) && match.size() > 1)
            org(match.str(1));
        else if(std::regex_match(line, match, struct_re) && match.size() > 1)
            parseStruct(match.str(1));
        else
            return false;

        return true;
    }

    void addLabel(Type type, const std::string &label, const std::string &structName = "")
    {
        if(labels.find(label) == labels.end())
            labels[label] = Symbol(PC + base_address, type, structName);
        else
            std::cerr << "error: duplicated label '" << label << "'" << std::endl; 
    }

    void assign_imm(const std::string &dst, const std::string &imm)
    {
        std::cout << "r" << dst << " = " << imm << std::endl;
        int rd = std::stoi(dst);

        unsigned value;
        if(imm.size() == 3 && imm[0] == '\'')
            value = imm[1];
        else
            value = std::stoi(imm);

        if(useShortInstructions && value < 16 && rd < 16)
            encodeShortB(SHORT_MOVI, rd, value);
        else
            encodeC(MOVI, rd, value);
    }

    void assign_reg(const std::string &dst, const std::string &src)
    {
        int rd = std::stoi(dst);
        int rs = std::stoi(src);

        std::cout << "r" << dst << " = r" << src << std::endl;
        if(useShortInstructions && rd < 16 && rs < 16)
            encodeShortA(SHORT_MOVR, rd, rs);
        else
            encodeA(MOVR, rd, rs, 0);
    }

    void assign_ref(const std::string &dst, const std::string &label)
    {
        unsigned address = resolveLabel(label);
        std::cout << "r" << dst << " = " << address << std::endl;
        encodeC(MOVI, std::stoi(dst), address);
    }

    unsigned getLoadWidth(const std::string &width)
    {
        if(width == "" || width == "w")
            return MEM_LOADW;
        else if(width == "h")
            return MEM_LOADHU;
        else if(width == "b")
            return MEM_LOADBU;
        return 0;
    }

    void loadr(const std::string &dst, const std::string &width, const std::string &src, const std::string &off)
    {
        int offset = 0;
        std::cout << "r" << dst << " = *r" << src << off << std::endl;
        if(off != "")
            offset = std::stoi(off);

        encodeD(LOADR, std::stoi(dst), std::stoi(src), getLoadWidth(width), offset);
    }

    void loadr_struct(const std::string &dst, const std::string &width, const std::string &src, const std::string &struct_field)
    {
        int offset = getFieldOffset(struct_field);
        if(offset < 0)
            return;

        std::cout << "r" << dst << " = *r" << src << "+" << offset << std::endl;

        encodeD(LOADR, std::stoi(dst), std::stoi(src), getLoadWidth(width), offset);
    }

    unsigned getStoreWidth(const std::string &width)
    {
        if(width == "" || width == "w")
            return MEM_STOREW;
        else if(width == "h")
            return MEM_STOREH;
        else if(width == "b")
            return MEM_STOREB;
        return 0;
    }

    void storer(const std::string &dst, const std::string &off, const std::string &width, const std::string &src)
    {
        int offset = 0;
        std::cout << "*r" << dst << off << " = r" << src << std::endl;
        if(off != "")
            offset = std::stoi(off);
        encodeD(STORER, std::stoi(dst), std::stoi(src), getStoreWidth(width), offset);
    }

    void storer_struct(const std::string &dst, const std::string &struct_field, const std::string &width, const std::string &src)
    {
        int offset = getFieldOffset(struct_field);
        if(offset < 0)
            return;

        std::cout << "*r" << dst << "+" << offset << " = r" << src << std::endl;

        encodeD(STORER, std::stoi(dst), std::stoi(src), getStoreWidth(width), offset);
    }

    void arith(unsigned opcode, const std::string &op, const std::string &dst, const std::string &src0, const std::string &src1)
    {
        int rd = std::stoi(dst);
        int rs = std::stoi(src0);
        int rt = std::stoi(src1);

        std::cout << "r" << dst << " = r" << src0 << " " << op << " r" << src1 << std::endl;
        if(useShortInstructions && opcode == ALU_SUB && rd == 0 && rs < 16 && rd < 16)
            encodeShortA(SHORT_CMPR, rs, rt);
        else if(useShortInstructions && rd == rs && rd < 16 && rt < 16)
            encodeShortA(SHORT_ALUR + opcode, rd, rt);
        else
            encodeA(ALU, rd, rs, rt, opcode);
    }

    void arithi(unsigned opcode, const std::string &op, const std::string &dst, const std::string &src, const std::string &imm)
    {
        int rd = std::stoi(dst);
        int rs = std::stoi(src);
        std::cout << "r" << dst << " = r" << src << " " << op << " " << imm << std::endl;
        int value;
        if(imm.size() == 3 && imm[0] == '\'')
            value = imm[1];
        else
            value = std::stoi(imm);
        if(useShortInstructions && opcode == SUBI && rd == 0 && rs < 16 && value < 16)
            encodeShortB(SHORT_CMPI, rs, value);
        else
            encodeB(opcode, rd, rs, value);
    }

    void inc(const std::string &dst, const std::string &imm)
    {
        std::cout << "r" << dst << " += " << imm << std::endl;
        encodeC(INCI, std::stoi(dst), std::stoi(imm));
    }

    void jmp(const std::string &cond, const std::string &label)
    {
        unsigned address = resolveLabel(label, 1);
        unsigned cond_n = 0;
        if(cond == "")
            cond_n = 0;
        else if(cond == ".z")
            cond_n = COND_ZERO;
        else if(cond == ".nz")
            cond_n = COND_NOTZERO;
        else if(cond == ".s")
            cond_n = COND_SIGN;
        else if(cond == ".ns")
            cond_n = COND_NOSIGN;
        else if(cond == ".lt")
            cond_n = COND_LT;
        else if(cond == ".ltu")
            cond_n = COND_LTU;
        else if(cond == ".ge")
            cond_n = COND_GE;
        else if(cond == ".geu")
            cond_n = COND_GEU;

        encodeC(JMP, cond_n, address >> 1);

        std::cout << "jmp " << cond << " " << address << std::endl;
    }

    void jmp_reg(const std::string &reg)
    {
        int rd = std::stoi(reg);
        std::cout << "jmp r" << reg << std::endl;
        if(useShortInstructions && rd < 16)
            encodeShortB(SHORT_JMPR, rd, 0);
        else
            encodeC(JMPR, rd, 0);
    }

    void jal(const std::string &dst, const std::string &label)
    {
        unsigned address;
        if(labels.find(label) == labels.end())
        {
            address = 0;
            unk_labels.emplace_back(Label{label, PC + 2, 1});
        }
        else
            address = labels[label].address;

        encodeC(JAL, std::stoi(dst), address >> 1);

        std::cout << "jal r" << dst << " " << address << std::endl;
    }

    void jalr(const std::string &dst, const std::string &src, const std::string &off)
    {
        int offset = 0;
        if(off != "")
            offset = std::stoi(off);

        encodeB(JALR, std::stoi(dst), std::stoi(src), offset);

        std::cout << "jalr r" << dst << " r" << src << off << std::endl;
    }

    void interrupt(const std::string &int_n)
    {
        int n = std::stoi(int_n);
        std::cout << "int " << int_n << std::endl;
        if(useShortInstructions)
            encodeShortC(SHORT_INT, n);
        else
            encodeC(INT, n, 0);
    }

    void push(const std::string &reg)
    {
        std::cout << "*r14 = r" << reg << std::endl;
        encodeD(STORER, 14, std::stoi(reg), MEM_STOREW, 0);
        std::cout << "r14 -= 4" << std::endl;
        encodeA(SUBI, 14, 14, 4);
    }

    void pop(const std::string &reg)
    {
        std::cout << "r14 += 4" << std::endl;
        encodeA(ADDI, 14, 14, 4);
        std::cout << "r" << reg << " = *r14" << std::endl;
        encodeD(LOADR, std::stoi(reg), 14, MEM_LOADW, 0);
    }

    void system(unsigned fun, const std::string &src, const std::string &dst)
    {
        std::cout << "SYSTEM " << fun << " " << src << " " << dst << std::endl;
        encodeA(SYSTEM, fun, std::stoi(src), std::stoi(dst));
    }

    // | opcode | dst | src0 | src1 | unused |
    //      7      5      5      5      10
    void encodeA(uint8_t opcode, uint8_t dst, uint8_t src0, uint8_t src1, unsigned other = 0)
    {
        PC = ALIGN(PC);
        unsigned inst = opcode | (dst << 7) | (src0 << 12) | (src1 << 17) | (other << 22);
        *(unsigned *)(code + PC) = inst;
        PC += 4;
    }

    // | opcode | dst | src | immediate |
    //      7      5     5       15
    void encodeB(uint8_t opcode, uint8_t dst, uint8_t src, unsigned imm)
    {
        PC = ALIGN(PC);
        unsigned inst = opcode | (dst << 7) | (src << 12) | ((imm & 0x7FFF) << 17);
        *(unsigned *)(code + PC) = inst;
        PC += 4;
    }

    // | opcode | dst | immediate |
    //      7      5       20
    void encodeC(uint8_t opcode, uint8_t dst, unsigned imm)
    {
        PC = ALIGN(PC);
        unsigned inst = opcode | (dst << 7) | ((imm & 0xFFFFF) << 12);
        *(unsigned *)(code + PC) = inst;
        PC += 4;
    }

    // | opcode | dst | src | fun | immediate |
    //      7      5     5     3       12
    void encodeD(uint8_t opcode, uint8_t dst, uint8_t src, uint8_t fun, unsigned imm)
    {
        PC = ALIGN(PC);
        unsigned inst = opcode | (dst << 7) | (src << 12) | (fun << 17) | ((imm & 0xFFF) << 20);
        *(unsigned *)(code + PC) = inst;
        PC += 4;
    }

    // | opcode | dst | src | unused |
    //      7      4     4       1
    void encodeShortA(uint8_t opcode, uint8_t dst, uint8_t src)
    {
        PC = ALIGN(PC);
        uint16_t inst = opcode | (dst << 7) | (src << 11);
        *(uint16_t *)(code + PC) = inst;
        PC += 2;
    }

    // | opcode | dst | immediate |
    //      7      4        5
    void encodeShortB(uint8_t opcode, uint8_t dst, unsigned imm)
    {
        PC = ALIGN(PC);
        uint16_t inst = opcode | (dst << 7) | ((imm & 0x1F) << 11);
        *(uint16_t *)(code + PC) = inst;
        PC += 2;
    }

    // | opcode | immediate |
    //      7         9
    void encodeShortC(uint8_t opcode, unsigned imm)
    {
        PC = ALIGN(PC);
        uint16_t inst = opcode | (imm << 7);
        *(uint16_t *)(code + PC) = inst;
        PC += 2;
    }

    void include(const std::string &filename)
    {
        parse(filename);
    }

    void org(const std::string &base)
    {
        base_address = std::stoi(base);
    }

    int parseArraySize(const std::string &n)
    {
        if(n == "")
            return 1;
        return std::stoi(n);
    }

    void parseStructLine(const std::string &l, Struct &str)
    {
        std::smatch match;
        if(std::regex_match(l, match, def_char_re) && match.size() > 1)
            str.member.emplace_back(match.str(1), Type::Char, parseArraySize(match.str(3)));
        else if(std::regex_match(l, match, def_int_re) && match.size() > 1)
            str.member.emplace_back(match.str(1), Type::Int, 1);
        else if(std::regex_match(l, match, def_uint16_re) && match.size() > 1)
            str.member.emplace_back(match.str(1), Type::UInt16, 1);
    }

    void parseStruct(const std::string &name)
    {
        Struct str;

        char *line = NULL;
        size_t len;

        while(!feof(file))
        {
            int res = getline(&line, &len, file);
            if(res != -1)
            {
                std::string l = cleanLine(line);
                if(l.empty())
                    continue;
                if(l == ".endstruct")
                    break;
                parseStructLine(l, str);
            }
        }
        
        structs[name] = str;
    }

    int getFieldOffset(const std::string &struct_field)
    {
        size_t dot = struct_field.find(".");
        if(dot == std::string::npos)
        {
            std::cerr << "expected struct.field" << std::endl;
            return -1;
        }
        std::string structName = std::string(struct_field.begin(), struct_field.begin() + dot);
        std::string field = std::string(struct_field.begin() + dot + 1, struct_field.end());

        if(structs.find(structName) == structs.end())
        {
            std::cerr << "unknown struct " << structName << std::endl;
            return -1;
        }

        int offset = structs[structName].getOffset(field);
        if(offset == -1)
        {
            std::cerr << "unknown field '" << field << "' in struct " << structName << std::endl;
            return -1;
        }

        return offset;
    }


    unsigned resolveLabel(const std::string &label, unsigned align = 0)
    {
        unsigned address;
        if(labels.find(label) == labels.end())
        {
            address = 0;
            unk_labels.emplace_back(Label{label, PC, align});
        }
        else
            address = labels[label].address;

        return address;
    }
};


int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "asm infile outfile\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "rt");
    if(!file)
    {
        fprintf(stderr, "error loading file %s\n", argv[1]);
        return 1;
    }

    Parser parser;
    parser.parse(argv[1]);

    parser.resolveUnknownLabels();

    parser.dump(argv[2]);

    return 0;
}
