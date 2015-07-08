#define MON_WS_SP 10;
#define MON_WS_DT 11;
#define MON_WS_HY 12;
#define MON_WS_QL 13;
#define MON_WS_QR 14;
#define MON_WS_QS 15;

class CMon {
private:
	TCHAR iniPath[_MAX_PATH];
	TCHAR logPath[_MAX_PATH];
	char debugstr[1000];
	FILE *outputfile;
	int clkNext250;
	int clkNext1000;
	int genLog;
	int spdThreshold;
	double trainLocBase;
	int trainID;
	int staDefault;
	int cscThreshold, cscPosition;
	bool cscActive, cscReset;
	int lbTimer;
	float spdPrev;
	int timePrev;

	// call from only "run" func. 
	void Clocker( ATS_VEHICLESTATE* );
	void RunClock250( ATS_VEHICLESTATE* );
	void RunClock1000( ATS_VEHICLESTATE* );
	void GenSpdMtr( float );
	void CSC( ATS_VEHICLESTATE* );

public:
	int speed;
	int spdDigits[3];
	int bcPlus;
	int timeDigits[6];
	int locDigits[5];
	int trainClass;
	int trainIDd[5];
	int unitPower[6];
	int crntDigits[5];
	int acclDigits[5];
	int cabDirection[2];
	int pwrDirection[2];
	struct ctrlOutput {
		int rvs;
		int brk;
		int mc;
		int csc;
	} ctrl;
	struct stations {
		int org;
		int prev;
		int now;
		int next;
		int next2;
		int dest;
	} sta;
	void SetVS( ATS_VEHICLESPEC*, TCHAR* );
	void Init( bool );
	void Run( ATS_VEHICLESTATE*, ATS_HANDLES* );
	void SetTrainID( int );
	void MCInput( int );
	void BHInput( int );
	void RVSInput( int );
	void SetKmOrg( double );
	void SetTrainOrg( int );
	void SetTrainDest( int );
	void SetNextSta( int );
	void SetNext2Sta( int );
	void DoorOpn( void );
	void DoorCls( void );
};
