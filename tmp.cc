#include <iostream>
#include <string>

using namespace std;

int main() {
	for (int i = 20000; i < 26000; i++)
		cout << "./query_32kbps/" + to_string(i) + ".wav" << endl;
	return 0;
}
