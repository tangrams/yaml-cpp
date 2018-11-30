#include <fstream>
#include <iostream>
#include <vector>

#include "yaml-cpp/yaml.h"
#include "yaml-cpp/proto/protobuf.h"
#include "yaml-cpp/emitterstyle.h"

void parse(std::istream& input, std::ostream& output) {
    try {

        // Stop eating new lines in binary mode!!!
        input.unsetf(std::ios::skipws);

        // get its size:
        std::streampos fileSize;

        input.seekg(0, std::ios::end);
        fileSize = input.tellg();
        input.seekg(0, std::ios::beg);

        // reserve capacity
        std::vector<char> vec;
        vec.reserve(fileSize);

        // read the data:
        vec.insert(vec.begin(),
                   std::istream_iterator<char>(input),
                   std::istream_iterator<char>());
    
        YAML::Node doc = YAML::Protobuf::Load(vec.data(), vec.size());
        
        YAML::Emitter out(output);
        out.SetBoolFormat(YAML::TrueFalseBool);
        // out.SetMapFormat(YAML::Flow);
        out.SetSeqFormat(YAML::Flow);
        out << doc;

        output << "\n";
    } catch (const YAML::Exception& e) {
        std::cerr << e.what() << "\n";
    }
}

int main(int argc, char** argv) {

    if (argc > 1) {
        std::ifstream fin;
        fin.open(argv[1]);
        
        std::ofstream fout;
        if (argc > 2) {
            fout.open(argv[2]);
            parse(fin, fout);
        } else {
            parse(fin, std::cout);
        }

    } else {
        parse(std::cin, std::cout);
    }
}
