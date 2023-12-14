#include "arvore.h"

using namespace std;

// Fun��o para comprimir um arquivo usando a �rvore Huffman
void comprimir(string inputFile, string outputFile) {
    Arvore a;
    // Ler o arquivo de entrada e contar as frequ�ncias dos bytes
    ifstream inFile(inputFile, ios::binary);
    map<unsigned char, int> frequencias;

    // L� cada byte e todas as vezes que ele aparecer soma +1 na frequ�ncia dele
    unsigned char byte;
    while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        frequencias[byte]++;
    }
    inFile.close();

    // Constr�i a �rvore e retorna a raiz da �rvore
    No* root = a.construirArvore(frequencias);

    // Gera os c�digos para cada byte
    a.gerarCodigos(root, "");

    // Obtendo a extens�o original do arquivo
    string extOrig = inputFile.substr(inputFile.find_last_of('.') + 1);

    // Escreve a �rvore Huffman no arquivo de sa�da
    ofstream outFile(outputFile, ios::binary);
    a.escreveArvore(outFile, root, extOrig);
    outFile.close();

    // Abrir novamente o arquivo de entrada para compress�o real
    inFile.open(inputFile, ios::binary);

    // Escrever c�digos Huffman comprimidos no arquivo de sa�da
    outFile.open(outputFile, ios::app | ios::binary);

    string buffer = "";
    while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        buffer += a.huffmanCodes[byte];
        // Buffer = C�digo
        // Ex: Buffer = 11011001
        while (buffer.length() >= 8) {
            // Enquanto ainda tiver n�meros no buffer, ele pega de 8 em 8 n�meros e escreve no arquivo e depois subtrai do buffer os 8 n�meros
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
    a.liberarArvore(root);
}

// Fun��o para descomprimir um arquivo comprimido
void descomprimir(string compressedFile, string decompressedFile) {
    Arvore a;
    // Abrir o arquivo comprimido
    ifstream inFile(compressedFile, ios::binary);
    if (!inFile.is_open()) {
        cerr << "Erro ao abrir o arquivo comprimido." << endl;
        return;
    }

    // Ler a �rvore Huffman do arquivo comprimido
    string extOrig("");
    No* root = a.lerArvore(inFile, extOrig);

    decompressedFile += "."+extOrig;
    // Abrir o arquivo de sa�da
    ofstream outFile(decompressedFile, ios::binary);
    if (!outFile.is_open()) {
        cerr << "Erro ao criar o arquivo descomprimido." << endl;
        a.liberarArvore(root);  // Liberar mem�ria antes de sair
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
    if (inFile.eof()) {}

    // Fechar os arquivos e liberar a mem�ria
    inFile.close();
    outFile.close();
    a.liberarArvore(root);
}

int main() {
    char resp = 'L';
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
            cout << "Digite o endereco do arquivo\nEx: C:/Users/Pichau/Documents/GitHub\n";
            cin >> inputDir;
            cout << "Digite o nome do arquivo\nEx: input.txt\n";
            cin >> inputName;

            p = inputName.find_last_of('.');
            nomeSemExtensao = inputName.substr(0, p);
            compressedFile = inputDir + "/" + nomeSemExtensao + ".pcb";

            comprimir((inputDir + "/" + inputName), compressedFile);
            break;
        case '2':
            cout << "Digite o endereco do arquivo\nEx: C:/Users/Pichau/Documents/GitHub\n";
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
