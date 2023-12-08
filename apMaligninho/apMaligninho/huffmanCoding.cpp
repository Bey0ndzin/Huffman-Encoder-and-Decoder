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
HuffmanNode* readHuffmanTree(ifstream& inFile) {
    char bit;
    inFile.get(bit);

    if (bit == '1') {
        char data;
        inFile.get(data);
        return new HuffmanNode(data, 0);
    }
    else {
        HuffmanNode* internalNode = new HuffmanNode('$', 0);
        internalNode->left = readHuffmanTree(inFile);
        internalNode->right = readHuffmanTree(inFile);
        return internalNode;
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
        buffer.clear();  // Limpar o buffer ap�s escrever o �ltimo byte
    }

    inFile.close();
    outFile.close();
}

void freeHuffmanTree(HuffmanNode* node) {
    if (node == nullptr) {
        return;
    }
    freeHuffmanTree(node->left);
    freeHuffmanTree(node->right);
    delete node;
}

// Fun��o para descomprimir um arquivo comprimido
void decompressFile(string compressedFile, string decompressedFile) {
    // Abrir o arquivo comprimido
    ifstream inFile(compressedFile, ios::binary);
    if (!inFile.is_open()) {
        cerr << "Erro ao abrir o arquivo comprimido." << endl;
        return;
    }

    // Ler a �rvore Huffman do arquivo comprimido
    HuffmanNode* root = readHuffmanTree(inFile);

    // Abrir o arquivo de sa�da
    ofstream outFile(decompressedFile, ios::binary);
    if (!outFile.is_open()) {
        cerr << "Erro ao criar o arquivo descomprimido." << endl;
        freeHuffmanTree(root);  // Liberar mem�ria antes de sair
        return;
    }

    HuffmanNode* currentNode = root;

    // Processar cada bit do arquivo comprimido
    char bit;
    while (inFile.get(bit)) {
        for (int i = 7; i >= 0; --i) {
            if (bit & (1 << i)) {
                currentNode = currentNode->right;
            }
            else {
                currentNode = currentNode->left;
            }

            if (currentNode->left == nullptr && currentNode->right == nullptr) {
                // Chegou a uma folha, escrever o caractere no arquivo
                outFile.put(currentNode->data);
                currentNode = root;  // Reiniciar a busca na �rvore
            }
        }
    }

    // Fechar os arquivos e liberar a mem�ria
    inFile.close();
    outFile.close();
    freeHuffmanTree(root);
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
