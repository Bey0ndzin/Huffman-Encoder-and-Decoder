#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <map>
#include <bitset>

using namespace std;

// Struct feita para servir como um nó de huffman na árvore
struct No {
    unsigned char data;
    int freq;
    No* left;
    No* right;

    No(unsigned char data, int freq) : data(data), freq(freq), left(nullptr), right(nullptr) {}
};

// Struct de Comparador para a fila de prioridade (usada para construir a árvore Huffman)
struct ComparaNo {
    bool operator()(No* lhs, No* rhs) const {
        return lhs->freq > rhs->freq;
    }
};

// Tabela para mapear caracteres convertendo eles em códigos binários
map<unsigned char, string> huffmanCodes;

// Função para construir a árvore Huffman
No* construirArvore(map<unsigned char, int>& frequencies) {
    priority_queue<No*, vector<No*>, ComparaNo> pq;

    for (auto& entry : frequencies) {
        pq.push(new No(entry.first, entry.second));
    }

    while (pq.size() > 1) {
        No* left = pq.top(); pq.pop();
        No* right = pq.top(); pq.pop();

        No* newNode = new No(0, left->freq + right->freq);
        newNode->left = left;
        newNode->right = right;

        pq.push(newNode);
    }

    return pq.top();
}

// Função para gerar códigos Huffman recursivamente
void generateHuffmanCodes(No* root, string code) {
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
void writeHuffmanTree(ofstream& outFile, No* root) {
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
No* readHuffmanTree(ifstream& inFile) {
    char bit;
    inFile.get(bit);

    if (bit == '1') {
        char data;
        inFile.get(data);
        return new No(data, 0);
    }
    else {
        No* internalNode = new No(0, 0);
        internalNode->left = readHuffmanTree(inFile);
        internalNode->right = readHuffmanTree(inFile);
        return internalNode;
    }
}

void freeHuffmanTree(No* node) {
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
    No* root = construirArvore(frequencies);

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
    No* root = readHuffmanTree(inFile);

    // Abrir o arquivo de saída
    ofstream outFile(decompressedFile, ios::binary);
    if (!outFile.is_open()) {
        cerr << "Erro ao criar o arquivo descomprimido." << endl;
        freeHuffmanTree(root);  // Liberar memória antes de sair
        inFile.close();
        return;
    }

    No* currentNode = root;

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
    char resp = ' ';
    string nomeSemExtensao, inputName, inputDir;
    string compressedFile;
    string decompressedFile;
    string extOrig;
    string::size_type p;
    do {
        cout << "1) Comprimir arquivo \n";
        cout << "2) Descomprimir arquivo \n";
        cout << "3) sair \n";
        cin >> resp;
        switch (resp) {
        case '1':
            cout << "Digite o endereco do arquivo\nEx: C:/Users/Pichau/Documents/GitHub/\n";
            cin >> inputDir;
            cout << "Digite o nome do arquivo\nEx: input.txt\n";
            cin >> inputName;

            p = inputName.find_last_of('.');
            nomeSemExtensao = inputName.substr(0, p);
            compressedFile = inputDir + "/" + nomeSemExtensao + ".pcb";

            compressFile((inputDir + "/" + inputName), compressedFile);
            break;
        case '2':
            cout << "Digite o endereco do arquivo\nEx: C:/Users/Pichau/Documents/GitHub/\n";
            cin >> inputDir;
            cout << "Digite o nome do arquivo a ser descomprimido\nEx: input\n";
            cin >> inputName;
            cout << "Digite a extensao original do arquivo\nEx: txt\n";
            cin >> extOrig;

            compressedFile = inputDir + "/" + inputName + ".pcb";

            p = inputName.find_last_of('.');
            nomeSemExtensao = inputName.substr(0, p);

            decompressedFile = inputDir + "/" + "decompressed_" + nomeSemExtensao + "." + extOrig;

            decompressFile(compressedFile, decompressedFile);
            break;
        case '3':
            cout << "Obrigado por utilizar o programa";
            break;
        default:
            cout << "Digite um numero válido!\n";
            break;
        }
    } while (resp != '3');

    return 0;
}
