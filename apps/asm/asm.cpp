#include <stdio.h>
#include <regex>
#include <iostream>
#include <map>
#include "opcodes.h"

enum Segment {Data, Text};  

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
    const std::regex assign_deref_re = std::regex("r(\\d+) = \\*r(\\d+)(\\+\\d+)?");
    const std::regex load_re = std::regex("r(\\d+) = (\\w+)");
    const std::regex store_re = std::regex("(\\w+) = r(\\d+)");
    const std::regex store_reg_re = std::regex("\\*r(\\d+)(\\+\\d+)? = r(\\d+)");
    const std::regex jmp_reg_re = std::regex("jmp r(\\d+)");
    const std::regex jmp_re = std::regex("jmp(\\..+)? (\\S+)");
    const std::regex jal_reg_re = std::regex("jal r(\\d+) r(\\d+)(\\+d+)?");
    const std::regex jal_re = std::regex("jal r(\\d+) (\\S+)");
    const std::regex int_re = std::regex("int (\\d+)");
    const std::regex call_re = std::regex("call (\\w+)");

    const std::regex set_cr_re = std::regex("cr(\\d+) = r(\\d+)");
    const std::regex read_cr_re = std::regex("r(\\d+) = cr(\\d+)");

    const std::regex include_re = std::regex("\\.include ([\\w\\./]+)");
    const std::regex org_re = std::regex("\\.org (\\d+)");

    const std::regex data_int_re = std::regex("int (\\w+) = (\\d+)");
    const std::regex data_char_re = std::regex("char (\\w+) = \"(.*)\"");
    const std::regex data_array_re = std::regex("char (\\w+)\\[(\\d+)\\]");

    std::map<std::string, unsigned> labels;
    //std::map<std::string, unsigned> symbols; // name, address

    unsigned PC = 0;
    //unsigned DP = 0; // data pointer

    uint8_t code[16 * 1024];
    uint8_t data[16 * 1024];

    unsigned base_address = 0;

    Segment segment = Data;

    struct Label {
        std::string name;
        unsigned address;
    };

    std::vector<Label> unk_labels;

    FILE *file;
