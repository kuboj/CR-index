#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

class HashIndex {
    public:
        HashIndex(string reads_path, int query_length = 13, bool save_reads = false);
        vector<int> find_indexes(const string& s);
        vector<string> find_reads(const string& s);

    private:
        unordered_map<string, vector<int>> data;
        vector<string> reads;
        int query_length;
        bool save_reads;

        bool process(const string& read, int read_id);
};
