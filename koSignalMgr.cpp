#include "stdafx.h"
#include "atsplugin.h"
#include "KoAtc.h"

void CKoAtc::CSigMgr::Init() {
	atcX = false;
	green = false;
	red = false;
	bell = false;
	emgBrk = false;
	stopRC = 0;
}

// Update signal by clock
void CKoAtc::CSigMgr::UpdateSig(ATS_VEHICLESTATE *state) {
	bool atcXPrev, redPrev;

	// when train stopped
	if (emgBrk && state->Speed == 0) {
		emgBrk = false;
		stopRC = 0;
	}

	// Bell when signal changed
	currSigInt = newSigVal;
	if (emgBrk) currSig = 0;
	else currSig = currSigInt;

	atcXPrev = atcX;
	redPrev = red;
	if (currSig == 0) {
		atcX = true;
		red = false;
		green = false;
	} else if (currSig == 1) {
		atcX = false;
		red = true;
		green = false;
	} else {
		atcX = false;
		red = false;
		green = true;
	}
	if (atcX ^ atcXPrev || red ^ redPrev) bell = true;
	else bell = false;
}

// Get signal information from SetSignal func.
void CKoAtc::CSigMgr::SetSig(int newSignal) {
	newSigVal = newSignal;
	emgBrk = false;
	stopRC = 0;
}

void CKoAtc::CSigMgr::EmgInput(int source) {
	emgBrk = true;
	if (stopRC == 0) stopRC = source;
}

