#pragma once
#include <vector>
#include <string>
#include <bitset>
#include <map>
#include <unordered_map>

class MusicInfo {
public:
	int id;
	int i_frame;
	MusicInfo(int ID, int FID):id(ID), i_frame(FID){};
	MusicInfo(){};
};

class Searcher {
private:
	FILE* fp;
	double diff;
	int match;
	std::vector<std::string> allFiles;
	int _insert_one_item(unsigned int key, MusicInfo& m);
	void _build_one_file_index(const std::string filepath, int id);
	void _inner_search(unsigned int key, const std::vector<std::bitset<32>>& finger_block,
										 int i, std::map<double, int>& result_map);
	bool _comp(std::pair<unsigned int, MusicInfo>, std::pair<unsigned int, MusicInfo>);
	long long _binary_search(unsigned int key);
	int _loadFingerFromOneFile(std::string filepath_prefix, unsigned int fileNum);
	int _outputFingerToOneFile(std::string filepath_prefix, unsigned int databaseSize, unsigned int fileNum);

public:
	std::vector<std::pair<unsigned int, MusicInfo>> index;
	std::vector<std::vector<std::bitset<32>>> finger_database;
	std::vector<std::string> file_map;
	Searcher(){};
	int BuildIndex(std::string dirPath);
	std::string SubSamplingSearch(const std::vector<std::bitset<32>>& finger_block);
	int compare_bitsets(int id, const std::vector<std::bitset<32>>& finger_block, int i_frame_in_block, int i_frame_in_file);
};
