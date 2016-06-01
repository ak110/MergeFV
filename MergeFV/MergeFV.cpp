#define _CRT_SECURE_NO_WARNINGS
#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
using namespace std;

bool MergeFV(const vector<string>& inFiles, const vector<double>& inWeights, const string& outFile) {
	vector<ifstream> ifsList;
	for (const auto& path : inFiles)
		ifsList.emplace_back(ifstream(path, ios_base::binary));
	if (!all_of(ifsList.begin(), ifsList.end(), bind(&ifstream::good, placeholders::_1))) {
		cerr << "�G���[: ���̓t�@�C���̓ǂݍ��݂Ɏ��s: " << strerror(errno) << endl;
		return false;
	}
	ofstream ofs(outFile, ios_base::binary);
	if (!ofs) {
		cerr << "�G���[: �o�̓t�@�C���̓ǂݍ��݂Ɏ��s: " << strerror(errno) << endl;
		return false;
	}

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
			cerr << "�G���[: ���̓t�@�C���̃T�C�Y�s��v" << endl;
			return false;
		}
		if (!all_of(ifsList.begin(), ifsList.end(), bind(&ifstream::good, placeholders::_1))) {
			cerr << "�G���[: �ǂݍ��ݎ��s: " << strerror(errno) << endl;
			return false;
		}

		double result = 0;
		for (size_t i = 0; i < values.size(); i++)
			result += values[i] * inWeights[i];
		int16_t result16 =
			INT16_MAX <= result ? INT16_MAX :
			result <= -INT16_MAX ? INT16_MAX :
			(int16_t)round(result);

		// ���͂ƌ��ʂ̍��قɂ��ĐF�X���v�����Z�o����
		for (size_t i = 0; i < values.size(); i++) {
			auto d = (int)result16 - (int)values[i];
			stats[i].sumDiff += d;
			stats[i].sumAbsDiff += abs(d);
			stats[i].sumSqDiff += d * d;
			if (d < stats[i].diffMin) stats[i].diffMin = d;
			if (stats[i].diffMax < d) stats[i].diffMax = d;
		}

		// �o��
		ofs.write((const char*)&result16, sizeof result16);
	}

	cout << fixed;
	cout << "���̕���:              ";
	for (auto& stat : stats)
		cout << " " << setw(10) << setprecision(3) << stat.sumDiff / elementCount;
	cout << endl;
	cout << "���̐�Βl�̕���:      ";
	for (auto& stat : stats)
		cout << " " << setw(10) << setprecision(1) << stat.sumAbsDiff / elementCount;
	cout << endl;
	cout << "���̓�敽�ϕ�����:    ";
	for (auto& stat : stats)
		cout << " " << setw(10) << setprecision(1) << sqrt(stat.sumSqDiff / elementCount);
	cout << endl;
	cout << "���̍ő�l:            ";
	for (auto& stat : stats)
		cout << " " << setw(10) << stat.diffMin;
	cout << endl;
	cout << "���̍ŏ��l:            ";
	for (auto& stat : stats)
		cout << " " << setw(10) << stat.diffMax;
	cout << endl;
	cout << endl;
	return true;
}

void Usage() {
	cerr << "usage: MergeFV [options] InFile1 [InFile2 ...] OutFile" << endl;
	cerr << endl;
	cerr << "�����̏d�݃t�@�C��(Bonanza��fv.bin�̂悤��2�o�C�g�����t�����l��" << endl;
	cerr << "�A���ŋL�^���ꂽ�o�C�i���t�@�C��)�̏d�ݕt�����Z���s���܂��B" << endl;
	cerr << endl;
	cerr << "    InFile     ���͌��̃t�@�C���p�X" << endl;
	cerr << "    OutFile    �o�͐�̃t�@�C���p�X" << endl;
	cerr << endl;
	cerr << "    -w �d��     �e���͂̏d�݂������̃J���}��؂�Ŏw�肷��B�ȗ�����1.0/���̓t�@�C�����ɂȂ�B(�ʏ�̎Z�p����)" << endl;
	cerr << endl;
	cerr << "  ��FMergeFV -w 0.3,0.7 FV1.bin FV2.bin result.bin" << endl;
	cerr << "    �� FV1.bin�̊e�v�f�̒l��0.3���|�������̂�" << endl;
	cerr << "       FV2.bin�̊e�v�f�̒l��0.7���|�������̘̂a(�[���͎l�̌ܓ�)���A" << endl;
	cerr << "       result.bin�ɏo�́B" << endl;
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
					cerr << "�s���ȃI�v�V����: " << args[i - 1] << endl;
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
			cerr << "�s���ȃI�v�V����: ���̓t�@�C���Əo�̓t�@�C�������w��" << endl;
			Usage();
			return 1;
		}
		if (hasInWeights && inWeights.size() != files.size() - 1) {
			cerr << "�s���ȃI�v�V����: ���̓t�@�C�����Əd��(-w)�̐����s��v" << endl;
			Usage();
			return 1;
		}
	}
	string outFile = files.back();
	files.pop_back();
	if (!hasInWeights)
		for (size_t i = 0; i < files.size(); i++)
			inWeights.push_back(1.0 / files.size());

	// ����
	return MergeFV(files, inWeights, outFile) ? 0 : 2;
}

