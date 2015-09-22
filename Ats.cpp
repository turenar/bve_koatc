// Ats.cpp : DLL アプリケーション用のエントリ ポイントを定義します。
//
#include "stdafx.h"
#include <list>
#include "atsplugin.h"
#include "KOATC.h"
#include "Monitor.h"
#include "Ats.h"


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
int g_sta_dopen_timer; // 駅停車→ドア開までのタイマー
int g_sta_dopen_wait; // 上の時間
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


BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	) {
	// get DLL path and create ini path
	GetModuleFileName((HINSTANCE) hModule, g_fullpath, sizeof(g_fullpath));
	_splitpath_s(g_fullpath, g_drive, g_dir, g_fname, g_ext);
	strcpy_s(g_path, sizeof(g_path), g_drive);
	strcat_s(g_path, sizeof(g_path) - strlen(g_path), g_dir);
	strcpy_s(g_inifilepath, sizeof(g_inifilepath), g_path);
	strcat_s(g_inifilepath, sizeof(g_inifilepath) - strlen(g_inifilepath), "ats.ini");
	OutputDebugString("\n\nDebug: DLL file path = ");
	OutputDebugString(g_fullpath);
	OutputDebugString("\n");
	OutputDebugString("\nIni file path+ ");
	OutputDebugString(g_inifilepath);
	OutputDebugString("\n");
	// end of ini path

	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

ATS_API int WINAPI GetPluginVersion() {
	return ATS_VERSION;
}

ATS_API void WINAPI SetVehicleSpec(ATS_VEHICLESPEC vehicleSpec) {
	g_SvcBrake = vehicleSpec.BrakeNotches;
	g_EmrBrake = g_SvcBrake + 1;
	g_mon.SetVS(&vehicleSpec, g_path);
	g_koatc.SetVS(&vehicleSpec, g_path);
}

ATS_API void WINAPI Initialize(int brake) {
	g_speed = 0;

	// ini file
	g_genLog = GetPrivateProfileInt(TEXT("General"), TEXT("Log"), 0, g_inifilepath);
	g_sta_dopen_wait = GetPrivateProfileInt(TEXT("Monitor"), TEXT("StaDopenClock"), 2500, g_inifilepath);

	// initialize
	g_koatc.Init();
	g_mon.Init(true);
}

ATS_API ATS_HANDLES WINAPI Elapse(ATS_VEHICLESTATE vehicleState, int *panel, int *sound) {
	// for debug ////////////////
	/*
	char debugstr[1000]="debugstr";
	FILE *outputfile;
	TCHAR outfilepath[_MAX_PATH];
	char logmode[3];
	if ( g_genLog == -1 ) strcpy_s( logmode, sizeof(logmode), "a" );
	else strcpy_s( logmode, sizeof(logmode), "w" );
	strcpy_s( outfilepath, sizeof(outfilepath), g_path );
	strcat_s( outfilepath, sizeof(outfilepath) - strlen(outfilepath), "Log.txt" );
	*/
	///////////////////////////////

	// Update Frame Info
	g_deltaT = vehicleState.Time - g_time;
	g_time = vehicleState.Time;
	g_speed = vehicleState.Speed;
	g_location = vehicleState.Location;

	// Process beacon recorded between this frame and previous frame
	BeaconReader();

	// Reverser Instruction
	g_output.Reverser = g_mon.ctrl.rvs;

	// Brake Instruction
	if (g_koatc.kosigmgr.emgBrk || (g_mon.ctrl.brk == g_EmrBrake)) {
		g_output.Brake = g_EmrBrake;
	} else if (g_mon.ctrl.brk < g_koatc.brkOutput) {
		g_output.Brake = g_koatc.brkOutput;
	} else {
		g_output.Brake = g_mon.ctrl.brk;
	}
	// Power Instruction
	if (g_output.Brake == 0 && g_pilotlamp) {
		g_output.Power = g_mon.ctrl.mc;
	} else g_output.Power = 0;

	g_mon.Run(&vehicleState, &g_output);

	g_output.ConstantSpeed = g_mon.ctrl.csc;

	g_koatc.Run(&vehicleState, g_mon.ctrl.brk);

	PanelAssign(panel);
	SoundAssign(sound);

	return g_output;
}

ATS_API void WINAPI SetPower(int notch) {
	g_mon.MCInput(notch);
}

ATS_API void WINAPI SetBrake(int notch) {
	g_mon.BHInput(notch);
}

ATS_API void WINAPI SetReverser(int pos) {
	g_mon.RVSInput(pos);
}

ATS_API void WINAPI KeyDown(int atsKeyCode) {
	switch (atsKeyCode) {
	case ATS_KEY_S:
		break;
	case ATS_KEY_A1: // Ins
		break;
	case ATS_KEY_B1: // Home
		break;
	case ATS_KEY_B2: // End
		if (g_speed == 0.0f) {
			g_koatc.Confirm(true);
		}
		break;
	case ATS_KEY_D:
		break;
	case ATS_KEY_C1:
		break;
	case ATS_KEY_C2:
		break;
	case ATS_KEY_H:
		break;
	case ATS_KEY_J:
		break;
	case ATS_KEY_K:
		break;
	}
}

ATS_API void WINAPI KeyUp(int hornType) {
	//	if( hornType == ATS_KEY_S ){ g_atssn.UpButton(); }
}

ATS_API void WINAPI HornBlow(int atsHornBlowIndex) {
}

ATS_API void WINAPI DoorOpen() {
	g_pilotlamp = false;
	g_sta_dopen_timer = g_time + g_sta_dopen_wait;
	g_mon.DoorOpn();
}

ATS_API void WINAPI DoorClose() {
	g_pilotlamp = true;
	g_mon.DoorCls();
	g_sta_dopen_timer = -1;
}

ATS_API void WINAPI SetSignal(int signal) {
	g_koatc.SetNewSig(g_location, signal);
}

ATS_API void WINAPI SetBeaconData(ATS_BEACONDATA beaconData) {
	// for ATC and Monitor
	beaconIdx.push_back(beaconData.Type);
	beaconSig.push_back(beaconData.Signal);
	beaconDist.push_back(beaconData.Distance);
	beaconOpt.push_back(beaconData.Optional);
	// for Wayside Instruments
	switch (beaconData.Type) {
	case	ATS_BEACON_KOATC_BARLIMIT:
		g_koatc.kobarlimit.BarLimit(g_location, g_speed, &beaconData);
		break;
	}
}
void BeaconReader() {
	int b_id, b_sig, b_op;
	float b_dist;
	// for debug ////////////////
	char debugstr[1000];
	FILE *outputfile;
	TCHAR outfilepath[_MAX_PATH];
	char logmode[3];
	if (g_genLog == -1) strcpy_s(logmode, sizeof(logmode), "a");
	else strcpy_s(logmode, sizeof(logmode), "w");
	strcpy_s(outfilepath, sizeof(outfilepath), g_path);
	strcat_s(outfilepath, sizeof(outfilepath) - strlen(outfilepath), "BeaconLog.txt");
	///////////////////////////////

	while (!beaconIdx.empty() && !beaconSig.empty() && !beaconDist.empty() && !beaconOpt.empty()) {
		b_id = beaconIdx.front(); beaconIdx.pop_front();
		b_sig = beaconSig.front(); beaconSig.pop_front();
		b_dist = beaconDist.front(); beaconDist.pop_front();
		b_op = beaconOpt.front(); beaconOpt.pop_front();
		BeaconProcessor(b_id, b_sig, b_dist, b_op);

		// for debug ////////////////
		OutputDebugString(("--beacon--\n"));
		sprintf_s(debugstr, sizeof(debugstr), "gLoc: %f / Ty: %d / Sg: %d / Dt: %f / Op: %d\n", g_location, b_id, b_sig, b_dist, b_op);
		OutputDebugString(debugstr);
		if (g_genLog != 0) {
			if (fopen_s(&outputfile, outfilepath, logmode) == 0) {
				fprintf_s(outputfile, debugstr);
				fclose(outputfile);
			}
		}
		///////////////////////////////
	}
	beaconIdx.clear();
	beaconSig.clear();
	beaconDist.clear();
	beaconOpt.clear();
}
void BeaconProcessor(int beaconindex, int sig, float dist, int optional) {
	switch (beaconindex) {
	case	ATS_BEACON_KOSTA_NOW:
		g_koatc.koatcsta.SetTrainID(optional);
		g_mon.SetTrainID(optional);
		break;
	case	ATS_BEACON_KOSTA_STOP:
		g_koatc.koatcsta.StaStop(optional, g_time, g_speed);
		break;
	case	ATS_BEACON_KOSTA_SB:
		g_koatc.staAvoidOverrun = false;
		g_koatc.kosta1.RegPatt(2, g_location, g_koatc.koatcsta.StaPatt(g_location, optional));
		break;
	case	ATS_BEACON_KOSTA_EB:
		g_koatc.staAvoidOverrun = false;
		g_koatc.kosta2.RegPatt(3, g_location, g_koatc.koatcsta.StaPatt(g_location, optional));
		break;
	case ATS_BEACON_KOSTA_OVERRUN:
		if (g_koatc.kosta1.type & 2 || g_koatc.kosta2.type & 2) {
			g_koatc.staAvoidOverrun = true;
		}
		break;
	case ATS_BEACON_KOATC_SECTION:
		g_koatc.Confirm(false);
		g_koatc.kosecmgr.Reg(g_location, dist, optional);
		break;
	case ATS_BEACON_KOATC_SB:
		g_koatc.ko2step1.RegPatt(0, g_location, g_koatc.ORPStart(g_location, sig, dist, optional));
		break;
	case ATS_BEACON_KOATC_LIMIT_1:
		g_koatc.kolimit1.RegPatt(0, g_location, optional);
		break;
	case ATS_BEACON_KOATC_LIMIT_2:
		g_koatc.kolimit2.RegPatt(0, g_location, optional);
		break;
	case ATS_BEACON_KOATC_LIMIT_3:
		g_koatc.kolimit3.RegPatt(0, g_location, optional);
		break;
	case ATS_BEACON_KOATC_LIMIT_4:
		g_koatc.kolimit4.RegPatt(0, g_location, optional);
		break;
	case ATS_BEACON_MON_KMORG:
		g_mon.SetKmOrg(g_location);
		break;
	case ATS_BEACON_MON_STAORG:
		g_mon.SetTrainOrg(optional);
		break;
	case ATS_BEACON_MON_STADEST:
		g_mon.SetTrainDest(optional);
		break;
	case ATS_BEACON_MON_STANEXT:
		g_mon.SetNextSta(optional);
		break;
	case ATS_BEACON_MON_STANEXT2:
		g_mon.SetNext2Sta(optional);
		break;
	case ATS_BEACON_SET_PATTDCLR_SH:
	case ATS_BEACON_SET_PATTDCLR_SB:
	case ATS_BEACON_SET_PATTDCLR_EB:
		g_koatc.SetPattDclr(beaconindex - ATS_BEACON_SET_PATTDCLR_SH, optional);
	}
}

void PanelAssign(int *panel) {
	panel[21] = g_koatc.atcEnable;
	panel[22] = g_koatc.kosigmgr.emgBrk;
	panel[23] = g_koatc.svcBrk;
	panel[46] = g_koatc.atcEnable;
	panel[48] = g_koatc.atcConfirm;
	panel[101] = g_koatc.kosigmgr.atcX;
	panel[102] = g_koatc.atcBits & 1;	// atc0
	panel[0] = panel[102];
	panel[103] = g_koatc.atcBits & 2;	// atc5
	panel[104] = g_koatc.atcBits & 4;	// atc10
	panel[105] = g_koatc.atcBits & 8;	// atc15
	panel[5] = panel[105];
	panel[106] = g_koatc.atcBits & 16;	// atc20
	panel[107] = g_koatc.atcBits & 32;	// atc25
	panel[1] = panel[107];
	panel[108] = g_koatc.atcBits & 64;	// atc30
	panel[109] = g_koatc.atcBits & 128;	// atc35
	panel[110] = g_koatc.atcBits & 256;	// atc40
	panel[111] = g_koatc.atcBits & 512;	// atc45
	panel[2] = panel[111];
	panel[112] = g_koatc.atcBits & 1024;	// atc50
	panel[113] = g_koatc.atcBits & 2048;	// atc55
	panel[114] = g_koatc.atcBits & 4096;	// atc60
	panel[115] = g_koatc.atcBits & 8192;	// atc65
	panel[116] = g_koatc.atcBits & 16384;	// atc70
	panel[117] = g_koatc.atcBits & 32768;	// atc75
	panel[3] = panel[117];
	panel[118] = g_koatc.atcBits & 65536;	// atc 80
	panel[119] = g_koatc.atcBits & 131072;	// atc 85
	panel[120] = g_koatc.atcBits & 262144;	// atc 90
	panel[121] = g_koatc.atcBits & 524288;	// atc 95
	panel[122] = g_koatc.atcBits & 1048576;	// atc 100
	panel[123] = g_koatc.atcBits & 2097152;	// atc 105
	panel[124] = g_koatc.atcBits & 4194304;	// atc 110
	panel[4] = panel[124];
	// Display does not have atc115
	panel[125] = g_koatc.atcBits & 16777216;	// atc 120
	panel[131] = g_koatc.kosigmgr.red;
	panel[132] = g_koatc.kosigmgr.green;

	//TIMS
	panel[185] = g_mon.crntDigits[0];
	panel[186] = g_mon.crntDigits[1];
	panel[187] = g_mon.crntDigits[2];
	panel[188] = g_mon.crntDigits[3];
	panel[189] = g_mon.crntDigits[4];
	panel[190] = g_mon.cabDirection[0];
	panel[191] = g_mon.pwrDirection[0];
	panel[192] = g_mon.unitPower[0];
	panel[193] = g_mon.unitPower[1];
	panel[194] = g_mon.unitPower[2];
	panel[195] = g_mon.unitPower[3];
	panel[196] = g_mon.unitPower[4];
	panel[197] = g_mon.unitPower[5];
	panel[198] = g_mon.pwrDirection[1];
	panel[199] = g_mon.cabDirection[1];
	panel[200] = g_mon.speed;
	panel[201] = g_mon.spdDigits[0];
	panel[202] = g_mon.spdDigits[1];
	panel[203] = g_mon.spdDigits[2];
	panel[206] = g_mon.acclDigits[0];
	panel[207] = g_mon.acclDigits[1];
	panel[208] = g_mon.acclDigits[2];
	panel[209] = g_mon.acclDigits[3];
	panel[210] = g_mon.acclDigits[4];
	panel[211] = g_mon.timeDigits[0];
	panel[212] = g_mon.timeDigits[1];
	panel[213] = g_mon.timeDigits[2];
	panel[214] = g_mon.timeDigits[3];
	panel[215] = g_mon.timeDigits[4];
	panel[216] = g_mon.timeDigits[5];
	panel[217] = g_mon.bcPlus;
	panel[219] = g_koatc.koatcsta.trainClass;
	panel[220] = g_koatc.koatcsta.staStopNum;
	panel[221] = g_mon.locDigits[0];
	panel[222] = g_mon.locDigits[1];
	panel[223] = g_mon.locDigits[2];
	panel[224] = g_mon.locDigits[3];
	panel[225] = g_mon.locDigits[4];
	panel[230] = g_mon.trainClass;
	panel[231] = g_mon.trainIDd[0];
	panel[232] = g_mon.trainIDd[1];
	panel[233] = g_mon.trainIDd[2];
	panel[234] = g_mon.trainIDd[3];
	panel[235] = g_mon.trainIDd[4];
	panel[237] = g_mon.sta.org;
	panel[238] = g_mon.sta.dest;
	panel[239] = g_mon.sta.now;
	panel[240] = g_mon.sta.next;
	panel[251] = g_koatc.kosecmgr.isValid;
	panel[252] = g_koatc.kosigmgr.stopRC;
}
void SoundAssign(int *sound) {
	sound[2] = g_koatc.atcBellSt;
	sound[171] = g_koatc.atcBuzzSt;
}
