#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <map>
#include <bitset>
#include <sstream>

using namespace std;

// Struct feita para servir como um n� de huffman na �rvore
struct No {
    unsigned char data;
    int freq;
    No* esq;
    No* dir;

    No(unsigned char data, int freq) : data(data), freq(freq), esq(nullptr), dir(nullptr) {}
};

// Struct de Comparador para a fila de prioridade (usada para construir a �rvore Huffman)
struct ComparaNo {
    bool operator()(No* noEsquerdo, No* noDireito) const {
        return noEsquerdo->freq > noDireito->freq;
    }
};

// Tabela para mapear caracteres convertendo eles em c�digos bin�rios
// Funciona igual objeto de javascript
// {"byte": "valorEmCodigo"}
map<unsigned char, string> huffmanCodes;

// Fun��o para construir a �rvore Huffman
// map de frequ�ncias
// {"byte": "frequencia"}
No* construirArvore(map<unsigned char, int>& frequencias) {
    // Fila de prioridade para guardas os n�s em ordem
    priority_queue<No*, vector<No*>, ComparaNo> pq;

    // Basicamente um foreach que puxa ponteiros para cada frequ�ncia
    for (auto& entry : frequencias) {
        // Insere esses dados
        // Entry.first -> byte, Entry.second -> frequ�ncia
        pq.push(new No(entry.first, entry.second));
    }

    while (pq.size() > 1) {
        // Pega os 2 primeiros n�s da fila
        No* esq = pq.top(); pq.pop();
        No* dir = pq.top(); pq.pop();

        // Cria um novo n� sem informa��o em bytes e com frequ�ncia igual a soma das 2 frequ�ncias anteriores
        No* newNode = new No(0, esq->freq + dir->freq);
        newNode->esq = esq;
        newNode->dir = dir;

        // "Posta" esse n� na fila
        pq.push(newNode);
    }

    return pq.top();
}

// Fun��o para gerar c�digos Huffman recursivamente
// Basicamente como ela gera os c�digos:
// 1.Para cada lado (esquerda ou direita) n�o nulo do c�digo ele adiciona um 0 (esquerda) ou um 1 (direita)
// 2.Depois de andar todos os lados da �rvore e chegar em uma folha (sem n�s filhos)
// 3.Pega o byte dessa folha e usa como "param�tro" para setar um valor correspondente l� no map anterior
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

// Fun��o para escrever a �rvore Huffman em um arquivo
void escreveArvore(ofstream& outFile, No* raiz, const string& extOrig) {
    if (raiz == nullptr) {
        return;
    }

    // Escrever a extens�o original no cabe�alho
    outFile << extOrig;

    // Caso seja uma folha ele adiciona um '1' e o byte lido
    if (raiz->esq == nullptr && raiz->dir == nullptr) {
        outFile.put('1');
        outFile.put(raiz->data);
    }
    // Se n�o for uma folha coloca um '0' e passa para a pr�xima
    else {
        outFile.put('0');
        escreveArvore(outFile, raiz->esq, extOrig);
        escreveArvore(outFile, raiz->dir, extOrig);
    }
}

// Fun��o para ler a �rvore Huffman de um arquivo comprimido
No* lerArvore(ifstream& inFile, string& extOrig) {
    // L� a extens�o original do cabe�alho
    char extBuffer[3];
    inFile.read(extBuffer, 3);
    extOrig = string(extBuffer, 3);

    char bit;
    inFile.get(bit);
    // Pega 1 �nico char do arquivo

    // Se esse char for 1 (logo vem um byte depois)
    if (bit == '1') {
        char data;
        inFile.get(data);
        // L� esse byte e salva em um n�
        return new No(data, 0);
    }
    else {
        // Se for um '0' ele cria um novo n� sem dado
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

// Fun��o para comprimir um arquivo usando a �rvore Huffman
void comprimir(string inputFile, string outputFile) {
    // Ler o arquivo de entrada e contar as frequ�ncias dos bytes
    ifstream inFile(inputFile, ios::binary);
    map<unsigned char, int> frequencies;

    // L� cada byte e todas as vezes que ele aparecer soma +1 na frequ�ncia dele
    unsigned char byte;
    while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        frequencies[byte]++;
    }
    inFile.close();

    // Constr�i a �rvore e retorna a raiz da �rvore
    No* root = construirArvore(frequencies);

    // Gera os c�digos para cada byte
    gerarCodigos(root, "");

    // Obtendo a extens�o original do arquivo
    string extOrig = inputFile.substr(inputFile.find_last_of('.') + 1);

    // Escreve a �rvore Huffman no arquivo de sa�da
    ofstream outFile(outputFile, ios::binary);
    escreveArvore(outFile, root, extOrig);
    outFile.close();

    // Abrir novamente o arquivo de entrada para compress�o real
    inFile.open(inputFile, ios::binary);

    // Escrever c�digos Huffman comprimidos no arquivo de sa�da
    outFile.open(outputFile, ios::app | ios::binary);

    string buffer = "";
    while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        buffer += huffmanCodes[byte];
        // Buffer = C�digo
        // Ex: Buffer = 11011001
        while (buffer.length() >= 8) {
            // Enquanto ainda tiver n�meros no buffer, ele pega de 8 em 8 n�meros e escreve no arquivo e depois substrai
            bitset<8> bits(buffer.substr(0, 8));
            outFile.put(static_cast<unsigned char>(bits.to_ulong()));
            buffer = buffer.substr(8);
        }
    }

    // Escreve os �ltimos bits que sobraram
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

    // Liberar mem�ria da �rvore Huffman
    liberarArvore(root);
}

// Fun��o para descomprimir um arquivo comprimido
void descomprimir(string compressedFile, string decompressedFile) {
    // Abrir o arquivo comprimido
    ifstream inFile(compressedFile, ios::binary);
    if (!inFile.is_open()) {
        cerr << "Erro ao abrir o arquivo comprimido." << endl;
        return;
    }

    // Ler a �rvore Huffman do arquivo comprimido
    string extOrig("");
    No* root = lerArvore(inFile, extOrig);

    decompressedFile += "."+extOrig;
    // Abrir o arquivo de sa�da
    ofstream outFile(decompressedFile, ios::binary);
    if (!outFile.is_open()) {
        cerr << "Erro ao criar o arquivo descomprimido." << endl;
        liberarArvore(root);  // Liberar mem�ria antes de sair
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
                currentNode = root;  // Reiniciar a busca na �rvore
            }
        }
    }

    // Verificar se chegou ao final do arquivo
    if (inFile.eof()) {
        // Realizar qualquer opera��o adicional necess�ria ao final do arquivo
    }

    // Fechar os arquivos e liberar a mem�ria
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
            cout << "Digite um numero v�lido!\n";
            break;
        }
    } while (resp != '3');

    return 0;
}
