#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <bitset>
#include <cmath>

using namespace std;

const string FILENAME = "test";
const string EXTENSION = ".txt";
const int BUFSIZE = 8;
const int BUFBYTES = BUFSIZE >> 3;

template <typename T, typename P> unordered_map<T, P> swap(const unordered_map<P,T>& dict) {
    unordered_map<T,P> res;
    for (const auto& p: dict) res[p.second] = p.first;
    return res;
} 


unordered_map<char,int> fileParse(string filename) {
    unordered_map<char, int> count;
    try
    {
        fstream input = fstream(filename);
        if(!input){
            input.close();
            throw runtime_error("Could not open file: " + filename);
        }
        string line;
        while(getline(input, line)){
            for(char &c: line) 
                count[c]++;
            if (!input.eof()) count['\n']++;
        } 
        input.close();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    } 
    return count;
}

unordered_map<char, string> produceCodes(vector<pair<char, int>> sortedFreq){
    int N = sortedFreq.size();
    if (N == 0) return {};
    unordered_map<char, string> encode;
    string builder = "";
    int c = 0;
    for(int i = 0; i < N; i++){
        pair<char, int>& p = sortedFreq[i];
        encode[p.first] = builder+"0";
        builder+="1";
    } 
    encode[(char)3] = builder;
    return encode;
}

void encode(string inputFileName, string outputFileName, unordered_map<char, string>& codes) {
    ifstream input(inputFileName);
    if (!input) {
        cerr << "Could not open  " << inputFileName << endl;
        return;
    }
    ofstream output(outputFileName, ios::binary);

    string builder = "";
    string line;

    while(getline(input, line)) {
        if (!input.eof())
            line += '\n';
        for(char c: line) {
            string current = codes[c];
            if (current.length() + builder.length() > BUFSIZE) {
                int size = builder.length();
                if (size < BUFSIZE) {
                    builder += current.substr(0, BUFSIZE - size);
                    current = current.substr(BUFSIZE - size);
                }

                bitset<BUFSIZE> bits(builder);
                int m = 0;
                unsigned char* buffer = new unsigned char[BUFBYTES]{};
                for(int i = 0; m < BUFBYTES; i++){
                    if (i == 8) {
                        i = 0;
                        m++;
                    }
                    if (bits[i]){
                        buffer[m] |= 1 << (i);
                    }
                }

                output.write(reinterpret_cast<const char*>(buffer), BUFBYTES);
                builder = current;
            } else {
                builder += current;
            }
        }
    }
    
    for(int i = builder.length(); i < BUFSIZE; i++) {
        builder+="1";
    }
    cout << builder << endl;
    cout << builder.length() << endl;
    int m = 0;
    bitset<BUFSIZE> bits(builder);
    unsigned char* buffer = new unsigned char[BUFBYTES]{};
    
    for(int i = 0; i < BUFSIZE; i++){
        int mod = i % 8;
        if (!mod) m++;
        cout << bits[i];
        if (bits[i]){
            buffer[m] |= 1 << (mod);
        }
    }
    //for (int i = 0; i < BUFBYTES; i++) cout << buffer[i];
    output.write(reinterpret_cast<const char*>(buffer), BUFBYTES);
    output.close();
    input.close();
    return;
}

void decode(string inputFileName, string outputFileName, unordered_map<string, char>& decodes) {
    string builder = "";
    ifstream input(inputFileName, ios::binary);
    if (!input) {
        cerr << "Could not open file: " << inputFileName << endl;
        return;
    }
    ofstream output(outputFileName);

    char c;
    while(input >> c) {
        for(int i = 7; i >= 0; i--) {
            if (c >> i & 1 == 1) {
                builder+="1";
            }else {
                output << decodes[builder+"0"];
                builder = "";
            }
        }
    }
    output.close();
    input.close();
    return;
}

int main() {
    unordered_map<char, int> count = fileParse(FILENAME+EXTENSION);
    vector<pair<char, int>> sorted(count.begin(), count.end());
    sort(sorted.begin(), sorted.end(), [](const pair<char, int>& a, const pair<char, int>& b) {
        return a.second > b.second;
    });

    unordered_map<char, string> codes = produceCodes(sorted);
    for(const pair<const char, string>& p: codes) cout << p.first << " " << p.second << endl;

    encode(FILENAME+EXTENSION, FILENAME+".bin", codes);

    unordered_map<string, char> decodes = swap(codes);

    decode(FILENAME+".bin", "HUFF"+FILENAME+".txt", decodes);

    return 0;
}