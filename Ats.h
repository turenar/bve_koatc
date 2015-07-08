// 以下の ifdef ブロックは DLL から簡単にエクスポートさせるマクロを作成する標準的な方法です。 
// この DLL 内のすべてのファイルはコマンドラインで定義された ATS_EXPORTS シンボル
// でコンパイルされます。このシンボルはこの DLL が使用するどのプロジェクト上でも未定義でなけ
// ればなりません。この方法ではソースファイルにこのファイルを含むすべてのプロジェクトが DLL 
// からインポートされたものとして ATS_API 関数を参照し、そのためこの DLL はこのマク 
// ロで定義されたシンボルをエクスポートされたものとして参照します。
//#pragma data_seg(".shared")
//#pragma data_seg()

TCHAR g_fullpath[_MAX_PATH];
TCHAR g_drive[_MAX_DRIVE];
TCHAR g_dir[_MAX_PATH];
TCHAR g_fname[_MAX_FNAME];
TCHAR g_ext[_MAX_EXT];
TCHAR g_path[_MAX_PATH];
TCHAR g_inifilepath[_MAX_PATH];

int g_EmrBrake; // 非常ノッチ
int g_SvcBrake; // 常用最大ノッチ
bool g_pilotlamp; // パイロットランプ
int g_time; // 現在時刻
float g_speed; // 速度計の速度[km/h]
int g_deltaT; // フレーム時間[ms/frame]
double g_location;	//現在位置
int g_genLog;
std::list<int> beaconIdx;
std::list<int> beaconSig;
std::list<float> beaconDist;
std::list<int> beaconOpt;

ATS_HANDLES g_output; // 出力

void BeaconProcessor(int, int, float, int);
void BeaconReader(void);
void PanelAssign(int*);
void SoundAssign(int*);

CKoAtc g_koatc;

CMon g_mon;
