#include "stdafx.h"
#include "atsplugin.h"
#include "KoAtc.h"

void CKoAtc::CPattern::Init(int dclr1, int dclr2, int dclr3, int offset, int intrcp1, int intrcp2, int intrcp3) {
	// safety value against ini input
	dclrHalf = 2.0f; 	dclrFull = 2.7f; 	dclrEmg = 3.5f;

	// check value & set deceleration value
	if (1000 < dclr1 && dclr1 < 9999) dclrHalf = dclr1 / 1000.0f;
	if (1000 < dclr2 && dclr2 < 9999 && dclr1 < dclr2) dclrFull = dclr2 / 1000.0f;
	if (1000 < dclr3 && dclr3 < 9999 && dclr2 < dclr3) dclrEmg = dclr3 / 1000.0f;
	pattOffset = offset / 1000.0f;
	pattBottom = -1;
	intrcpHalf = intrcp1 / 1000.0f;
	intrcpFull = intrcp2 / 1000.0f;
	intrcpEmg = intrcp3 / 1000.0f;
	flatLimit = -1;
	flatPattEnd = 0;
	pattDist = 0;
}
void CKoAtc::CPattern::RegPatt(int ty, double location, int opt) {
	// cancel existing pattern
	if (opt == KOATC_LIMIT_CANCEL) {
		flatPattEnd = location;
	}
	// new pattern
	else {
		type = ty;
		pattBottom = opt % 1000;
		flatLimit = pattBottom;
		pattEnd = location + (int) (opt / 1000);
		pattEndZero = pattEnd + ((flatLimit * flatLimit) / (7.2f * 3.5f));
		flatPattEnd = 0;	// clear prevous pattern
	}
}
void CKoAtc::CPattern::CalcPatt(double location) {
	flatLimit = pattBottom;
	pattDist = pattEndZero - location;
	if (pattDist < 0) pattDist = 0.0f;

	// if pattern inactive
	// 1st & 2nd criteria means this pattern was expired, or 3rd = inactive pattern
	if ((0 < flatPattEnd && flatPattEnd <= location) || flatLimit < 0) {
		halfBrkLimit = -1;
		fullBrkLimit = -1;
		emgBrkLimit = -1;
	}
	// calc pattern - curving
	else if (0 < pattEnd - location) {
		double pattDistAdj = pattDist + pattOffset;

		if (pattDistAdj < 0) {
			pattDistAdj = 0.0f;
		}
		halfBrkLimit = sqrt(2.0f * (dclrHalf / 3.6f) * pattDistAdj) * 3.6f + intrcpHalf;
		fullBrkLimit = sqrt(2.0f * (dclrFull / 3.6f) * pattDistAdj) * 3.6f + intrcpFull;
		emgBrkLimit = sqrt(2.0f * (dclrEmg / 3.6f) * pattDistAdj) * 3.6f + intrcpEmg;

		// Adjust border of flat-limit and pattern (above calulated value might be underrun the flat limit)
		if (halfBrkLimit < flatLimit) {
			halfBrkLimit = flatLimit;
			fullBrkLimit = flatLimit + (intrcpFull - intrcpHalf);
			emgBrkLimit = flatLimit + (intrcpEmg - intrcpFull);
		}
	}
	// if pattern inactive
	// other than aboves, train runs under flat-limit:
	else {
		halfBrkLimit = flatLimit;
		fullBrkLimit = flatLimit + (intrcpFull - intrcpHalf);
		emgBrkLimit = flatLimit + (intrcpEmg - intrcpFull);
	}
}

void CKoAtc::CPattern::SetPattDclr(int pattIndex, int dclrIndex) {
	switch (pattIndex) {
	case 0:
		dclrHalf = dclrIndex / 1000.f;
		break;
	case 1:
		dclrFull = dclrIndex / 1000.f;
		break;
	case 2:
		dclrEmg = dclrIndex / 1000.f;
		break;
	}
}
