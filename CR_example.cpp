#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include "CR.hpp"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " filename" << endl;
        exit(1);
    }

    string filename = argv[1];

    try {
        CR rm = CR(filename, true);

    } catch(exception &e) {
        cerr << "Error: " << e.what() << endl;
        exit(1);
    }

    return 0;
}
