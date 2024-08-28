#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <bitset>

using namespace std;

// Node structure for Huffman Tree
struct Node {
    char ch;
    int freq;
    Node *left, *right;

    Node(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

// Compare function for priority queue
struct Compare {
    bool operator()(Node* left, Node* right) {
        return left->freq > right->freq;
    }
};

// Function to build the Huffman Tree
Node* buildHuffmanTree(unordered_map<char, int>& freqMap) {
    priority_queue<Node*, vector<Node*>, Compare> pq;

    // Create a leaf node for each character and add it to the priority queue
    for (auto pair : freqMap) {
        pq.push(new Node(pair.first, pair.second));
    }

    // Iterate while size of heap doesn't become one
    while (pq.size() != 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();

        // Create a new internal node with frequency equal to the sum of the two nodes frequencies.
        Node* sum = new Node('\0', left->freq + right->freq);
        sum->left = left;
        sum->right = right;

        pq.push(sum);
    }

    // The remaining node is the root node and the tree is complete.
    return pq.top();
}

// Function to generate Huffman codes from the Huffman tree
void generateCodes(Node* root, string code, unordered_map<char, string>& huffmanCode) {
    if (!root) return;

    // Found a leaf node
    if (root->ch != '\0') {
        huffmanCode[root->ch] = code;
    }

    generateCodes(root->left, code + "0", huffmanCode);
    generateCodes(root->right, code + "1", huffmanCode);
}

// Function to write encoded data to a file
void writeEncodedFile(const string& inputFileName, const string& outputFileName, unordered_map<char, string>& huffmanCode) {
    ifstream inputFile(inputFileName, ios::binary);
    ofstream outputFile(outputFileName, ios::binary);

    string encodedData = "";
    char ch;
    while (inputFile.get(ch)) {
        encodedData += huffmanCode[ch];
    }

    // Writing encoded data to the file
    int extraBits = 8 - encodedData.size() % 8;
    for (int i = 0; i < extraBits; ++i) encodedData += "0";

    bitset<8> bits;
    for (size_t i = 0; i < encodedData.size(); i += 8) {
        bits = bitset<8>(encodedData.substr(i, 8));
        outputFile.put(static_cast<char>(bits.to_ulong()));
    }

    // Write extra bits count
    outputFile.put(extraBits);

    inputFile.close();
    outputFile.close();
}

// Function to decode encoded file using Huffman Tree
void decodeFile(const string& inputFileName, const string& outputFileName, Node* root) {
    ifstream inputFile(inputFileName, ios::binary);
    ofstream outputFile(outputFileName);

    string encodedData = "";
    char ch;
    while (inputFile.get(ch)) {
        encodedData += bitset<8>(ch).to_string();
    }

    // Get the last byte (extra bits count)
    int extraBits = encodedData.back() - '0';
    encodedData = encodedData.substr(0, encodedData.size() - 8);

    // Removing the extra bits
    if (extraBits > 0) {
        encodedData = encodedData.substr(0, encodedData.size() - extraBits);
    }

    Node* current = root;
    for (char bit : encodedData) {
        if (bit == '0') {
            current = current->left;
        } else {
            current = current->right;
        }

        if (current->ch != '\0') {
            outputFile.put(current->ch);
            current = root;
        }
    }

    inputFile.close();
    outputFile.close();
}

// Function to calculate frequency of each character in the file
unordered_map<char, int> calculateFrequency(const string& fileName) {
    unordered_map<char, int> freqMap;
    ifstream inputFile(fileName, ios::binary);
    char ch;
    while (inputFile.get(ch)) {
        freqMap[ch]++;
    }
    inputFile.close();
    return freqMap;
}

int main() {
    string inputFileName = "input.txt";
    string compressedFileName = "compressed.bin";
    string decompressedFileName = "decompressed.txt";

    // Step 1: Calculate frequency of each character in the input file
    unordered_map<char, int> freqMap = calculateFrequency(inputFileName);

    // Step 2: Build the Huffman Tree
    Node* root = buildHuffmanTree(freqMap);

    // Step 3: Generate Huffman codes from the Huffman Tree
    unordered_map<char, string> huffmanCode;
    generateCodes(root, "", huffmanCode);

    // Step 4: Write the encoded data to a compressed file
    writeEncodedFile(inputFileName, compressedFileName, huffmanCode);

    // Step 5: Decode the compressed file back to original content
    decodeFile(compressedFileName, decompressedFileName, root);

    cout << "Compression and decompression completed." << endl;

    return 0;
}
