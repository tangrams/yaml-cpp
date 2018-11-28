#include <fstream>
#include <iostream>
#include <vector>

#include "yaml-cpp/yaml.h"
#include "yaml-cpp/proto/protobuf.h"


void parse(std::istream& input, std::ostream& output) {
    try {
        YAML::Node doc = YAML::Load(input);

        YAML::proto::Node out = YAML::Protobuf::Dump(doc);

        out.SerializeToOstream(&output);
    
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
        } else {
            fout.open("dump.pbf");
        }
        parse(fin, fout);

    } else {
        parse(std::cin, std::cout);
    }
}

