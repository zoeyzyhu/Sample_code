#include "Bucket.hpp"

#include <iostream>

using namespace std;

Bucket::Bucket(Disk* _disk) : disk(_disk) {}

vector<uint> Bucket::get_left_rel() { return left_rel; }

vector<uint> Bucket::get_right_rel() { return right_rel; }

void Bucket::add_left_rel_page(uint page_id) {
	left_rel.push_back(page_id);
	num_left_rel_record += disk->diskRead(page_id)->size();
}

void Bucket::add_right_rel_page(uint page_id) {
	right_rel.push_back(page_id);
	num_right_rel_record += disk->diskRead(page_id)->size();
}

// void Bucket::print() {
// 	cout << "disk ids of left rel records in this bucket: " << endl;
// 	for (int i = 0; i < left_rel.size(); ++i) {
// 		cout << left_rel[i] << " ";
// 	}
// 	cout << endl;
// 	cout << "disk ids of right rel records in this bucket: " << endl;
// 	for (int i = 0; i < right_rel.size(); ++i) {
// 		cout << right_rel[i] << " ";
// 	}
// 	cout << endl;
// }
