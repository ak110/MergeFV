#define _CRT_SECURE_NO_WARNINGS
#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
using namespace std;

bool CompareFV(const vector<string>& inFiles) {
	vector<unique_ptr<ifstream>> ifsList;
	for (const auto& path : inFiles)
		ifsList.emplace_back(new ifstream(path, ios_base::binary));
	if (!all_of(ifsList.begin(), ifsList.end(), bind(&ifstream::good, placeholders::_1))) {
		cerr << "エラー: 入力ファイルの読み込みに失敗: " << strerror(errno) << endl;
		return false;
	}

	int64_t notEqualCount = 0;
	int absMax = 0;
	double absSum = 0, absSq = 0;

	vector<int16_t> values(inFiles.size());

	size_t elementCount = 0;
	for (; ; elementCount++) {
		for (size_t i = 0; i < inFiles.size(); i++)
			ifsList[i]->read((char*)&values[i], sizeof values[i]);
		if (all_of(ifsList.begin(), ifsList.end(), bind(&ifstream::eof, placeholders::_1)))
			break;
		if (any_of(ifsList.begin(), ifsList.end(), bind(&ifstream::eof, placeholders::_1))) {
			cerr << "エラー: 入力ファイルのサイズ不一致" << endl;
			return false;
		}
		if (!all_of(ifsList.begin(), ifsList.end(), bind(&ifstream::good, placeholders::_1))) {
			cerr << "エラー: 読み込み失敗: " << strerror(errno) << endl;
			return false;
		}

		assert(values.size() == 2);

		if (values[0] != values[1])
		{
			notEqualCount++;
			auto d = abs((int)values[0] - (int)values[1]);
			if (absMax < d)
				absMax = d;
			absSum += d;
			absSq += d * d;
		}
	}

	cout << fixed;
	cout << "入力ファイル:" << endl;
	for (auto& path : inFiles)
		cout << "    " << path << endl;
	cout << "要素数:             " << elementCount << endl;
	cout << "不一致要素数:       " << notEqualCount
		<< " (" << setprecision(1) << (notEqualCount * 100.0) / elementCount << "%)" << endl;
	cout << "差の絶対値の最大:   " << absMax << endl;
	cout << "差の絶対値の平均:   " << setprecision(1) << (absSum / notEqualCount) << endl;
	cout << "差の二乗平均平方根: " << setprecision(1) << sqrt(absSq / notEqualCount) << endl;
	cout << endl;
	return true;
}

void Usage() {
	cerr << "usage: CompareFV InFile1 InFile2" << endl;
	cerr << endl;
	cerr << "2つの重みファイル(Bonanzaのfv.binのような2バイト符号付整数値が" << endl;
	cerr << "連続で記録されたバイナリファイル)の差の統計(差の絶対値の平均など)を算出します。" << endl;
	cerr << endl;
	cerr << "    InFile1    比較元のファイルパス" << endl;
	cerr << "    InFile2    比較先のファイルパス" << endl;
	cerr << endl;
	cerr << "  例：CompareFV FV1.bin FV2.bin > CompareFV.log" << endl;
	cerr << "    ⇒ FV1.binとFV2.binの差の統計を、" << endl;
	cerr << "       CompareFV.logに出力。" << endl;
}

int main(int argc, char* argv[]) {
	vector<string> files;
	bool hasInWeights = false;
	{
		vector<string> args(argv + 1, argv + argc);
		for (size_t i = 0; i < args.size(); i++) {
			files.push_back(args[i]);
		}
		if (files.size() != 2) {
			cerr << "不正なオプション: 入力ファイル数が2以外" << endl;
			Usage();
			return 1;
		}
	}
	// 処理
	return CompareFV(files) ? 0 : 2;
}

