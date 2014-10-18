#include <vector>
#include <bitset>
#include <map>
#include <cmath>
#include <ctime>
#include <climits>
#include <forward_list>
#include <algorithm>
#include "finger-extractor.h"
#include "searcher.h"
#include "util.h"

using namespace std;

const double THRESHOLD = 0.5;

bool comp(pair<int, MusicInfo> a, pair<int, MusicInfo> b) {
	return a.first < b.first;
}

int Searcher::BuildIndex(string dirPath) {
	finger_database.resize(10010);
	time_t sort_start, sort_end;
	Util::load_dir(dirPath, "txt", allFiles);
	file_map.resize(allFiles.size());

	for(int i = 0; i < (signed)allFiles.size(); i++) {
		_build_one_file_index(allFiles[i], i);
	}
	
	sort(index.begin(), index.end(), comp);
	return 0;
}

string Searcher::SubSamplingSearch(const vector<bitset<32>>& finger_block) {
	map<double, int> result_map;
	int result = -1;
	for (int i = 0; i < finger_block.size(); i++) {
		unsigned int key = finger_block[i].to_ulong();
		if (key == 0)
			continue;
		_inner_search(key, finger_block, i, result_map);
	}
	for (int i = 0; i < finger_block.size(); i++) {
		for (int j = 0; j < 32; j++)	{
			bitset<32> item = finger_block[i];
			item.flip(j);
			unsigned int key = item.to_ulong();
			if (key == 0)
				continue;
			_inner_search(key, finger_block, i, result_map);
		}
	}
	for (int i = 0; i < finger_block.size(); i++)	{
		for (int j = 0; j < 31; j++) {
			for (int m = j + 1; m < 32; m++)	{
				bitset<32> item = finger_block[i];
				item.flip(j);
				item.flip(m);
				unsigned int key = item.to_ulong();
				if (key == 0)
					continue;
				_inner_search(key, finger_block, i, result_map);
			}
		}
	}
	int file_id = result_map.begin()->second;
	return file_map[file_id];
}

long long Searcher::_binary_search(unsigned int key) {
	long long start = 0;
	long long end = index.size() - 1;
	long long mid;
	while(end >= start) {
		mid = start + (end - start) / 2;
		if(key < index[mid].first)
			end = mid - 1;
		else if(key > index[mid].first)
			start = mid + 1;
		else
			return mid;
	}
	return -1;
}

void Searcher::_build_one_file_index(const string filepath, int id) {
	vector<unsigned int> finger_file;
	string filename;
	Util::load_one_file(filepath, finger_file, filename);
	vector<bitset<32>> fingers_block = Util::VectorIntToVectorBitset(finger_file);
	finger_database[id] = fingers_block;
	file_map[id] = filename;
	MusicInfo m(id, 0);
	for(int i = 0; i < (signed)finger_file.size(); i++) {
		if (finger_file[i] != 0) {
			m.i_frame = i;
			index.push_back(make_pair(finger_file[i], m));
		}
	}
	return;
}

void Searcher::_inner_search(unsigned int key, const vector<bitset<32>>& finger_block,
														const int i, map<double, int>& result_map) {
	long long result = _binary_search(key);
	if (result == -1)
		return;
	long long start = result;
	long long end = result;
	do {
		start--;
	} while (start >= 0 && index[start].first == key);
	start++;
	do {
		end++;
	} while (end < (signed)index.size() && index[end].first == key);
	end--;

	for (long long iter = start; iter <= end; iter++) {
		int diffbits = compare_bitsets(index[iter].second.id, finger_block, i, index[iter].second.i_frame);
		double ratio = (double)diffbits / (finger_block.size() * 32);
		if (ratio <= THRESHOLD)	{
			result_map[ratio] = index[iter].second.id;
		}
	}
	return;
}


/*
 *i_frame_in_block: 在query指纹块中命中的index
 *i_frame_in_file: 在file中命中的index
*/
int Searcher::compare_bitsets(int id, const vector<bitset<32>>& finger_block,
	const int i_frame_in_block, int i_frame_in_file) {
	int block_size = finger_block.size();
	if(i_frame_in_file - i_frame_in_block < 0)
		return INT_MAX;//表示错误，返回1.0
	int diff_bits = 0;
	vector<bitset<32>>& full_audio_fingers = finger_database[id];

	if(i_frame_in_file + block_size - i_frame_in_block > full_audio_fingers.size())
		return INT_MAX;
	i_frame_in_file -= i_frame_in_block;


	for(int i = 0; i < block_size;i++)
	{
		bitset<32> subfinger_xor = finger_block[i] ^ full_audio_fingers[i_frame_in_file];
		i_frame_in_file++;
		diff_bits += (int)subfinger_xor.count();
	}

	return diff_bits;
}
