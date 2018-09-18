#include <stdio.h>
#include <regex>
#include <iostream>
#include <map>
#include "opcodes.h"

enum Segment {Data, Text};  

class Parser {

    const std::regex label_re = std::regex("(\\S+):");
    const std::regex add_re = std::regex("r(\\d+) = r(\\d+) \\+ r(\\d+)");
    const std::regex sub_re = std::regex("r(\\d+) = r(\\d+) \\- r(\\d+)");
    const std::regex assign_imm_re = std::regex("r(\\d+) = (\\d+)");
    const std::regex assign_reg_re = std::regex("r(\\d+) = r(\\d+)");
    const std::regex assign_ref_re = std::regex("r(\\d+) = &(\\w+)");
    const std::regex assign_deref_re = std::regex("r(\\d+) = \\*r(\\d+)");
    const std::regex load_re = std::regex("r(\\d+) = (\\w+)");
    const std::regex store_re = std::regex("(\\w+) = r(\\d+)");
    const std::regex store_reg_re = std::regex("\\*r(\\d+) = r(\\d+)");
    const std::regex jmp_reg_re = std::regex("jmp r(\\d+)");
    const std::regex jmp_re = std::regex("jmp(\\..+)? (\\S+)");
    const std::regex jal_re = std::regex("jal r(\\d+) (\\S+)");
    const std::regex int_re = std::regex("int (\\d+)");
    const std::regex call_re = std::regex("call (\\w+)");

    const std::regex include_re = std::regex("\\.include ([\\w\\./]+)");

    const std::regex data_int_re = std::regex("int (\\w+) = (\\d+)");
    const std::regex data_char_re = std::regex("char (\\w+) = \"(.*)\"");

    std::map<std::string, unsigned> labels;
    std::map<std::string, unsigned> symbols; // name, address

    unsigned PC = 0;
    unsigned DP = 0; // data pointer

    uint8_t code[16 * 1024];
    uint8_t data[16 * 1024];

    Segment segment = Data;

    struct Label {
        std::string name;
        unsigned address;
    };

    std::vector<Label> unk_labels;

public:
    void parse(const std::string &filename)
    {
        FILE *file = fopen(filename.c_str(), "rt");
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
                parseLine(std::string(line));
        }

        fclose(file);
    }

    void parseLine(const std::string &line)
    {
        auto start = line.begin();
        while(start != line.end())
        {
            if(*start != ' ')
                break;
            start++;
        }
        std::string l = std::string(start, line.end() - 1);
        if(l.size() == 0 || l[0] == '#')
            return;

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

            *(int16_t *)(code + label.address) = labels[label.name];
        }
    }

    void dump(const char *filename)
    {
        FILE *file = fopen(filename, "wb");
        fwrite(&PC, 4, 1, file);
        fwrite(&DP, 4, 1, file);

        fwrite(code, PC, 1, file);
        fwrite(data, DP, 1, file);
    }

