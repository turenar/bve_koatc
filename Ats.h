// 以下の ifdef ブロックは DLL から簡単にエクスポートさせるマクロを作成する標準的な方法です。 
// この DLL 内のすべてのファイルはコマンドラインで定義された ATS_EXPORTS シンボル
// でコンパイルされます。このシンボルはこの DLL が使用するどのプロジェクト上でも未定義でなけ
// ればなりません。この方法ではソースファイルにこのファイルを含むすべてのプロジェクトが DLL 
// からインポートされたものとして ATS_API 関数を参照し、そのためこの DLL はこのマク 
// ロで定義されたシンボルをエクスポートされたものとして参照します。
//#pragma data_seg(".shared")
//#pragma data_seg()

#pragma once
#include <list>
#include "atsplugin.h"
#include "koatc.h"
#include "Monitor.h"

extern TCHAR g_fullpath[_MAX_PATH];
extern TCHAR g_drive[_MAX_DRIVE];
extern TCHAR g_dir[_MAX_PATH];
extern TCHAR g_fname[_MAX_FNAME];
extern TCHAR g_ext[_MAX_EXT];
extern TCHAR g_path[_MAX_PATH];
extern TCHAR g_inifilepath[_MAX_PATH];

extern int g_EmrBrake; // 非常ノッチ
extern int g_SvcBrake; // 常用最大ノッチ
extern bool g_pilotlamp; // パイロットランプ
extern int g_time; // 現在時刻
extern float g_speed; // 速度計の速度[km/h]
extern int g_deltaT; // フレーム時間[ms/frame]
extern double g_location;	//現在位置
extern int g_sta_dopen_timer; // ドア開から駅停車パターン消去までのタイマー
extern int g_sta_dopen_wait; // 上の時間
extern int g_genLog;
extern std::list<int> beaconIdx;
extern std::list<int> beaconSig;
extern std::list<float> beaconDist;
extern std::list<int> beaconOpt;

extern ATS_HANDLES g_output; // 出力

void BeaconProcessor(int, int, float, int);
void BeaconReader(void);
void PanelAssign(int*);
void SoundAssign(int*);

extern CKoAtc g_koatc;

extern CMon g_mon;
