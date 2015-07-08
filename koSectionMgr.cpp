#include "stdafx.h"
#include "atsplugin.h"
#include "KoAtc.h"

void CKoAtc::CSectMgr::Init() {
	ptr = 0;
	location = 0;
	isValid = 0;
}
void CKoAtc::CSectMgr::Reg(double newLocation, float newDistance, int optional) {
	int loc_diff;

	loc_diff = (int) (location - newLocation);
	if (loc_diff != 0) {
		location = newLocation;
		sectLocs[0] = newDistance + location;
		sectOpts[0] = optional;
		ptr = 1;
	} else if (ptr < 10) {
		sectLocs[ptr] = newDistance + location;
		sectOpts[ptr] = optional;
		ptr++;
	}
}

// Calculate current section value
void CKoAtc::CSectMgr::Calc(double location, int sectNum) {
	// If invalid input
	if (sectNum < 0) {
		distanceToR = sectLocs[0] - location;
		locationOfR = sectLocs[0];
		sectionOpt = sectOpts[0];
		isValid = 2;
	} else if (ptr <= sectNum) {
		distanceToR = sectLocs[ptr] - location;
		locationOfR = sectLocs[ptr];
		sectionOpt = sectOpts[ptr];
		isValid = 1;
	}
	// normal input, pull from array
	else {
		distanceToR = sectLocs[sectNum] - location;
		locationOfR = sectLocs[sectNum];
		sectionOpt = sectOpts[sectNum];
		isValid = 1;
	}
}
