#include "arvore.h"

using namespace std;

// Fun��o para construir a �rvore Huffman
// map de frequ�ncias
// {"byte": "frequencia"}
No* Arvore::construirArvore(map<unsigned char, int>& frequencias) {
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
void Arvore::gerarCodigos(No* raiz, string code) {
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
void Arvore::escreveArvore(ofstream& outFile, No* raiz, const string& extOrig) {
    if (raiz == nullptr) {
        return;
    }

    // Escrever a extens�o original no cabe�alho
    outFile << extOrig.length();
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
No* Arvore::lerArvore(ifstream& inFile, string& extOrig) {
    // L� o tamanho da extens�o original do cabe�alho
    char lenBuffer[1];
    inFile.read(lenBuffer, 1);
    extOrig = string(lenBuffer, 1);

    // L� a extens�o original do cabe�alho]
    string tam = string(lenBuffer, 1);
    char* extBuffer = new char[stoi(tam)];
    inFile.read(extBuffer, stoi(tam));
    extOrig = string(extBuffer, stoi(tam));

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

void Arvore::liberarArvore(No* node) {
    if (node == nullptr) {
        return;
    }
    liberarArvore(node->esq);
    liberarArvore(node->dir);
    delete node;
}

