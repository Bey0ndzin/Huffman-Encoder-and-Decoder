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

class Arvore {
	private:
	public:
        // Tabela para mapear caracteres convertendo eles em códigos binários
        // Funciona igual objeto de javascript
        // {"byte": "valorEmCodigo"}
        map<unsigned char, string> huffmanCodes;

        No* construirArvore(map<unsigned char, int>& frequencias);

        void gerarCodigos(No* raiz, string code);

        void escreveArvore(ofstream& outFile, No* raiz, const string& extOrig);

        No* lerArvore(ifstream& inFile, string& extOrig);

        void liberarArvore(No* node);
};