protected:
    void parseDataSegment(const std::string &line)
    {
        std::smatch match;
        if(std::regex_match(line, match, data_int_re) && match.size() > 1)
            createIntVariable(match.str(1), std::stoi(match.str(2)));
        else if(std::regex_match(line, match, data_char_re) && match.size() > 1)
            createCharVariable(match.str(1), match.str(2));
        else if(std::regex_match(line, match, include_re) && match.size() > 1)
            include(match.str(1));
        else if(std::regex_match(line, match, label_re) && match.size() > 1)
            symbols[match.str(1)] = DP;

    }

    void createIntVariable(const std::string &name, int value)
    {
        *(int *)(data + DP) = value;
        symbols[name] = DP;
        DP += 4;
    }

    void createCharVariable(const std::string &name, const std::string &value)
    {
        symbols[name] = DP;
        for(unsigned i = 0; i < value.size(); i++)
        {
            *(int *)(data + DP) = value[i];
            DP++;
        }
    }

    void parseTextSegment(const std::string &line)
    {
        std::smatch match;
        if(std::regex_match(line, match, label_re) && match.size() > 1)
        {
            addLabel(match.str(1));
            return;
        }

        std::cout << PC << "   ";

        if(std::regex_match(line, match, add_re) && match.size() > 1)
            add(match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, sub_re) && match.size() > 1)
            sub(match.str(1), match.str(2), match.str(3));
        else if(std::regex_match(line, match, assign_reg_re) && match.size() > 1)
            assign_reg(match.str(1), match.str(2));
        else if(std::regex_match(line, match, assign_imm_re) && match.size() > 1)
            assign_imm(match.str(1), match.str(2));
        else if(std::regex_match(line, match, assign_ref_re) && match.size() > 1)
            assign_ref(match.str(1), match.str(2));
        else if(std::regex_match(line, match, assign_deref_re) && match.size() > 1)
            assign_deref(match.str(1), match.str(2));
        else if(std::regex_match(line, match, load_re) && match.size() > 1)
            load(match.str(1), match.str(2));
        else if(std::regex_match(line, match, store_reg_re) && match.size() > 1)
            storer(match.str(1), match.str(2));
        else if(std::regex_match(line, match, store_re) && match.size() > 1)
            store(match.str(1), match.str(2));
        else if(std::regex_match(line, match, jmp_reg_re) && match.size() > 1)
            jmp_reg(match.str(1));
        else if(std::regex_match(line, match, jmp_re) && match.size() > 1)
            jmp(match.str(1), match.str(2));
        else if(std::regex_match(line, match, jal_re) && match.size() > 1)
            jal(match.str(1), match.str(2));
        else if(std::regex_match(line, match, call_re) && match.size() > 1)
            jal("15", match.str(1));
        else if(std::regex_match(line, match, int_re) && match.size() > 1)
            interrupt(match.str(1));
        else if(line == "ret")
            jmp_reg("15");
        else if(std::regex_match(line, match, include_re) && match.size() > 1)
            include(match.str(1));
        else
        {
            std::cout << "???" << std::endl;
            std::cout << line << std::endl;
        }
    }

    void addLabel(const std::string &label)
    {
        if(labels.find(label) == labels.end())
            labels[label] = PC;
        else
            std::cerr << "error: duplicated label '" << label << "'" << std::endl; 
    }

    void assign_imm(const std::string &dst, const std::string &val)
    {
        std::cout << "r" << dst << " = " << val << std::endl;
        encode(MOVI, std::stoi(dst), std::stoi(val));
    }

    void assign_reg(const std::string &dst, const std::string &src)
    {
        std::cout << "r" << dst << " = r" << src << std::endl;
        encode(MOVR, std::stoi(dst), std::stoi(src), 0);
    }

    void assign_ref(const std::string &dst, const std::string &src)
    {
        unsigned address = symbols[src];
        std::cout << "r" << dst << " = " << address << std::endl;
        encode(MOVI, std::stoi(dst), address);
    }

    void assign_deref(const std::string &dst, const std::string &src)
    {
        unsigned address = symbols[src];
        std::cout << "r" << dst << " = *r" << src << std::endl;
        encode(LOADR, std::stoi(dst), std::stoi(src), 0);
    }

    void load(const std::string &dst, const std::string &src)
    {
        unsigned address = symbols[src];
        std::cout << "r" << dst << " = [" << address << "]" <<  std::endl;
        encode(LOAD, std::stoi(dst), address);
    }

    void store(const std::string &dst, const std::string &src)
    {
        unsigned address = symbols[dst];
        std::cout << "[" << address << "] = r" << src << std::endl;
        encode(STORE, std::stoi(src), address);
    }

    void storer(const std::string &dst, const std::string &src)
    {
        encode(STORER, std::stoi(dst), std::stoi(src), 0);
    }

    void add(const std::string &dst, const std::string &src0, const std::string &src1)
    {
        std::cout << "r" << dst << " = r" << src0 << " + r" << src1 << std::endl;
        encode(ADD, std::stoi(dst), std::stoi(src0), std::stoi(src1));
    }

    void sub(const std::string &dst, const std::string &src0, const std::string &src1)
    {
        std::cout << "r" << dst << " = r" << src0 << " - r" << src1 << std::endl;
        encode(SUB, std::stoi(dst), std::stoi(src0), std::stoi(src1));
    }

    void jmp(const std::string &cond, const std::string &label)
    {
        unsigned address;
        if(labels.find(label) == labels.end())
        {
            address = 0;
            unk_labels.emplace_back(Label{label, PC + 2});
        }
        else
            address = labels[label];

        unsigned cond_n = 0;
        if(cond == "")
            cond_n = 0;
        else if(cond == ".z")
            cond_n = COND_ZERO;
        else if(cond == ".nz")
            cond_n = COND_NOTZERO;
            
        encode(JMP, cond_n, address);

        std::cout << "jmp " << cond << " " << address << std::endl;
    }

    void jmp_reg(const std::string &reg)
    {
        std::cout << "jmp r" << reg << std::endl;
        encode(JMPR, std::stoi(reg), 0, 0);
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
            
        encode(JAL, std::stoi(dst), address);

        std::cout << "jal r" << dst << " " << address << std::endl;
    }

    void interrupt(const std::string &int_n)
    {
        std::cout << "int " << int_n << std::endl;
        encode(INT, std::stoi(int_n), 0, 0);
    }


    void encode(uint8_t opcode, uint8_t dst, uint8_t src0, uint8_t src1)
    {
        code[PC] = opcode;
        code[PC+1] = dst;
        code[PC+2] = src0;
        code[PC+3] = src1;
        PC += 4;
    }

    void encode(uint8_t opcode, uint8_t dst, uint16_t imm)
    {        
        code[PC] = opcode;
        code[PC+1] = dst;
        *(int16_t *)(&code[PC+2]) = imm;

        PC += 4;
    }

    void include(const std::string &filename)
    {
        parse(filename);
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
            parser.parseLine(std::string(line));
    }

    parser.resolveUnknownLabels();

    parser.dump(argv[2]);

    return 0;
}