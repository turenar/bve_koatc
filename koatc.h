#pragma once
#define MAX_PATTERNS 4
#define KOATC_PRIORITY_PT2 1
#define KOATC_STA_CANCEL 99
#define KOATC_LIMIT_CANCEL 999
#define KOATC_LIMIT_CANCEL_ABSOLUTE -1999
#define ATS_BEACON_KOATC_SECTION 170 // 距離更新
#define ATS_BEACON_KOATC_BARLIMIT 0 // 点速照(ATSと共通)
#define ATS_BEACON_KOATC_SB   172 // 2段パターン(常用B)
#define ATS_BEACON_KOATC_EB   173 // 2段パターン(非常B) → 2012/7/5廃止
#define ATS_BEACON_KOATC_LIMIT_1 176 // 速度制限1
#define ATS_BEACON_KOATC_LIMIT_2 177 // 速度制限2
#define ATS_BEACON_KOATC_LIMIT_3 178 // 速度制限3
#define ATS_BEACON_KOATC_LIMIT_4 179 // 速度制限4
#define ATS_BEACON_KOSTA_NOW 180	// 列番・位置設定
#define ATS_BEACON_KOSTA_STOP 181	// 駅停車制御開始/終了
#define ATS_BEACON_KOSTA_SB 182	// 駅停車パターン(常用B)
#define ATS_BEACON_KOSTA_EB 183	// 駅停車パターン(非常B)
#define ATS_BEACON_KOSTA_OVERRUN 184 // 過走防止点照査
#define ATS_BEACON_MON_KMORG 	190// 距離ゼロ設定
#define ATS_BEACON_MON_STAORG 	191// 始発駅設定
#define ATS_BEACON_MON_STADEST	192// 行先設定
#define ATS_BEACON_MON_STANEXT 193	// 次駅設定
#define ATS_BEACON_MON_STANEXT2	194	// 次々駅設定

#define ATS_BEACON_SET_PATTDCLR_SH 201
#define ATS_BEACON_SET_PATTDCLR_SB 202
#define ATS_BEACON_SET_PATTDCLR_EB 203

////////////////////////////////////////////////////////
class CKoAtc {
public:
	class CPattern;
private:
	TCHAR iniPath[_MAX_PATH];
	TCHAR logPath[_MAX_PATH];
	char debugstr[1000];
	FILE *outputfile;
	int atcClock;
	int clkNext;
	int genLog;

	int numOfBrkNotches;
	int pattType;
	double pattEnd;
	int atcLimitUpper;
	int atcLimitLower;
	int lastTime;
	int halfBrkTimer;
	int halfBrkApp;
	double halfBrkLimit;
	double fullBrkLimit;
	double emgBrkLimit;
	int pattBottom;
	bool atcBell;
	double ORPdist;

	void BellStop(void);
	void BellHit(void);
	void SetPattern(ATS_VEHICLESTATE*, CPattern&, bool);
	void Timeshock();
	void Output(float, int, int);
	void RunClock(ATS_VEHICLESTATE*, int brake);

public:
	bool atcEnable;
	bool atcConfirm;
	bool staAvoidOverrun = false;
	unsigned long atcBits;
	int brkOutput;
	bool svcBrk;
	double stopLoc;
	int limitSpd;
	int atcBellSt;
	int atcBuzzSt;

	CKoAtc(void);
	virtual ~CKoAtc(void);
	void Init();
	void SetVS(ATS_VEHICLESPEC*, TCHAR*);
	void Run(ATS_VEHICLESTATE*, int);
	void Confirm(bool);
	int ORPStart(double, int, float, int);
	void SetNewSig(double, int);
	void SetPattDclr(int, int);

	////////////////////////////////////////////////////////
	class CSigMgr {
	private:
		int newSigVal;
		int currSigInt;

	public:
		int currSig;
		bool atcX;
		bool green;
		bool red;
		bool bell;
		bool emgBrk;
		int stopRC;

		void Init(void);
		void UpdateSig(ATS_VEHICLESTATE*);
		void SetSig(int);
		void EmgInput(int);
	};

	////////////////////////////////////////////////////////
	class CSectMgr {
	private:
		int ptr;
		double location;
		double sectLocs[10];
		int sectOpts[10];

	public:
		double distanceToR;
		double locationOfR;
		int sectionOpt;
		int isValid;
		void Init();
		void Reg(double, float, int);
		void Calc(double, int);
	};

	////////////////////////////////////////////////////////
	class CPattern {
	private:
		int pattBottom;
		double flatPattEnd;
		float dclrHalf;
		float dclrFull;
		float dclrEmg;
		float intrcpHalf;
		float intrcpFull;
		float intrcpEmg;

	public:
		int type;
		int flatLimit;
		float pattOffset;
		double pattEnd;
		double pattEndZero;
		double pattDist;
		double halfBrkLimit;
		double fullBrkLimit;
		double emgBrkLimit;
		void Init(int, int, int, int, int, int, int);
		void RegPatt(int, double, int);
		void ChangeDclr(int, int);
		void CalcPatt(double);
		void SetPattDclr(int, int);
	};
	////////////////////////////////////////////////////////
	class CKoAtcSta {
	private:
		int trainID;
		int staNow;
		double staStopLoc;
		int buzzEndTime;

	public:
		bool enable;
		int buzzLoop_ms;
		int trainClass;
		int staStopNum;
		int staBuzzSt;
		int numOfBrkNotches;

		void SetVS(ATS_VEHICLESPEC*, TCHAR*);
		void Init(bool);
		void SetTrainID(int);
		void StaStop(int, int, float);
		int StaPatt(double, int);
		bool RunJob(double, float, int);
		void BuzzMgr(int);
	};
	////////////////////////////////////////////////////////
	class CBarLimit {
	private:
		double startLoc;
		int coupleNum;
	public:
		bool tooFast;
		void Reset();
		void BarLimit(double, float, ATS_BEACONDATA*);
	};
	////////////////////////////////////////////////////////
	CKoAtc::CSectMgr kosecmgr;
	CKoAtc::CSigMgr kosigmgr;
	CKoAtc::CPattern kopattern;
	CKoAtc::CPattern ko2step1;
	CKoAtc::CPattern kolimit1;
	CKoAtc::CPattern kolimit2;
	CKoAtc::CPattern kolimit3;
	CKoAtc::CPattern kolimit4;
	CKoAtc::CPattern kosta1;
	CKoAtc::CPattern kosta2;
	CKoAtc::CPattern koatcmax;
	CKoAtc::CKoAtcSta koatcsta;
	CKoAtc::CBarLimit kobarlimit;
};