public:
    void parse(const std::string &filename)
    {
        file = fopen(filename.c_str(), "rt");
        if(!file)
        {
            std::cerr << "error: file not found: " << filename << std::endl;
            return;
        }
        char *line = NULL;
        size_t len;

        while(!feof(file))
        {
            int res = getline(&line, &len, file);
            if(res != -1)
                parseLine(line);
        }

        fclose(file);
        file = 0;
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

            *(unsigned *)(code + label.address) |= labels[label.name] << 12;
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
        //else if(std::regex_match(line, match, label_re) && match.size() > 1)
        //    symbols[match.str(1)] = DP;
        else
            return false;

        return true;
    }

    void createIntVariable(const std::string &name, int value)
    {
        *(int *)(code + PC) = value;
        addLabel(name);
        PC += 4;
    }

    void createArrayVariable(const std::string &name, int size)
    {
        addLabel(name);
        PC += size;
    }

    void createCharVariable(const std::string &name, const std::string &value)
    {
        addLabel(name);
        for(unsigned i = 0; i < value.size(); i++)
        {
            *(int *)(code + PC) = value[i];
            PC++;
        }

        //code[PC] = 0;
        //PC++;
    }

    bool parseTextSegment(const std::string &line)
    {
        std::smatch match;
        if(std::regex_match(line, match, label_re) && match.size() > 1)
        {
            addLabel(match.str(1));
            return true;
        }

        std::cout << (PC + base_address) << "   ";

        if(std::regex_match(line, match, add_re) && match.size() > 1)
            arith(ADD, "+", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, addi_re) && match.size() > 1)
            arithi(ADDI, "+", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, sub_re) && match.size() > 1)
            arith(SUB, "-", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, subi_re) && match.size() > 1)
            arithi(SUBI, "-", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, mul_re) && match.size() > 1)
            arith(MUL, "*", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, muli_re) && match.size() > 1)
            arithi(MULI, "*", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, shri_re) && match.size() > 1)
            arithi(SHRI, ">>", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, shli_re) && match.size() > 1)
            arithi(SHLI, "<<", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, and_re) && match.size() > 1)
            arithi(AND, "&", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, andi_re) && match.size() > 1)
            arithi(ANDI, "&", match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, or_re) && match.size() > 1)
            arithi(OR, "|", match.str(1), match.str(2), match.str(3));
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
        else if(std::regex_match(line, match, assign_deref_re) && match.size() > 1)
            assign_deref(match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, set_cr_re) && match.size() > 1)
            system(SYSTEM_CR_SET, match.str(2), match.str(1));
        else if(std::regex_match(line, match, read_cr_re) && match.size() > 1)
            system(SYSTEM_CR_READ, match.str(2), match.str(1));
        else if(std::regex_match(line, match, load_re) && match.size() > 1)
            load(match.str(1), match.str(2));
        else if(std::regex_match(line, match, store_reg_re) && match.size() > 1)
            storer(match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, store_re) && match.size() > 1)
            store(match.str(1), match.str(2));
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
        else if(line == "syscall")
            system(SYSTEM_CALL, "0", "0");
        else if(line == "sysret")
            system(SYSTEM_RETURN, "0", "0");
        else if(std::regex_match(line, match, include_re) && match.size() > 1)
            include(match.str(1));
        else if(std::regex_match(line, match, org_re) && match.size() > 1)
            org(match.str(1));
        else
            return false;

        return true;
    }

    void addLabel(const std::string &label)
    {
        if(labels.find(label) == labels.end())
            labels[label] = PC + base_address;
        else
            std::cerr << "error: duplicated label '" << label << "'" << std::endl; 
    }

    void assign_imm(const std::string &dst, const std::string &imm)
    {
        std::cout << "r" << dst << " = " << imm << std::endl;

        unsigned value;
        if(imm.size() == 3 && imm[0] == '\'')
            value = imm[1];
        else
            value = std::stoi(imm);

        encodeC(MOVI, std::stoi(dst), value);
    }

    void assign_reg(const std::string &dst, const std::string &src)
    {
        std::cout << "r" << dst << " = r" << src << std::endl;
        encodeA(MOVR, std::stoi(dst), std::stoi(src), 0);
    }

    void assign_ref(const std::string &dst, const std::string &label)
    {
        unsigned address = resolveLabel(label);
        std::cout << "r" << dst << " = " << address << std::endl;
        encodeC(MOVI, std::stoi(dst), address);
    }

    void assign_deref(const std::string &dst, const std::string &src, const std::string &off)
    {
        int offset = 0;
        std::cout << "r" << dst << " = *r" << src << off << std::endl;
        if(off != "")
            offset = std::stoi(off);
        encodeB(LOADR, std::stoi(dst), std::stoi(src), offset);
    }

    void load(const std::string &dst, const std::string &label)
    {
        unsigned address = resolveLabel(label);
        std::cout << "r" << dst << " = [" << address << "]" <<  std::endl;
        encodeC(LOAD, std::stoi(dst), address);
    }

    void store(const std::string &src, const std::string &label)
    {
        unsigned address = resolveLabel(label);
        std::cout << "[" << address << "] = r" << src << std::endl;
        encodeC(STORE, std::stoi(src), address);
    }

    void storer(const std::string &dst, const std::string &off, const std::string &src)
    {
        int offset = 0;
        std::cout << "*r" << dst << off << " = r" << src << off << std::endl;
        if(off != "")
            offset = std::stoi(off);
        encodeB(STORER, std::stoi(dst), std::stoi(src), offset);
    }

    void arith(unsigned opcode, const std::string &op, const std::string &dst, const std::string &src0, const std::string &src1)
    {
        std::cout << "r" << dst << " = r" << src0 << " " << op << " r" << src1 << std::endl;
        encodeA(opcode, std::stoi(dst), std::stoi(src0), std::stoi(src1));
    }

    void arithi(unsigned opcode, const std::string &op, const std::string &dst, const std::string &src, const std::string &imm)
    {
        std::cout << "r" << dst << " = r" << src << " " << op << " " << imm << std::endl;
        int value;
        if(imm.size() == 3 && imm[0] == '\'')
            value = imm[1];
        else
            value = std::stoi(imm);
        encodeB(opcode, std::stoi(dst), std::stoi(src), value);
    }

    void inc(const std::string &dst, const std::string &imm)
    {
        std::cout << "r" << dst << " += " << imm << std::endl;
        encodeC(INC, std::stoi(dst), std::stoi(imm));
    }

    void jmp(const std::string &cond, const std::string &label)
    {
        unsigned address = resolveLabel(label);
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

        encodeC(JMP, cond_n, address);

        std::cout << "jmp " << cond << " " << address << std::endl;
    }

    void jmp_reg(const std::string &reg)
    {
        std::cout << "jmp r" << reg << std::endl;
        encodeC(JMPR, std::stoi(reg), 0);
    }

    void jal(const std::string &dst, const std::string &label)
    {
        unsigned address;
        if(labels.find(label) == labels.end())
        {
            address = 0;
            unk_labels.emplace_back(Label{label, PC + 2});
        }
        else
            address = labels[label];

        encodeC(JAL, std::stoi(dst), address);

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
        std::cout << "int " << int_n << std::endl;
        encodeC(INT, std::stoi(int_n), 0);
    }

    void system(unsigned fun, const std::string &src, const std::string &dst)
    {
        std::cout << "SYSTEM " << fun << " " << src << " " << dst << std::endl;
        encodeA(SYSTEM, fun, std::stoi(src), std::stoi(dst));
    }

    // | opcode | dst | src0 | src1 | unused |
    //      7      5      5      5      10
    void encodeA(uint8_t opcode, uint8_t dst, uint8_t src0, uint8_t src1)
    {
        PC += PC & 3; // align
        unsigned inst = opcode | (dst << 7) | (src0 << 12) | (src1 << 17);
        *(unsigned *)(code + PC) = inst;
        PC += 4;
    }

    // | opcode | dst | src | immediate |
    //      7      5     5       15
    void encodeB(uint8_t opcode, uint8_t dst, uint8_t src, unsigned imm)
    {
        PC += PC & 3; // align
        unsigned inst = opcode | (dst << 7) | (src << 12) | ((imm & 0x7FFF) << 17);
        *(unsigned *)(code + PC) = inst;
        PC += 4;
    }

    // | opcode | dst | immediate |
    //      7      5       20
    void encodeC(uint8_t opcode, uint8_t dst, unsigned imm)
    {
        PC += PC & 3; // align
        unsigned inst = opcode | (dst << 7) | ((imm & 0xFFFFF) << 12);
        *(unsigned *)(code + PC) = inst;
        PC += 4;
    }

    void include(const std::string &filename)
    {
        parse(filename);
    }

    void org(const std::string &base)
    {
        base_address = std::stoi(base);
    }

    unsigned resolveLabel(const std::string &label)
    {
        unsigned address;
        if(labels.find(label) == labels.end())
        {
            address = 0;
            unk_labels.emplace_back(Label{label, PC});
        }
        else
            address = labels[label];

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

    char *line = NULL;
    size_t len;

    while(!feof(file))
    {
        int res = getline(&line, &len, file);
        if(res != -1)
            parser.parseLine(line);
    }

    parser.resolveUnknownLabels();

    parser.dump(argv[2]);

    return 0;
}
