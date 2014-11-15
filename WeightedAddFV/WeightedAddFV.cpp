#define _CRT_SECURE_NO_WARNINGS
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
using namespace std;
using namespace boost;

bool WeightedAddFV(const string& name, const string& fromDir1, double w1, const string& fromDir2, double w2, const string& toDir) {
	cout << "**************************************" << name << "**************************************" << endl;

	ifstream ifs1(fromDir1 + "/" + name, ios_base::binary);
	ifstream ifs2(fromDir2 + "/" + name, ios_base::binary);
	if (!ifs1 || !ifs2) {
			cerr << "エラー: 入力ファイルの読み込みに失敗: " << strerror(errno) << endl;
		return false;
	}
	ofstream ofs(toDir + "/" + name, ios_base::binary);

	size_t elementCount = 0;
	double sumDiff = 0, sumAbsDiff = 0, sumSqDiff = 0;
	int diffMin = +65536, diffMax = -65536;
	for (; ; elementCount++) {
		int16_t value1, value2;
		ifs1.read((char*)&value1, sizeof(value1));
		ifs2.read((char*)&value2, sizeof(value2));
		if (ifs1.eof() && ifs2.eof())
			break;
		if (ifs1.eof() || ifs2.eof()) {
			cerr << "エラー: 2つの入力ファイルのサイズの不一致" << endl;
			return false;
		}
		if (!ifs1 || !ifs2) {
			cerr << "エラー: 読み込み失敗: " << strerror(errno) << endl;
			return false;
		}

		// 入力2個の差異について色々統計情報を算出する
		auto d = (int)value2 - (int)value1;
		sumDiff += d;
		sumAbsDiff += abs(d);
		sumSqDiff += d * d;
		if (d < diffMin) diffMin = d;
		if (diffMax < d) diffMax = d;

		// 入力2個の平均を出力する
		int16_t outValue = (int16_t)round(value1 * w1 + value2 * w2);
		ofs.write((const char*)&outValue, sizeof(outValue));
	}

	cout << fixed;
	cout << "差の平均:               " << setw(10) << setprecision(3) << sumDiff / elementCount << endl;
	cout << "差の絶対値の平均:       " << setw(10) << setprecision(1) << sumAbsDiff / elementCount << endl;
	cout << "差の二乗の平均の平方根: " << setw(10) << setprecision(1) << sqrt(sumSqDiff / elementCount) << endl;
	cout << "差の最大値:             " << setw(10) << diffMin << endl;
	cout << "差の最小値:             " << setw(10) << diffMax << endl;
	cout << endl;
	return true;
}

int main(int argc, char* argv[]) {
	vector<string> args(argv + 1, argv + argc);
	if (args.size() != 6) {
		cerr << "usage: WeightedAddFV FromDir1 Weight1 FromDir2 Weight2 ToDir FileNames" << endl;
		cerr << endl;
		cerr << "2つの重みファイル(Bonanzaのfv.binのような2バイト符号付整数値が" << endl;
		cerr << "連続で記録されたバイナリファイル)の重み付き加算を行います。" << endl;
		cerr << endl;
		cerr << "    FromDir1,2 入力元のフォルダパス" << endl;
		cerr << "    Weight1,2  それぞれの入力元の重み(浮動小数点数)" << endl;
		cerr << "    ToDir      重み付き加算結果の出力先のフォルダパス" << endl;
		cerr << "    FileNames  ファイル名をカンマ区切りで指定" << endl;
		cerr << endl;
		cerr << "  例：WeightedAddFV dir1 0.3 dir2 0.7 result KKP.bin,KPP.bin" << endl;
		cerr << "    ⇒ dir1/KKP.binの各要素の値に0.3を掛けたものと" << endl;
		cerr << "       dir2/KKP.binの各要素の値に0.7を掛けたものの和(端数は四捨五入)を、" << endl;
		cerr << "       result/KKP.binに出力します。同様にKPP.binも。" << endl;
		return 1;
	}
	vector<string> names;
	split(names, args[5], is_any_of(" ,"));
	for (auto name : names) {
		if (!WeightedAddFV(name,
			args[0], lexical_cast<double>(args[1]),
			args[2], lexical_cast<double>(args[3]), args[4]))
			return 2;
	}
	return 0;
}

