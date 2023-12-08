#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <map>
#include <bitset>

using namespace std;

// Estrutura para representar um n� na �rvore Huffman
struct HuffmanNode {
    char data;
    int frequency;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char data, int frequency) : data(data), frequency(frequency), left(nullptr), right(nullptr) {}
};

// Comparador para a fila de prioridade (usada para construir a �rvore Huffman)
struct CompareNodes {
    bool operator()(HuffmanNode* lhs, HuffmanNode* rhs) const {
        return lhs->frequency > rhs->frequency;
    }
};

// Tabela para mapear caracteres para c�digos bin�rios
map<char, string> huffmanCodes;

// Fun��o para construir a �rvore Huffman
HuffmanNode* buildHuffmanTree(map<char, int>& frequencies) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, CompareNodes> pq;

    for (auto& entry : frequencies) {
        pq.push(new HuffmanNode(entry.first, entry.second));
    }

    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();

        HuffmanNode* newNode = new HuffmanNode('$', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;

        pq.push(newNode);
    }

    return pq.top();
}

// Fun��o para gerar c�digos Huffman recursivamente
void generateHuffmanCodes(HuffmanNode* root, string code) {
    if (root == nullptr) {
        return;
    }

    if (root->data != '$') {
        huffmanCodes[root->data] = code;
    }

    generateHuffmanCodes(root->left, code + "0");
    generateHuffmanCodes(root->right, code + "1");
}

// Fun��o para escrever a �rvore Huffman em um arquivo
void writeHuffmanTree(ofstream& outFile, HuffmanNode* root) {
    if (root == nullptr) {
        return;
    }

    if (root->data != '$') {
        outFile << '1' << root->data;
    }
    else {
        outFile << '0';
        writeHuffmanTree(outFile, root->left);
        writeHuffmanTree(outFile, root->right);
    }
}
// Fun��o para ler a �rvore Huffman de um arquivo comprimido
void readHuffmanTree(ifstream& inFile, HuffmanNode* root) {
    char bit;
    inFile.get(bit);

    if (bit == '1') {
        char data;
        inFile.get(data);
        root->data = data;
    }
    else {
        root->left = new HuffmanNode('$', 0);
        root->right = new HuffmanNode('$', 0);
        readHuffmanTree(inFile, root->left);
        readHuffmanTree(inFile, root->right);
    }
}


// Fun��o para comprimir um arquivo usando a �rvore Huffman
void compressFile(string inputFile, string outputFile) {
    // Ler o arquivo de entrada e contar as frequ�ncias dos caracteres
    ifstream inFile(inputFile, ios::binary);
    map<char, int> frequencies;

    char ch;
    while (inFile.get(ch)) {
        frequencies[ch]++;
    }

    inFile.close();

    // Construir a �rvore Huffman
    HuffmanNode* root = buildHuffmanTree(frequencies);

    // Gerar c�digos Huffman
    generateHuffmanCodes(root, "");

    // Escrever a �rvore Huffman no arquivo de sa�da
    ofstream outFile(outputFile, ios::binary);
    writeHuffmanTree(outFile, root);
    outFile.close();

    // Abrir novamente o arquivo de entrada para compress�o real
    inFile.open(inputFile, ios::binary);

    // Escrever c�digos Huffman comprimidos no arquivo de sa�da
    outFile.open(outputFile, ios::app | ios::binary);

    string buffer = "";
    while (inFile.get(ch)) {
        buffer += huffmanCodes[ch];
        while (buffer.length() >= 8) {
            bitset<8> byte(buffer.substr(0, 8));
            outFile.put(byte.to_ulong());
            buffer = buffer.substr(8);
        }
    }

    // Escrever os �ltimos bits que sobraram
    if (!buffer.empty()) {
        bitset<8> byte(buffer);
        outFile.put(byte.to_ulong());
    }

    inFile.close();
    outFile.close();
}

// Fun��o para descomprimir um arquivo comprimido
// Fun��o para descomprimir um arquivo comprimido
void decompressFile(string compressedFile, string decompressedFile) {
    // Ler a �rvore Huffman do arquivo comprimido
    ifstream inFile(compressedFile, ios::binary);
    HuffmanNode* root = nullptr;

    char bit;
    inFile.get(bit);
    while (bit == '0' || bit == '1') {
        if (bit == '1') {
            char data;
            inFile.get(data);
            root = new HuffmanNode(data, 0);
        }
        else {
            root = new HuffmanNode('$', 0);
            root->left = new HuffmanNode('$', 0);
            root->right = new HuffmanNode('$', 0);
            readHuffmanTree(inFile, root->left);
            readHuffmanTree(inFile, root->right);
        }
        inFile.get(bit);
    }

    // Descomprimir o restante do arquivo
    ofstream outFile(decompressedFile, ios::binary);
    HuffmanNode* currentNode = root;

    // Processar cada bit do arquivo comprimido
    while (inFile.get(bit)) {
        for (int i = 7; i >= 0; --i) {
            if (bit & (1 << i)) {
                currentNode = currentNode->right;
            }
            else {
                currentNode = currentNode->left;
            }

            // Se atingir uma folha, escrever o caractere no arquivo de sa�da
            if (currentNode->left == nullptr && currentNode->right == nullptr) {
                outFile.put(currentNode->data);
                currentNode = root;
            }
        }
    }

    inFile.close();
    outFile.close();
}


int main() {
    string inputFile = "testes/input.txt";
    string compressedFile = "testes/compressed.pcb";
    string decompressedFile = "testes/decompressed.txt";

    // Comprimir o arquivo
    compressFile(inputFile, compressedFile);

    // Descomprimir o arquivo comprimido
    decompressFile(compressedFile, decompressedFile);

    return 0;
}
