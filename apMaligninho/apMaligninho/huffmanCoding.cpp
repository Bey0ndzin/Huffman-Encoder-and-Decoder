#include <iostream>
#include "huffman.h"
using namespace std;

int main(int argc, char* argv[])
{
	/*if (argc != 3)
	{
		cout << "Usage:\n\t huffmanCoding inputfile outputfile" << endl;
		exit(1);
	}*/
	huffman h("input.txt", "output.pcb");
	h.create_pq();
	h.create_huffman_tree();
	h.calculate_huffman_codes();
	h.coding_save();
	cout << endl;
	return 0;
}