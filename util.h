#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <bitset>

class Util
{
public:
	static int load_one_file(std::string filepath, std::vector<unsigned int>& audio_fingers, std::string& filename);
	static bool load_dir(std::string dirpath, std::string type, std::vector<std::string>& allFiles);
	static std::vector<std::bitset<32>> VectorIntToVectorBitset(std::vector<unsigned int> v);
	static std::vector<unsigned int> VectorBitsetToVectorInt(std::vector<std::bitset<32>> v);
};
