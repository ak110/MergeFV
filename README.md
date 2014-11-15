WeightedAddFV
===========
2つの重みファイル(Bonanzaのfv.binのような2バイト符号付整数値が連続で記録されたバイナリファイル)の重み付き加算を行うコマンドラインツールです。
エラー時も出力ファイルが半端な状態で残るなど、いろいろ適当実装なので悪しからず。

使い方
----
    WeightedAddFV FromDir1 Weight1 FromDir2 Weight2 ToDir FileNames

* FromDir1,2 入力元のフォルダパス
* Weight1,2  それぞれの入力元の重み(浮動小数点数)
* ToDir      重み付き加算結果の出力先のフォルダパス
* FileNames  ファイル名をカンマ区切りで指定

例
----
    WeightedAddFV dir1 0.3 dir2 0.7 result KKP.bin,KPP.bin

dir1/KKP.binの各要素の値に0.3を掛けたものとdir2/KKP.binの各要素の値に0.7を掛けたものの和(端数は四捨五入)を、result/KKP.binに出力します。同様にKPP.binも。
