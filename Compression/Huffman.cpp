#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

#define FILENAME "Words"
#define EXTENSION ".txt"

unordered_map<char,int> fileParse(string filename) {
    unordered_map<char, int> count;
    try
    {
        fstream fout = fstream(filename);
        if(!fout){
            fout.close();
            throw runtime_error("Could not open file: " + filename);
        }
        char c;
        while(fout >> c) count[c]++;
        fout.close();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    } 
    return count;
}

int main(){
    unordered_map<char, int> count = string(FILENAME) + EXTENSION;
    vector<pair<char, int>> sorted(count.begin(), count.end());
    sort(sorted.begin(), sorted.end(), [](const pair<char, int>& a, const pair<char, int>& b) {
        return a.second > b.second;
    });
    unordered_map<char, string> encode;
    string builder = "";
    for(pair<char, int> p: sorted){
        encode[p.first] = builder+"0";
        builder+="1";
        c = p.first;
    } 
    encode[c] = builder;
    fstream fin = fstream("encoded.bin")


    return 0;
}