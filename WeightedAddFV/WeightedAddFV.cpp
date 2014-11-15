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
			cerr << "�G���[: ���̓t�@�C���̓ǂݍ��݂Ɏ��s: " << strerror(errno) << endl;
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
			cerr << "�G���[: 2�̓��̓t�@�C���̃T�C�Y�̕s��v" << endl;
			return false;
		}
		if (!ifs1 || !ifs2) {
			cerr << "�G���[: �ǂݍ��ݎ��s: " << strerror(errno) << endl;
			return false;
		}

		// ����2�̍��قɂ��ĐF�X���v�����Z�o����
		auto d = (int)value2 - (int)value1;
		sumDiff += d;
		sumAbsDiff += abs(d);
		sumSqDiff += d * d;
		if (d < diffMin) diffMin = d;
		if (diffMax < d) diffMax = d;

		// ����2�̕��ς��o�͂���
		int16_t outValue = (int16_t)round(value1 * w1 + value2 * w2);
		ofs.write((const char*)&outValue, sizeof(outValue));
	}

	cout << fixed;
	cout << "���̕���:               " << setw(10) << setprecision(3) << sumDiff / elementCount << endl;
	cout << "���̐�Βl�̕���:       " << setw(10) << setprecision(1) << sumAbsDiff / elementCount << endl;
	cout << "���̓��̕��ς̕�����: " << setw(10) << setprecision(1) << sqrt(sumSqDiff / elementCount) << endl;
	cout << "���̍ő�l:             " << setw(10) << diffMin << endl;
	cout << "���̍ŏ��l:             " << setw(10) << diffMax << endl;
	cout << endl;
	return true;
}

int main(int argc, char* argv[]) {
	vector<string> args(argv + 1, argv + argc);
	if (args.size() != 6) {
		cerr << "usage: WeightedAddFV FromDir1 Weight1 FromDir2 Weight2 ToDir FileNames" << endl;
		cerr << endl;
		cerr << "2�̏d�݃t�@�C��(Bonanza��fv.bin�̂悤��2�o�C�g�����t�����l��" << endl;
		cerr << "�A���ŋL�^���ꂽ�o�C�i���t�@�C��)�̏d�ݕt�����Z���s���܂��B" << endl;
		cerr << endl;
		cerr << "    FromDir1,2 ���͌��̃t�H���_�p�X" << endl;
		cerr << "    Weight1,2  ���ꂼ��̓��͌��̏d��(���������_��)" << endl;
		cerr << "    ToDir      �d�ݕt�����Z���ʂ̏o�͐�̃t�H���_�p�X" << endl;
		cerr << "    FileNames  �t�@�C�������J���}��؂�Ŏw��" << endl;
		cerr << endl;
		cerr << "  ��FWeightedAddFV dir1 0.3 dir2 0.7 result KKP.bin,KPP.bin" << endl;
		cerr << "    �� dir1/KKP.bin�̊e�v�f�̒l��0.3���|�������̂�" << endl;
		cerr << "       dir2/KKP.bin�̊e�v�f�̒l��0.7���|�������̘̂a(�[���͎l�̌ܓ�)���A" << endl;
		cerr << "       result/KKP.bin�ɏo�͂��܂��B���l��KPP.bin���B" << endl;
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

