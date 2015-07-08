#include "stdafx.h"
#include "atsplugin.h"
#include "Monitor.h"
#include <math.h>
#include <time.h>

void CMon::SetVS(ATS_VEHICLESPEC *vehicleSpec, TCHAR *fullpath) {
	// Generate file path
	strcpy_s(iniPath, sizeof(iniPath), fullpath);
	strcat_s(iniPath, sizeof(iniPath) - strlen(iniPath), "ats.ini");

	// Read from ini file
	genLog = GetPrivateProfileInt(TEXT("General"), TEXT("Log"), 0, iniPath);

	spdThreshold = GetPrivateProfileInt(TEXT("Monitor"), TEXT("SpdThreshold"), 0, iniPath);
	if (spdThreshold <= 0 || 10000 <= spdThreshold) spdThreshold = 1000;	// For Satefy

	cscThreshold = GetPrivateProfileInt(TEXT("Monitor"), TEXT("CSCThreshold"), 0, iniPath);
	if (cscThreshold <= 0) cscThreshold = 0;	// For Satefy
	cscPosition = GetPrivateProfileInt(TEXT("Monitor"), TEXT("CSCPosition"), 0, iniPath);
	if (cscPosition <= 0) cscPosition = 0;	// For Satefy
	lbTimer = GetPrivateProfileInt(TEXT("Monitor"), TEXT("LBTimer"), 0, iniPath);

	staDefault = GetPrivateProfileInt(TEXT("Monitor"), TEXT("StaDefault"), 0, iniPath);
	if (staDefault < 0) staDefault = 0;	// For Satefy
	trainClass = GetPrivateProfileInt(TEXT("Monitor"), TEXT("TrainClassDefault"), 0, iniPath);
	if (trainClass < 0) trainClass = 0;	// For Satefy

	// for debug ////////////////
	strcpy_s(logPath, sizeof(logPath), fullpath);
	strcat_s(logPath, sizeof(logPath) - strlen(logPath), "Monitor.txt");
	if (genLog != 0) {
		fopen_s(&outputfile, logPath, "w");
		fclose(outputfile);
	}
}
void CMon::Init(bool enable) {
	int i;
	for (i = 0; i < 2; i++) {
		cabDirection[i] = 0;	pwrDirection[i] = 0;
	}
	for (i = 0; i < 3; i++) {
		spdDigits[i] = 0;
	}
	for (i = 0; i < 5; i++) {
		locDigits[i] = 0;
		crntDigits[i] = 0;
		acclDigits[i] = 0;
	}
	for (i = 0; i < 6; i++) {
		timeDigits[i] = 0;
		unitPower[i] = 0;
	}
	trainLocBase = 0.0f;

	trainID = 10000;

	srand((unsigned) time(NULL));

	clkNext1000 = 0;
	clkNext250 = 0;

	spdPrev = 0.0f;
	timePrev = 0;

	sta.org = staDefault;
	sta.prev = staDefault;
	sta.now = staDefault;
	sta.next = staDefault;
	sta.next2 = staDefault;
	sta.dest = staDefault;

	ctrl.csc = ATS_CONSTANTSPEED_CONTINUE;
	cscActive = false;
	cscReset = false;

	if (enable) {
	} else {
	}
}
void CMon::Run(ATS_VEHICLESTATE *state, ATS_HANDLES *handle) {
	// Process by clock
	Clocker(state);

	// Process anytime
	// speedometer need
	GenSpdMtr(state->Speed);

	bcPlus = (int) (state->BcPressure * 1000);
}
void CMon::Clocker(ATS_VEHICLESTATE *state) {
	if (clkNext250 <= state->Time || state->Time <= clkNext250 - 10000) {
		RunClock250(state);
		clkNext250 = state->Time + 250;
	}
	if (clkNext1000 <= state->Time || state->Time <= clkNext1000 - 10000) {
		RunClock1000(state);
		clkNext1000 = state->Time + 1000;
	}
}
void CMon::RunClock250(ATS_VEHICLESTATE *state) {
	CSC(state);
}
void CMon::RunClock1000(ATS_VEHICLESTATE *state) {
	int tmp;

	// Digital Clock
	tmp = (int) (state->Time / 1000);
	timeDigits[0] = tmp % 10;
	timeDigits[1] = ((tmp - timeDigits[0]) / 10) % 6;
	timeDigits[2] = ((tmp - timeDigits[0] - timeDigits[1]) / 10 / 6) % 10;
	timeDigits[3] = ((tmp - timeDigits[0] - timeDigits[1] - timeDigits[2]) / 10 / 6 / 10) % 6;
	timeDigits[4] = ((tmp - timeDigits[0] - timeDigits[1] - timeDigits[2] - timeDigits[3]) / 10 / 6 / 10 / 6) % 10;
	timeDigits[5] = ((tmp - timeDigits[0] - timeDigits[1] - timeDigits[2] - timeDigits[4]) / 10 / 6 / 10 / 6 / 10) % 10;
	if (timeDigits[5] == 0) timeDigits[5] = MON_WS_SP;

	// Digital Speed Display
	tmp = (int) (speed / 1000);
	spdDigits[0] = (tmp % 1000) / 100;
	spdDigits[1] = (tmp % 100) / 10;
	spdDigits[2] = tmp % 10;
	if (spdDigits[0] == 0) {
		spdDigits[0] = MON_WS_SP;
		if (spdDigits[1] == 0) {
			spdDigits[1] = MON_WS_SP;
		}
	}
	// Train Location
	tmp = abs((int) (state->Location - trainLocBase));
	locDigits[0] = (tmp % 1000) / 100;
	locDigits[1] = MON_WS_DT;
	locDigits[2] = (tmp % 10000) / 1000;
	locDigits[3] = (tmp % 100000) / 10000;
	locDigits[4] = (tmp % 1000000) / 100000;
	if (locDigits[4] == 0) {
		locDigits[4] = MON_WS_SP;
		if (locDigits[3] == 0) {
			locDigits[3] = MON_WS_SP;
		}
	}
	// Train ID
	if (trainID < 0) {
		trainIDd[0] = MON_WS_QL;
		trainIDd[1] = MON_WS_QS;
		trainIDd[2] = MON_WS_QS;
		trainIDd[3] = MON_WS_QS;
		trainIDd[4] = MON_WS_QR;
	} else {
		trainIDd[0] = (trainID % 10000) / 1000;
		trainIDd[1] = (trainID % 1000) / 100;
		trainIDd[2] = (trainID % 100) / 10;
		trainIDd[3] = trainID % 10;
		trainIDd[4] = MON_WS_SP;
	}
	// Cab Direction [0] = Motoyawata [1] = Hashimoto
	if (trainID % 2 == 0) {
		cabDirection[0] = 1;	cabDirection[1] = 0;
	} else {
		cabDirection[0] = 0;	cabDirection[1] = 1;
	}
	// Unit state
	if (0 < state->Current) {
		int i;
		for (i = 0; i < 9; i++) {
			if (unitPower[i] != 1 && rand() % 4 != 0) {
				unitPower[i] = 1;
			}
		}
	} else if (state->Current < 0) {
		int i;
		for (i = 0; i < 9; i++) {
			if (unitPower[i] != 2 && rand() % 4 != 0) {
				unitPower[i] = 2;
			}
		}
	} else {
		int i;
		for (i = 0; i < 9; i++) {
			if (unitPower[i] != 0 && rand() % 4 != 0) {
				unitPower[i] = 0;
			}
		}
	}
	if (0 < ctrl.rvs) {
		pwrDirection[0] = cabDirection[0] * (0 < state->Current);
		pwrDirection[1] = cabDirection[1] * (0 < state->Current);
	}
	if (ctrl.rvs < 0) {
		pwrDirection[0] = cabDirection[1] * (0 < state->Current);
		pwrDirection[1] = cabDirection[0] * (0 < state->Current);
	}
	// Current
	if (state->Current < 0) {
		crntDigits[0] = MON_WS_HY;
		tmp = 0 - (int) state->Current;
	} else {
		crntDigits[0] = MON_WS_SP;
		tmp = (int) state->Current;
	}
	crntDigits[1] = (tmp % 10000) / 1000;
	crntDigits[2] = (tmp % 1000) / 100;
	crntDigits[3] = (tmp % 100) / 10;
	crntDigits[4] = tmp % 10;
	if (crntDigits[1] == 0) {
		crntDigits[1] = MON_WS_SP;
		if (crntDigits[2] == 0) {
			crntDigits[2] = MON_WS_SP;
			if (crntDigits[3] == 0) {
				crntDigits[3] = MON_WS_SP;
			}
		}
	}

	// Acceralate
	tmp = state->Time - timePrev;
	if (tmp != 0 && speed != 0) {
		float tmp_f;
		tmp_f = (state->Speed - spdPrev) / tmp * 1000;

		if (tmp_f < 0) {
			acclDigits[0] = MON_WS_HY;
		} else {
			acclDigits[0] = MON_WS_SP;
		}
		if (tmp_f < 0) tmp_f = 0 - tmp_f;
		acclDigits[1] = (int) tmp_f;
		acclDigits[2] = MON_WS_DT;
		acclDigits[3] = (int) (tmp_f * 10.0f) % 10;
		acclDigits[4] = (int) (tmp_f * 100.0f) % 10;
	} else {
		acclDigits[0] = MON_WS_SP;
		acclDigits[1] = 0;
		acclDigits[2] = MON_WS_DT;
		acclDigits[3] = 0;
		acclDigits[4] = 0;
	}
	spdPrev = state->Speed;
	timePrev = state->Time;

}
void CMon::GenSpdMtr(float spdsys) {
	speed = abs((int) (spdsys * 1000));
	if (speed < spdThreshold) speed = 0;
	else if (spdThreshold < speed && speed < spdThreshold*1.4) speed = (int) (spdThreshold * 1.4);
}
void CMon::SetTrainID(int optional) {
	trainID = optional / 100;
	trainClass = trainID / 1000;
	sta.prev = sta.now;
	sta.now = optional % 100;
}
void CMon::SetKmOrg(double location) {
	trainLocBase = location;
}
void CMon::SetTrainOrg(int optional) {
	sta.org = optional;
}
void CMon::SetTrainDest(int optional) {
	sta.dest = optional;
}
void CMon::SetNextSta(int optional) {
	sta.next = optional % 100;
}
void CMon::SetNext2Sta(int optional) {
	sta.next2 = optional % 100;
}
void CMon::CSC(ATS_VEHICLESTATE *state) {
	if (cscActive && ctrl.csc != ATS_CONSTANTSPEED_ENABLE && cscPosition <= ctrl.mc && cscThreshold <= state->Speed) {
		ctrl.csc = ATS_CONSTANTSPEED_ENABLE;
	} else if (ctrl.mc < cscPosition || cscReset) {
		ctrl.csc = ATS_CONSTANTSPEED_DISABLE;
	} else {
		ctrl.csc = ATS_CONSTANTSPEED_CONTINUE;
	}
	cscActive = false;
}
void CMon::MCInput(int mcpos) {
	// Moved to lower position
	if (mcpos < ctrl.mc) {
		cscActive = true;
		cscReset = false;
	}
	// Moved to higher position
	else if (ctrl.mc < mcpos) {
		cscActive = false;
		cscReset = true;
	}
	// Same Position
	else {
		cscActive = false;
		cscReset = false;
	}
	ctrl.mc = mcpos;
}
void CMon::BHInput(int brkpos) {
	ctrl.brk = brkpos;
	cscActive = false;
}
void CMon::RVSInput(int rvspos) {
	ctrl.rvs = rvspos;
	cscActive = false;
}
void CMon::DoorOpn(void) {
	if (sta.now == sta.next) {
		if (trainID % 2 == 1) sta.next++;
		else sta.next--;
	}
	if (sta.next == sta.next2) {
		if (trainID % 2 == 1) sta.next2++;
		else sta.next2--;
	}
}
void CMon::DoorCls(void) {
}