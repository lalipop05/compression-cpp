#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <bitset>

using namespace std;

const string FILENAME = "test";
const string EXTENSION = ".txt";

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

unordered_map<char, string> produceCodes(vector<pair<char, int>> sortedFreq){
    int N = sortedFreq.size();
    if (N == 0) return {};
    unordered_map<char, string> encode;
    string builder = "";
    for(int i = 0; i < N-1; i++){
        pair<char, int>& p = sortedFreq[i];
        encode[p.first] = builder+"0";
        builder+="1";
    } 
    encode[sortedFreq[N-1].first] = builder;
    return encode;
}

void encoder(string inputFileName, string outputFileName, unordered_map<char, string> codes){
    try {
        ifstream inputFileBuffer(inputFileName); 
        if (!inputFileBuffer){
            inputFileBuffer.close();
            throw runtime_error("Could not open file to write to " + inputFileName);
        }
        ofstream binaryOutput(outputFileName, ios::binary);
        string builder = "";
        string current = "";
        char c;
        while(inputFileBuffer >> c){
            current = codes[c];
            
            int size = builder.length() + current.length();
            if (size > 128) {
                builder += current.substr(0, 128-builder.length());
                current = current.substr(128-builder.length());
                bitset<128> bits(builder);
                unsigned char* buffer = new unsigned char[16]{};
                for(int i = 0; i < 128; i++){
                    if (bits[i]){
                        buffer[i/8] |= 1 << (i%8);
                    }
                }
                binaryOutput.write(reinterpret_cast<const char*>(buffer), 16);
                delete[] buffer;
                builder = current;
            } else {
                builder += current;
            }
        }
        if (builder == "") return;
        for(int i = 0; i < 128-builder.length(); i++){
            builder+="1";
        }
        bitset<128> bits(builder);
        unsigned char* buffer = new unsigned char[16]{};
        for(int i = 0; i < 128; i++){
            if (bits[i]){
                buffer[i/8] |= 1 << (i%8);
            }
        }
        binaryOutput.write(reinterpret_cast<const char*>(buffer), 16);
        delete[] buffer;
        inputFileBuffer.close();
        binaryOutput.close();
    }catch (exception& e) {
        cerr << e.what() << "\n";
        
    }
}

void decoder(string inputBinary, string outputFileName, unordered_map<string, char> decodes) {
    string builder = "";
    try {
        ifstream inputBins(inputBinary, ios::binary);
        if (!inputBins) {
            inputBins.close();
            throw runtime_error("Could not open file: " + inputBinary);
        }
        ofstream outputFile (outputFileName);
        
        unsigned char c;
        string builder;
        while(inputBins >> c){
            for(int bit = 7; bit >= 0; bit--) {
                if (((c >> bit) & 1)) builder+="1";
                else {
                    builder += "0";
                    outputFile << decodes[builder];
                    cout << decodes[builder] << endl;
                    builder = "";

                }
            }
        }

        outputFile.close();
        inputBins.close();
    } catch (exception& e) {
        cerr << e.what() << endl;
    }
}
int main(){
    unordered_map<char, int> count = fileParse(FILENAME+EXTENSION);
    vector<pair<char, int>> sorted(count.begin(), count.end());
    sort(sorted.begin(), sorted.end(), [](const pair<char, int>& a, const pair<char, int>& b) {
        return a.second > b.second;
    });
    
    unordered_map<char, string> codes = produceCodes(sorted);
    unordered_map<string, char> decodes;
    for(const pair<char, string>& p: codes) {
        cout << p.first << ":" << p.second << endl;
        decodes[p.second] = p.first;
    }

    encoder(FILENAME+EXTENSION, FILENAME+".bin", codes);

    //decoder(FILENAME+".bin", "HUFF"+FILENAME+EXTENSION, decodes);

    
    return 0;
}