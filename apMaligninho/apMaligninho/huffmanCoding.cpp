#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <map>
#include <bitset>
#include <sstream>

using namespace std;

// Struct feita para servir como um nó de huffman na árvore
struct No {
    unsigned char data;
    int freq;
    No* esq;
    No* dir;

    No(unsigned char data, int freq) : data(data), freq(freq), esq(nullptr), dir(nullptr) {}
};

// Struct de Comparador para a fila de prioridade (usada para construir a árvore Huffman)
struct ComparaNo {
    bool operator()(No* noEsquerdo, No* noDireito) const {
        return noEsquerdo->freq > noDireito->freq;
    }
};

// Tabela para mapear caracteres convertendo eles em códigos binários
// Funciona igual objeto de javascript
// {"byte": "valorEmCodigo"}
map<unsigned char, string> huffmanCodes;

// Função para construir a árvore Huffman
// map de frequências
// {"byte": "frequencia"}
No* construirArvore(map<unsigned char, int>& frequencias) {
    // Fila de prioridade para guardas os nós em ordem
    priority_queue<No*, vector<No*>, ComparaNo> pq;

    // Basicamente um foreach que puxa ponteiros para cada frequência
    for (auto& entry : frequencias) {
        // Insere esses dados
        // Entry.first -> byte, Entry.second -> frequência
        pq.push(new No(entry.first, entry.second));
    }

    while (pq.size() > 1) {
        // Pega os 2 primeiros nós da fila
        No* esq = pq.top(); pq.pop();
        No* dir = pq.top(); pq.pop();

        // Cria um novo nó sem informação em bytes e com frequência igual a soma das 2 frequências anteriores
        No* newNode = new No(0, esq->freq + dir->freq);
        newNode->esq = esq;
        newNode->dir = dir;

        // "Posta" esse nó na fila
        pq.push(newNode);
    }

    return pq.top();
}

// Função para gerar códigos Huffman recursivamente
// Basicamente como ela gera os códigos:
// 1.Para cada lado (esquerda ou direita) não nulo do código ele adiciona um 0 (esquerda) ou um 1 (direita)
// 2.Depois de andar todos os lados da árvore e chegar em uma folha (sem nós filhos)
// 3.Pega o byte dessa folha e usa como "paramêtro" para setar um valor correspondente lá no map anterior
void gerarCodigos(No* raiz, string code) {
    if (raiz == nullptr) {
        return;
    }

    if (raiz->esq == nullptr && raiz->dir == nullptr) {
        huffmanCodes[raiz->data] = code;
    }

    gerarCodigos(raiz->esq, code + "0");
    gerarCodigos(raiz->dir, code + "1");
}

// Função para escrever a árvore Huffman em um arquivo
void escreveArvore(ofstream& outFile, No* raiz, const string& extOrig) {
    if (raiz == nullptr) {
        return;
    }

    // Escrever a extensão original no cabeçalho
    outFile << extOrig;

    // Caso seja uma folha ele adiciona um '1' e o byte lido
    if (raiz->esq == nullptr && raiz->dir == nullptr) {
        outFile.put('1');
        outFile.put(raiz->data);
    }
    // Se não for uma folha coloca um '0' e passa para a próxima
    else {
        outFile.put('0');
        escreveArvore(outFile, raiz->esq, extOrig);
        escreveArvore(outFile, raiz->dir, extOrig);
    }
}

// Função para ler a árvore Huffman de um arquivo comprimido
No* lerArvore(ifstream& inFile, string& extOrig) {
    // Lê a extensão original do cabeçalho
    char extBuffer[3];
    inFile.read(extBuffer, 3);
    extOrig = string(extBuffer, 3);

    char bit;
    inFile.get(bit);
    // Pega 1 único char do arquivo

    // Se esse char for 1 (logo vem um byte depois)
    if (bit == '1') {
        char data;
        inFile.get(data);
        // Lê esse byte e salva em um nó
        return new No(data, 0);
    }
    else {
        // Se for um '0' ele cria um novo nó sem dado
        No* internalNode = new No(0, 0);
        internalNode->esq = lerArvore(inFile, extOrig);
        internalNode->dir = lerArvore(inFile, extOrig);
        return internalNode;
    }
}

