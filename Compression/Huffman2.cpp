#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <bitset>
#include <cmath>
#include <queue>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string.h>

#define CHAR unsigned char

using namespace std;


const string FILENAME = "Words";
const string EXTENSION = ".txt";
const int BUFFSIZE = 1024*64*8;
const int BUFFBYTES = BUFFSIZE;


struct Node {
    int freq;
    Node* left;
    Node* right;
    CHAR val;
    Node(CHAR c) : freq(1), left(nullptr), right(nullptr), val(c) {}
    Node(int f, Node* l, Node* r) : freq(f), left(l), right(r), val(255) {}
};

template <typename T, typename P> unordered_map<T, P> swap(const unordered_map<P,T>& dict) {
    unordered_map<T,P> res;
    for (const auto& p: dict) res[p.second] = p.first;
    return res;
} 

void newfileParse(string filename, vector<Node*>& frequency) {
    FILE* input = fopen(filename.c_str(), "rb");
    CHAR buffer[BUFFSIZE]{};
    if(!input){
        cerr << "Could not open file: " << filename << '\n';
        exit(1);
    }
    int buffsize;
    while ((buffsize = fread(&buffer, 1, BUFFSIZE, input)) && (buffsize > 0)) { 
        for (int i = 0; i < buffsize; i++) {
            CHAR c = buffer[i];
            if (frequency[c] == nullptr) {
                frequency[c] = new Node(c);
            } else {
                frequency[c]->freq++;
            }
        }
    }
    fclose(input);
}

Node* makeTree(vector<Node*>& count) {
    struct CompareNodes {
        bool operator()(const Node* a, const Node* b) const {
            return a->freq > b->freq; // Min-heap behavior
        }
    };

    priority_queue<Node*, vector<Node*>, CompareNodes> pq;
    
    for (auto& v: count) {
        if (v != nullptr) {
            pq.push(v);
        }
    }

    while(pq.size() != 1) {
        Node* right = pq.top();
        pq.pop();
        Node* left = pq.top();
        pq.pop();
        Node* node = new Node(left->freq+right->freq, left, right);
        pq.push(node);
    }
    return pq.top();
}

void parseTree(Node* root, unordered_map<CHAR, string>& codes, string acc) {
    if (!root) return;
    if (root->val != 255) {
        codes[root->val] = acc;
    }else {
        parseTree(root->left, codes, acc+"0");
        parseTree(root->right, codes, acc+"1");
    }
    //delete root;
}

void encode(string inputFilename, string outputFilename, unordered_map<CHAR, string>& codes) {
    FILE* output = fopen(outputFilename.c_str(), "wb");
    FILE* input = fopen(inputFilename.c_str(), "rb");
    if (!input) {
        cerr << "Could not open file: " << inputFilename << endl;
        exit(1);
    }
       
    CHAR buffer[BUFFSIZE]{};
    CHAR writeBuffer[BUFFSIZE]{};  // Buffer to store bits being written
    int bytesRead;
    int bit = 7;
    int byte = 0;
    while ((bytesRead = fread(buffer, 1, BUFFSIZE, input)) && bytesRead > 0) {
        for (int i = 0; i < bytesRead; i++) {
            const string& s = codes[buffer[i]];
            //cout << s << endl;
            for (const char& c : s) {
                writeBuffer[byte] |= (c == '1') << bit;
                bit--;
                if (bit == -1) {
                    bit = 7;
                    byte++;
                    if (byte >= BUFFSIZE) {
                        fwrite(writeBuffer, 1, BUFFSIZE, output);
                        byte = 0;
                        memset(writeBuffer, 0, BUFFSIZE);
                    }
                }
            }
        }
    }

    // Handle the final part where we may not have filled the entire buffer
    if (byte > 0 || bit < 7) {
        fwrite(writeBuffer, 1, byte + (bit < 7 ? 1 : 0), output);  // Write the remaining part of the buffer
    }

    fclose(input);
    fclose(output);
}

void decode(string inputFilename, string outputFilename, const unordered_map<string, CHAR>& decodes, Node* root) {
    FILE* input = fopen(inputFilename.c_str(), "rb");
    FILE* output = fopen(outputFilename.c_str(), "wb");
    if (!input) {
        cerr << "Could not open file: " << inputFilename << endl;
        exit(1);
    }
    int bytesRead;
    CHAR c;
    Node* node = root;
    vector<CHAR> buffer;
    while((bytesRead = fread(&c, 1, 1, input)) && bytesRead > 0) {
        for(int i = 7; i >= 0; i--) {
            if (c >> i & 1) {
                node = node->right;
            } else {
                node = node->left;
            }
            if (!node->left) {
                buffer.push_back(node->val);
                node = root;
                if (buffer.size() == BUFFSIZE) { 
                    fwrite(buffer.data(), 1, BUFFSIZE, output);
                    buffer.clear();
                }
            }
                
        }
    }
    fwrite(buffer.data(), 1, buffer.size(), output);
    fclose(output);
    fclose(input);

}
 
int main() {
    auto start = chrono::high_resolution_clock::now(); 
    
    vector<Node*> count = vector<Node*>(256, nullptr);
    newfileParse(FILENAME+EXTENSION, count);
    
    Node* HuffmanRoot = makeTree(count);

    unordered_map<CHAR, string> codes;
    parseTree(HuffmanRoot, codes, "");
    // 6 secs to encode and 3 seconds to decode
    // 0.3 for all other functions total
    encode(FILENAME+EXTENSION, FILENAME+".bin", codes);

    unordered_map<string, CHAR> decodes = swap(codes);

    decode(FILENAME+".bin", FILENAME+".decoded"+".txt", decodes, HuffmanRoot);

    auto end = chrono::high_resolution_clock::now(); 

    chrono::duration<double> elapsed = end - start; // Calculate elapsed time
    cout << "Execution time: " << elapsed.count() << " seconds" << endl;    

    return 0;
}

