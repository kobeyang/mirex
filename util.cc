#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include "util.h"

using namespace std;

static const int SIZE = 400000 * 33 + 1;
char buffer[SIZE];

int Util::load_one_file(string filepath, vector<unsigned int>& audio_fingers,
												string& filename) {
	FILE* fp = fopen(filepath.c_str(), "r");
	if(fp == NULL) {
		std::cout<<"no such file: "<< filepath<<std::endl;
		return -1;
	}
	char line[100];
	fgets(line, 100, fp);
	filename = string(line);
	filename = filename.substr(0, filename.size() - 1);
	while(true)	{
		fgets(line, 100, fp);
		// need to implement
		if(feof(fp))
			break;
		string s(line);
		s = s.substr(0, (signed)s.size() - 1);
		audio_fingers.push_back(stoul(s));
	}
	fclose(fp);
	return 0;
}

bool Util::load_dir(std::string dirpath, std::string type, vector<std::string>& allFiles)
{
	allFiles.clear();
	string temppath = dirpath + "\\*." + type;
	DIR *d;
	struct dirent *file;
	if(!(d = opendir(dirpath.c_str()))) {
		cerr << "can not find dir or file!" << endl;
		return false;
	}
	while ((file = readdir(d)) != NULL) {
		if (strncmp(file->d_name, ".", 1) == 0)
			continue;
		allFiles.push_back(dirpath + "/" + string(file->d_name));
	}
	closedir(d);
	return true;
}

vector<bitset<32>> Util::VectorIntToVectorBitset(vector<unsigned int> v)
{
	vector<bitset<32>> bv;
	for(int i = 0; i < v.size(); i++)
	{
		bitset<32> b(v[i]);
		bv.push_back(b);
	}
	return bv;
}

vector<unsigned int> Util::VectorBitsetToVectorInt(vector<bitset<32>> v)
{
	vector<unsigned int> iv;
	for(int i = 0; i < v.size(); i++)
	{
		unsigned int key = v[i].to_ulong();
		iv.push_back(key);
	}
	return iv;
}