void liberarArvore(No* node) {
    if (node == nullptr) {
        return;
    }
    liberarArvore(node->esq);
    liberarArvore(node->dir);
    delete node;
}

// Função para comprimir um arquivo usando a árvore Huffman
void comprimir(string inputFile, string outputFile) {
    // Ler o arquivo de entrada e contar as frequências dos bytes
    ifstream inFile(inputFile, ios::binary);
    map<unsigned char, int> frequencies;

    // Lê cada byte e todas as vezes que ele aparecer soma +1 na frequência dele
    unsigned char byte;
    while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        frequencies[byte]++;
    }
    inFile.close();

    // Constrói a árvore e retorna a raiz da árvore
    No* root = construirArvore(frequencies);

    // Gera os códigos para cada byte
    gerarCodigos(root, "");

    // Obtendo a extensão original do arquivo
    string extOrig = inputFile.substr(inputFile.find_last_of('.') + 1);

    // Escreve a árvore Huffman no arquivo de saída
    ofstream outFile(outputFile, ios::binary);
    escreveArvore(outFile, root, extOrig);
    outFile.close();

    // Abrir novamente o arquivo de entrada para compressão real
    inFile.open(inputFile, ios::binary);

    // Escrever códigos Huffman comprimidos no arquivo de saída
    outFile.open(outputFile, ios::app | ios::binary);

    string buffer = "";
    while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        buffer += huffmanCodes[byte];
        // Buffer = Código
        // Ex: Buffer = 11011001
        while (buffer.length() >= 8) {
            // Enquanto ainda tiver números no buffer, ele pega de 8 em 8 números e escreve no arquivo e depois substrai
            bitset<8> bits(buffer.substr(0, 8));
            outFile.put(static_cast<unsigned char>(bits.to_ulong()));
            buffer = buffer.substr(8);
        }
    }

    // Escreve os últimos bits que sobraram
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
    liberarArvore(root);
}

// Função para descomprimir um arquivo comprimido
void descomprimir(string compressedFile, string decompressedFile) {
    // Abrir o arquivo comprimido
    ifstream inFile(compressedFile, ios::binary);
    if (!inFile.is_open()) {
        cerr << "Erro ao abrir o arquivo comprimido." << endl;
        return;
    }

    // Ler a árvore Huffman do arquivo comprimido
    string extOrig("");
    No* root = lerArvore(inFile, extOrig);

    decompressedFile += "."+extOrig;
    // Abrir o arquivo de saída
    ofstream outFile(decompressedFile, ios::binary);
    if (!outFile.is_open()) {
        cerr << "Erro ao criar o arquivo descomprimido." << endl;
        liberarArvore(root);  // Liberar memória antes de sair
        inFile.close();
        return;
    }

    No* currentNode = root;

    // Processar cada byte do arquivo comprimido
    unsigned char byte;
    while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        for (int i = 7; i >= 0; --i) {
            if (byte & (1 << i)) {
                currentNode = currentNode->dir;
            }
            else {
                currentNode = currentNode->esq;
            }

            if (currentNode->esq == nullptr && currentNode->dir == nullptr) {
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
    liberarArvore(root);
}

int main() {
    char resp = ' ';
    string nomeSemExtensao, inputName, inputDir;
    string compressedFile;
    string decompressedFile;
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

            comprimir((inputDir + "/" + inputName), compressedFile);
            break;
        case '2':
            cout << "Digite o endereco do arquivo\nEx: C:/Users/Pichau/Documents/GitHub/\n";
            cin >> inputDir;
            cout << "Digite o nome do arquivo a ser descomprimido\nEx: input\n";
            cin >> inputName;

            compressedFile = inputDir + "/" + inputName + ".pcb";

            p = inputName.find_last_of('.');
            nomeSemExtensao = inputName.substr(0, p);

            decompressedFile = inputDir + "/" + "decompressed_" + nomeSemExtensao;

            descomprimir(compressedFile, decompressedFile);
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
