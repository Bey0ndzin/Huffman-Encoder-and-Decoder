#include "arvore.h"

using namespace std;

// Função para construir a árvore Huffman
// map de frequências
// {"byte": "frequencia"}
No* Arvore::construirArvore(map<unsigned char, int>& frequencias) {
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

// Função para escrever a árvore Huffman em um arquivo
void Arvore::escreveArvore(ofstream& outFile, No* raiz, const string& extOrig) {
    if (raiz == nullptr) {
        return;
    }

    // Escrever a extensão original no cabeçalho
    outFile << extOrig.length();
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
No* Arvore::lerArvore(ifstream& inFile, string& extOrig) {
    // Lê o tamanho da extensão original do cabeçalho
    char lenBuffer[1];
    inFile.read(lenBuffer, 1);
    extOrig = string(lenBuffer, 1);

    // Lê a extensão original do cabeçalho]
    string tam = string(lenBuffer, 1);
    char* extBuffer = new char[stoi(tam)];
    inFile.read(extBuffer, stoi(tam));
    extOrig = string(extBuffer, stoi(tam));

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

void Arvore::liberarArvore(No* node) {
    if (node == nullptr) {
        return;
    }
    liberarArvore(node->esq);
    liberarArvore(node->dir);
    delete node;
}

