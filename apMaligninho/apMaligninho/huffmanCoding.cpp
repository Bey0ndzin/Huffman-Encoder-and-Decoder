#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <map>
#include <bitset>

using namespace std;

// Estrutura para representar um nó na árvore Huffman
struct HuffmanNode {
    unsigned char data;
    int frequency;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(unsigned char data, int frequency) : data(data), frequency(frequency), left(nullptr), right(nullptr) {}
};

// Comparador para a fila de prioridade (usada para construir a árvore Huffman)
struct CompareNodes {
    bool operator()(HuffmanNode* lhs, HuffmanNode* rhs) const {
        return lhs->frequency > rhs->frequency;
    }
};

// Tabela para mapear caracteres para códigos binários
map<unsigned char, string> huffmanCodes;

// Função para construir a árvore Huffman
HuffmanNode* buildHuffmanTree(map<unsigned char, int>& frequencies) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, CompareNodes> pq;

    for (auto& entry : frequencies) {
        pq.push(new HuffmanNode(entry.first, entry.second));
    }

    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();

        HuffmanNode* newNode = new HuffmanNode(0, left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;

        pq.push(newNode);
    }

    return pq.top();
}

// Função para gerar códigos Huffman recursivamente
void generateHuffmanCodes(HuffmanNode* root, string code) {
    if (root == nullptr) {
        return;
    }

    if (root->left == nullptr && root->right == nullptr) {
        huffmanCodes[root->data] = code;
    }

    generateHuffmanCodes(root->left, code + "0");
    generateHuffmanCodes(root->right, code + "1");
}

// Função para escrever a árvore Huffman em um arquivo
void writeHuffmanTree(ofstream& outFile, HuffmanNode* root) {
    if (root == nullptr) {
        return;
    }

    if (root->left == nullptr && root->right == nullptr) {
        outFile.put('1');
        outFile.put(root->data);
    }
    else {
        outFile.put('0');
        writeHuffmanTree(outFile, root->left);
        writeHuffmanTree(outFile, root->right);
    }
}

// Função para ler a árvore Huffman de um arquivo comprimido
HuffmanNode* readHuffmanTree(ifstream& inFile) {
    char bit;
    inFile.get(bit);

    if (bit == '1') {
        char data;
        inFile.get(data);
        return new HuffmanNode(data, 0);
    }
    else {
        HuffmanNode* internalNode = new HuffmanNode(0, 0);
        internalNode->left = readHuffmanTree(inFile);
        internalNode->right = readHuffmanTree(inFile);
        return internalNode;
    }
}

void freeHuffmanTree(HuffmanNode* node) {
    if (node == nullptr) {
        return;
    }
    freeHuffmanTree(node->left);
    freeHuffmanTree(node->right);
    delete node;
}

// Função para comprimir um arquivo usando a árvore Huffman
void compressFile(string inputFile, string outputFile) {
    // Ler o arquivo de entrada e contar as frequências dos bytes
    ifstream inFile(inputFile, ios::binary);
    map<unsigned char, int> frequencies;

    unsigned char byte;
    while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        frequencies[byte]++;
    }
    inFile.close();

    // Construir a árvore Huffman
    HuffmanNode* root = buildHuffmanTree(frequencies);

    // Gerar códigos Huffman
    generateHuffmanCodes(root, "");

    // Escrever a árvore Huffman no arquivo de saída
    ofstream outFile(outputFile, ios::binary);
    writeHuffmanTree(outFile, root);
    outFile.close();

    // Abrir novamente o arquivo de entrada para compressão real
    inFile.open(inputFile, ios::binary);

    // Escrever códigos Huffman comprimidos no arquivo de saída
    outFile.open(outputFile, ios::app | ios::binary);

    string buffer = "";
    while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        buffer += huffmanCodes[byte];
        while (buffer.length() >= 8) {
            bitset<8> bits(buffer.substr(0, 8));
            outFile.put(static_cast<unsigned char>(bits.to_ulong()));
            buffer = buffer.substr(8);
        }
    }

    // Escrever os últimos bits que sobraram
    while (buffer.length() % 8 != 0) {
        buffer += "0";
    }

    while (!buffer.empty()) {
        bitset<8> bits(buffer.substr(0, 8));
        outFile.put(static_cast<unsigned char>(bits.to_ulong()));
        buffer = buffer.substr(8);
    }

    inFile.close();
    outFile.close();

    // Liberar memória da árvore Huffman
    freeHuffmanTree(root);
}

// Função para descomprimir um arquivo comprimido
void decompressFile(string compressedFile, string decompressedFile) {
    // Abrir o arquivo comprimido
    ifstream inFile(compressedFile, ios::binary);
    if (!inFile.is_open()) {
        cerr << "Erro ao abrir o arquivo comprimido." << endl;
        return;
    }

    // Ler a árvore Huffman do arquivo comprimido
    HuffmanNode* root = readHuffmanTree(inFile);

    // Abrir o arquivo de saída
    ofstream outFile(decompressedFile, ios::binary);
    if (!outFile.is_open()) {
        cerr << "Erro ao criar o arquivo descomprimido." << endl;
        freeHuffmanTree(root);  // Liberar memória antes de sair
        inFile.close();
        return;
    }

    HuffmanNode* currentNode = root;

    // Processar cada byte do arquivo comprimido
    unsigned char byte;
    while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        for (int i = 7; i >= 0; --i) {
            if (byte & (1 << i)) {
                currentNode = currentNode->right;
            }
            else {
                currentNode = currentNode->left;
            }

            if (currentNode->left == nullptr && currentNode->right == nullptr) {
                // Chegou a uma folha, escrever o byte no arquivo
                outFile.put(currentNode->data);
                currentNode = root;  // Reiniciar a busca na árvore
            }
        }
    }

    // Verificar se chegou ao final do arquivo
    if (inFile.eof()) {
        // Realizar qualquer operação adicional necessária ao final do arquivo
    }

    // Fechar os arquivos e liberar a memória
    inFile.close();
    outFile.close();
    freeHuffmanTree(root);
}

int main() {
    string inputFile = "testes/Bloxstrap.exe";
    string compressedFile = "testes/compressed4.pcb";
    string decompressedFile = "testes/decompressed4.exe";

    // Comprimir o arquivo
    compressFile(inputFile, compressedFile);

    // Descomprimir o arquivo comprimido
    decompressFile(compressedFile, decompressedFile);

    return 0;
}
