#include "stdafx.h"
#include "Ats.h"
#include "atsplugin.h"
#include "KoAtc.h"

void CKoAtc::CKoAtcSta::Init(bool pwr) {
	enable = pwr;
	trainID = 10000;
	trainClass = 10;
	staStopNum = KOATC_STA_CANCEL;
	staStopLoc = -1.0f;
	staBuzzSt = ATS_SOUND_STOP;
}

void CKoAtc::CKoAtcSta::SetTrainID(int optional) {
	if (enable) {
		trainID = optional / 100;
		trainClass = trainID / 1000;
		staNow = optional % 100;
	}
}
void CKoAtc::CKoAtcSta::StaStop(int optional, int time, float speed) {
	if (enable && speed != 0.0f) {
		int class_num;
		int this_train_bin;
		int stop_train_bin;
		// Higher 4 digits shows the train class should be stopped
		stop_train_bin = (optional - optional % 100) / 100;

		// Highest digit of train number shows train class ( e.g. '5' = local )
		class_num = trainClass;
		this_train_bin = 1 << (class_num % 10);

		if ((this_train_bin & stop_train_bin) != 0) {
			staStopNum = optional % 100;	// Sta# 00 Shinjuku, 01 Sasazuka....
			// Buzzer set
			buzzEndTime = time + buzzLoop_ms * 3;
			staBuzzSt = ATS_SOUND_PLAYLOOPING;
		}
		// If the train class did not macth, keep previous state ( reset only performed by Stop & B7)
	}
}
int CKoAtc::CKoAtcSta::StaPatt(double location, int optional) {
	int out;
	if (enable && staStopNum != KOATC_STA_CANCEL) {
		staStopLoc = location;
		out = optional;
	} else out = KOATC_LIMIT_CANCEL;
	return out;
}
bool CKoAtc::CKoAtcSta::RunJob(double location, float speed, int bh_pos) {
	bool out;
	if (enable) {
		if (speed == 0.0f && numOfBrkNotches <= bh_pos && staStopNum == staNow) {
			staStopNum = KOATC_STA_CANCEL;
			out = true;
		} else out = false;
	} else out = false;
	return out;
}

void CKoAtc::CKoAtcSta::BuzzMgr(int time) {
	// buzz reset (1) Buzzer time expired (2) Buzzer time too old
	if (buzzEndTime < time || time < buzzEndTime - 10000) {
		staBuzzSt = ATS_SOUND_STOP;
	}
}