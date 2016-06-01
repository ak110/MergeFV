#define _CRT_SECURE_NO_WARNINGS
#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
using namespace std;

bool MergeFV(const vector<string>& inFiles, const vector<double>& inWeights_, const string& outFile) {
	vector<double> inWeights = inWeights_;
	if (inWeights.empty())
		for (size_t i = 0; i < inFiles.size(); i++)
			inWeights.push_back(1.0 / inFiles.size());

	vector<ifstream> ifsList;
	for (const auto& path : inFiles)
		ifsList.emplace_back(ifstream(path, ios_base::binary));
	if (!all_of(ifsList.begin(), ifsList.end(), bind(&ifstream::good, placeholders::_1))) {
		cerr << "エラー: 入力ファイルの読み込みに失敗: " << strerror(errno) << endl;
		return false;
	}
	ofstream ofs(outFile, ios_base::binary);
	if (!ofs) {
		cerr << "エラー: 出力ファイルの読み込みに失敗: " << strerror(errno) << endl;
		return false;
	}

	cout << "マージ中 . . ." << endl;

	struct Stat
	{
		double sumDiff = 0, sumAbsDiff = 0, sumSqDiff = 0;
		int diffMin = +65536, diffMax = -65536;
	};
	vector<Stat> stats(inFiles.size());
	vector<int16_t> values(inFiles.size());

	size_t elementCount = 0;
	for (; ; elementCount++) {
		for (size_t i = 0; i < inFiles.size(); i++)
			ifsList[i].read((char*)&values[i], sizeof values[i]);
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

		double result = 0;
		for (size_t i = 0; i < values.size(); i++)
			result += values[i] * inWeights[i];
		int16_t result16 =
			INT16_MAX <= result ? INT16_MAX :
			result <= -INT16_MAX ? INT16_MAX :
			(int16_t)round(result);

		// 入力と結果の差異について色々統計情報を算出する
		for (size_t i = 0; i < values.size(); i++) {
			auto d = (int)result16 - (int)values[i];
			stats[i].sumDiff += d;
			stats[i].sumAbsDiff += abs(d);
			stats[i].sumSqDiff += d * d;
			if (d < stats[i].diffMin) stats[i].diffMin = d;
			if (stats[i].diffMax < d) stats[i].diffMax = d;
		}

		// 出力
		ofs.write((const char*)&result16, sizeof result16);
	}

	cout << fixed;
	cout << "入力ファイル:" << endl;
	for (auto& path : inFiles)
		cout << "    " << path << endl;
	cout << "出力ファイル:  " << outFile << endl;
	cout << "要素数:        " << elementCount << endl;
	cout << "差の平均:              ";
	for (auto& stat : stats)
		cout << " " << setw(10) << setprecision(3) << stat.sumDiff / elementCount;
	cout << endl;
	cout << "差の絶対値の平均:      ";
	for (auto& stat : stats)
		cout << " " << setw(10) << setprecision(1) << stat.sumAbsDiff / elementCount;
	cout << endl;
	cout << "差の二乗平均平方根:    ";
	for (auto& stat : stats)
		cout << " " << setw(10) << setprecision(1) << sqrt(stat.sumSqDiff / elementCount);
	cout << endl;
	cout << "差の最大値:            ";
	for (auto& stat : stats)
		cout << " " << setw(10) << stat.diffMin;
	cout << endl;
	cout << "差の最小値:            ";
	for (auto& stat : stats)
		cout << " " << setw(10) << stat.diffMax;
	cout << endl;
	cout << "マージ完了" << endl;
	return true;
}

void Usage() {
	cerr << "usage: MergeFV [options] InFile1 [InFile2 ...] OutFile" << endl;
	cerr << endl;
	cerr << "複数の重みファイル(Bonanzaのfv.binのような2バイト符号付整数値が" << endl;
	cerr << "連続で記録されたバイナリファイル)の重み付き加算を行います。" << endl;
	cerr << endl;
	cerr << "    InFile     入力元のファイルパス" << endl;
	cerr << "    OutFile    出力先のファイルパス" << endl;
	cerr << endl;
	cerr << "    -w 重み     各入力の重みを実数のカンマ区切りで指定する。省略時は1.0/入力ファイル数になる。(通常の算術平均)" << endl;
	cerr << endl;
	cerr << "  例：MergeFV -w 0.3,0.7 FV1.bin FV2.bin result.bin" << endl;
	cerr << "    ⇒ FV1.binの各要素の値に0.3を掛けたものと" << endl;
	cerr << "       FV2.binの各要素の値に0.7を掛けたものの和(端数は四捨五入)を、" << endl;
	cerr << "       result.binに出力。" << endl;
}

int main(int argc, char* argv[]) {
	vector<string> files;
	vector<double> inWeights;
	bool hasInWeights = false;
	{
		vector<string> args(argv + 1, argv + argc);
		for (size_t i = 0; i < args.size(); i++) {
			if (args[i] == "-w") {
				i++;
				if (args.size() <= i) {
					cerr << "不正なオプション: " << args[i - 1] << endl;
					Usage();
					return 1;
				}
				hasInWeights = true;
				vector<string> sp;
				boost::split(sp, args[i], boost::is_any_of(","));
				for (auto s : sp)
					inWeights.push_back(stod(s));
			} else {
				files.push_back(args[i]);
			}
		}
		if (files.size() < 2) {
			cerr << "不正なオプション: 入力ファイルと出力ファイルが未指定" << endl;
			Usage();
			return 1;
		}
		if (hasInWeights && inWeights.size() != files.size() - 1) {
			cerr << "不正なオプション: 入力ファイル数と重み(-w)の数が不一致" << endl;
			Usage();
			return 1;
		}
	}
	string outFile = files.back();
	files.pop_back();

	// 処理
	return MergeFV(files, inWeights, outFile) ? 0 : 2;
}

