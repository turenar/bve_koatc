#include "stdafx.h"
#include "atsplugin.h"
#include "KoAtc.h"
#include <math.h>

CKoAtc::CKoAtc(void)
{
}

CKoAtc::~CKoAtc(void)
{
}

void CKoAtc::SetVS( ATS_VEHICLESPEC *vehicleSpec, TCHAR *fullpath )
{
	int inidata[7];

	// Generate file path
	strcpy_s( iniPath, sizeof(iniPath), fullpath );
	strcat_s( iniPath, sizeof(iniPath) - strlen(iniPath), "ats.ini" );

	// Read from ini file
	genLog = GetPrivateProfileInt( TEXT("General"), TEXT("Log"), 0, iniPath );

	koatcsta.buzzLoop_ms = GetPrivateProfileInt( TEXT("ATC"), TEXT("BuzzLength "), 0, iniPath );
	if ( koatcsta.buzzLoop_ms <= 0 || 5000 <= koatcsta.buzzLoop_ms ) koatcsta.buzzLoop_ms = 750;	// For Satefy
	koatcsta.numOfBrkNotches = vehicleSpec ->BrakeNotches;

	// for debug ////////////////
	strcpy_s( logPath, sizeof(logPath), fullpath );
	strcat_s( logPath, sizeof(logPath) - strlen(logPath), "LogATC.txt" );
	if ( genLog != 0 ) {
		fopen_s( &outputfile, logPath, "w" );
		fclose(outputfile);
	}
	// Clock value [ms]
	atcClock = GetPrivateProfileInt( TEXT("ATC"), TEXT("Clock"), 0, iniPath );
	if ( atcClock <= 0 || 3000 < atcClock ) atcClock = 500;	// For Satefy

	// ATC Power
	if (  GetPrivateProfileInt( TEXT("ATC"), TEXT("UseATC"), 0, iniPath ) == 1 ) {
		atcEnable = true;
	}
	else {
		atcEnable = false;
	}

	numOfBrkNotches	= vehicleSpec->BrakeNotches;

	halfBrkApp	=	GetPrivateProfileInt( TEXT("ATC"), TEXT("HalfBrkApp"), 0, iniPath );
	if ( halfBrkApp < 0 ) halfBrkApp = 1000;	// for Safety
	
	inidata[0] = GetPrivateProfileInt( TEXT("ATC"), TEXT("Pattern1"), 0, iniPath );
	inidata[1] = GetPrivateProfileInt( TEXT("ATC"), TEXT("Pattern2"), 0, iniPath );
	inidata[2] = GetPrivateProfileInt( TEXT("ATC"), TEXT("Pattern3"), 0, iniPath );
	inidata[3] = GetPrivateProfileInt( TEXT("ATC"), TEXT("PatternOffset"), 0, iniPath );
	inidata[4] = GetPrivateProfileInt( TEXT("ATC"), TEXT("Intercep1"), 0, iniPath );
	inidata[5] = GetPrivateProfileInt( TEXT("ATC"), TEXT("Intercep2"), 0, iniPath );
	inidata[6] = GetPrivateProfileInt( TEXT("ATC"), TEXT("Intercep3"), 0, iniPath );

	kopattern.Init( inidata[0], inidata[1], inidata[2], inidata[3], inidata[4], inidata[5], inidata[6] );
	ko2step1.Init( inidata[0], inidata[1], inidata[2], inidata[3], inidata[4], inidata[5], inidata[6] );
	kolimit1.Init( inidata[0], inidata[1], inidata[2], inidata[3], inidata[4], inidata[5], inidata[6] );
	kolimit2.Init( inidata[0], inidata[1], inidata[2], inidata[3], inidata[4], inidata[5], inidata[6] );
	kolimit3.Init( inidata[0], inidata[1], inidata[2], inidata[3], inidata[4], inidata[5], inidata[6] );
	kolimit4.Init( inidata[0], inidata[1], inidata[2], inidata[3], inidata[4], inidata[5], inidata[6] );
	kosta1.Init( inidata[0], inidata[1], inidata[2], inidata[3], inidata[4], inidata[5], inidata[6] );
	kosta2.Init( inidata[0], inidata[1], inidata[2], inidata[3], inidata[4], inidata[5], inidata[6] );
	koatcmax.Init( inidata[0], inidata[1], inidata[2], inidata[3], inidata[4], inidata[5], inidata[6] );
	koatcmax.RegPatt( 0, 0, GetPrivateProfileInt( TEXT("ATC"), TEXT("ATCMax"), 0, iniPath ) );
	kosecmgr.Init();
	kosigmgr.Init();
}
void CKoAtc::Init(void) {
	if ( atcEnable ) {
		brkOutput = 0;
		svcBrk = false;
	}
	else {
		brkOutput = 0;
		svcBrk = false;
	}
	atcConfirm = false;
	pattType = 0;
	ORPdist	= 0;
	atcBits	= 0;
	atcBell	= false;
	atcBellSt= ATS_SOUND_STOP;
	atcBuzzSt= ATS_SOUND_STOP;
	pattEnd = -1;	// -1 means no pattern generates
	atcLimitUpper = 0;
	atcLimitLower = 0;
	pattBottom	= 0;
	lastTime = -1;
	halfBrkTimer = 0;
	koatcsta.Init( true );
	kobarlimit.Reset();
}
void CKoAtc::Run( ATS_VEHICLESTATE *state, int brake  )
{
	BellStop();
	koatcsta.BuzzMgr( state->Time );
	if ( kobarlimit.tooFast ) {
		kosigmgr.EmgInput(1);
		kobarlimit.Reset();
	}

	if ( clkNext <= state ->Time ||  state ->Time <= clkNext - 3000 ) {
		RunClock( state, brake );
		clkNext = state ->Time + atcClock;
	}
}

void CKoAtc::RunClock( ATS_VEHICLESTATE *state, int brake )
{
	BellHit();
	kosigmgr.UpdateSig( state );
	kosecmgr.Calc( state->Location, kosigmgr.currSig  );
	kopattern.RegPatt( kosecmgr.sectionOpt, state->Location, (int)kosecmgr.distanceToR*1000 );
	kopattern.CalcPatt( state->Location );
	if ( ORPdist + 50.0f < kosecmgr.locationOfR ) {
		ko2step1.RegPatt( 0, 0, KOATC_LIMIT_CANCEL_ABSOLUTE );
	}
	if ( koatcsta.RunJob( state->Location, state->Speed, brake ) ) {
		kosta1.RegPatt( 0, state->Location, KOATC_LIMIT_CANCEL_ABSOLUTE );
		kosta2.RegPatt( 0, state->Location, KOATC_LIMIT_CANCEL_ABSOLUTE );
	}
	ko2step1.CalcPatt( state->Location );
	kolimit1.CalcPatt( state->Location );
	kolimit2.CalcPatt( state->Location );
	kolimit3.CalcPatt( state->Location );
	kolimit4.CalcPatt( state->Location );
	kosta1.CalcPatt( state->Location );
	kosta2.CalcPatt( state->Location );
	koatcmax.CalcPatt( state->Location );

	SetPattern( state->Time, kopattern.type, kopattern.halfBrkLimit, kopattern.fullBrkLimit, kopattern.emgBrkLimit, kopattern.flatLimit, kopattern.pattDist );
	SetPattern( state->Time, ko2step1.type, ko2step1.halfBrkLimit, ko2step1.fullBrkLimit, ko2step1.emgBrkLimit, ko2step1.flatLimit, ko2step1.pattDist );
	SetPattern( state->Time, kolimit1.type, kolimit1.halfBrkLimit, kolimit1.fullBrkLimit, kolimit1.emgBrkLimit, kolimit1.flatLimit, kolimit1.pattDist );
	SetPattern( state->Time, kolimit2.type, kolimit2.halfBrkLimit, kolimit2.fullBrkLimit, kolimit2.emgBrkLimit, kolimit2.flatLimit, kolimit2.pattDist );
	SetPattern( state->Time, kolimit3.type, kolimit3.halfBrkLimit, kolimit3.fullBrkLimit, kolimit3.emgBrkLimit, kolimit3.flatLimit, kolimit3.pattDist );
	SetPattern( state->Time, kolimit4.type, kolimit4.halfBrkLimit, kolimit4.fullBrkLimit, kolimit4.emgBrkLimit, kolimit4.flatLimit, kolimit4.pattDist );
	SetPattern( state->Time, kosta1.type, kosta1.halfBrkLimit, kosta1.fullBrkLimit, kosta1.emgBrkLimit, kosta1.flatLimit, kosta1.pattDist );
	SetPattern( state->Time, kosta2.type, kosta2.halfBrkLimit, kosta2.fullBrkLimit, kosta2.emgBrkLimit, kosta2.flatLimit, kosta2.pattDist );
	SetPattern( state->Time, koatcmax.type, koatcmax.halfBrkLimit, koatcmax.fullBrkLimit, koatcmax.emgBrkLimit, koatcmax.flatLimit, koatcmax.pattDist );
	Output( state->Speed, kosigmgr.currSig, state->Time );
}


void CKoAtc::Confirm(bool confirm)
{
	if ( !atcConfirm && confirm && 15 < atcLimitUpper ) atcBell = true;
	atcConfirm = confirm;
}
void CKoAtc::BellStop(void)
{
	// bell to continue - must run every frame
	if ( atcBellSt == ATS_SOUND_PLAY ) {
		atcBellSt = ATS_SOUND_CONTINUE;	
	}
}
void CKoAtc::BellHit(void)
{
	if ( ( atcBell || kosigmgr.bell ) && atcEnable)  atcBellSt = ATS_SOUND_PLAY;
}

void CKoAtc::SetPattern( int time, int type, double spd1, double spd2, double spd3, int lim, double dist ) {
	// at new location ( reflesh in any case ) 
	if ( lastTime != time ) {
		lastTime = time;
		pattType = type;
		pattEnd = dist;
		halfBrkLimit = spd1;
		fullBrkLimit = spd2;
		emgBrkLimit = spd3;
		pattBottom = lim;
	}
	// at the same location: compare values then choose lower pattern
	// if valid pattern & lower pattern
	else if ( 0 <= spd1 && spd1 < halfBrkLimit ) {
		// overwrite pattern
		pattType = type;
		halfBrkLimit = spd1;
		fullBrkLimit = spd2;
		emgBrkLimit = spd3;
		pattEnd = dist;
		pattBottom = lim;
	}
	// Next pattern ( < 10km/h) Approach
	else if ( halfBrkLimit < spd1 && spd1 < halfBrkLimit +10 && lim <= pattBottom && pattType%2 != 1 ) {
		pattBottom = lim;
		if ( type%2 != 0 ) pattType |= 1;
	}
}

void CKoAtc::Output( float speed, int sig, int time  )
{
	int speedAbs;
	int atcLimitUpperPrev;

	speedAbs = abs( (int)speed );
	atcLimitUpperPrev = atcLimitUpper;

	// Confirm Running
	if ( atcConfirm ) {
		halfBrkLimit = 25.0f;
		fullBrkLimit = 25.0f;
		emgBrkLimit = 25.0f;
		atcLimitUpper = 25;
		atcLimitLower = 25;
	}
	// Safety for backward in case of station stop pattern
	else if ( ( pattType - pattType%2 )== 2 && speedAbs <= 5 ) {
		halfBrkLimit = 5.0f;
		fullBrkLimit = 5.0f;
		emgBrkLimit = 6.0f;
		atcLimitUpper = 5;
		atcLimitLower = 5;
	}
	// section shows zero (X)
	else if ( 	kosigmgr.currSig == 0 )	{
		atcLimitUpper = 0;
		atcLimitLower = 0;
		halfBrkLimit = 0;
		fullBrkLimit = 0;
		emgBrkLimit = 0;
	}
	// approaching to pattern
	else {
		// ORP 2nd pattern
		if ( (pattType%2)== 1 ) {
			if ( 25 < emgBrkLimit ) {
				// higher indicator shound not exceed 25 km/h
					atcLimitUpper = 25;
					atcLimitLower = 0;
			}
			else if ( emgBrkLimit < 5.0f || speedAbs <= 5 ) {
				emgBrkLimit = 5.0f;
				atcLimitUpper = 5;
				atcLimitLower = 5;
			}
			else  {
				// higher indicator = speed limit
				atcLimitUpper = (int)emgBrkLimit;
				atcLimitLower = 0;
			}
			// lower indicator = bottom
			pattBottom = 0;
		}
		// near section ( pattern < 5km/h )
		else if  ( halfBrkLimit < 5.0f ) {
			halfBrkLimit = 5.0;
			fullBrkLimit = 5.0;
			emgBrkLimit = 6.0;
			atcLimitUpper = 5;
			atcLimitLower = 5;
		}
		// stopping near red signal
		else if (  speedAbs == 0 && kosigmgr.currSig == 1 && kosecmgr.sectionOpt != 1 ) {
			halfBrkLimit = 0;
			fullBrkLimit = 0;
			emgBrkLimit = 6.0;
			atcLimitUpper = 0;
			atcLimitLower = 0;
		}
		// normal pattern
		else {
			// higher indicator = speed limit
			atcLimitUpper = (int)halfBrkLimit;
	
			// lower indicator = bottom
			if ( pattBottom < atcLimitUpper )	atcLimitLower = pattBottom;
			else atcLimitLower = atcLimitUpper;
		}
	}

	// Generate Brake Output
	// ATC not active
	if ( !atcEnable ) {
		brkOutput = 0;
		svcBrk	= false;
	}
	// Emergency brake
	else if ( emgBrkLimit < speedAbs || ( kosigmgr.currSig == 0 && !atcConfirm) ) {
		kosigmgr.EmgInput(2);
		brkOutput = numOfBrkNotches + 1;
		svcBrk = false;
	}
	// Not emergency brake
	else {
		// 2 Step pattern does not apply service brake
		if ( (pattType%2)== 1 &&  pattBottom ==0  || kosecmgr.sectionOpt == 1 ) {
			brkOutput = 0;
			svcBrk = false;
		}
		// Full Brake
		// (1) Speed too fast (2) Half brake but timer passed.
		// (3) Stopping near the red signal
		else if ( fullBrkLimit <  speedAbs ||
			( 0 < halfBrkTimer ) && ( halfBrkTimer < time ) && (halfBrkLimit < speedAbs) ||
			( speedAbs == 0 && kosigmgr.currSig == 1 && !atcConfirm )	) {
			brkOutput = numOfBrkNotches;
			svcBrk = true;
		}
		// Half Brake w/ Timer
		else if ( halfBrkLimit < speedAbs ) {
			svcBrk = true;
			brkOutput = (int)numOfBrkNotches/2;
			if ( halfBrkTimer == 0 )	halfBrkTimer = time + halfBrkApp;
		}
		// Half Brake w/o Timer near the red signal
		else if ( speedAbs < 5 && kosigmgr.currSig == 1 && !atcConfirm ) {
			svcBrk = true;
			brkOutput = (int)numOfBrkNotches/2;
		}
		// No brake applied
		else {
			brkOutput = 0;
			svcBrk = false;
		}
	}
	if ( brkOutput == 0 || brkOutput == numOfBrkNotches + 1 ) halfBrkTimer = 0;
	// End of Brake output

	// Sound
	// Bell
	if ( atcLimitUpperPrev < atcLimitUpper ) atcBell = true;
	else atcBell = false;

	//Buzzer
	if ( ( atcLimitLower == 0 && (pattType%2)== 1 && 5 <= speedAbs ) ||
		koatcsta.staBuzzSt == ATS_SOUND_PLAYLOOPING ||
		atcConfirm && 5 <= speedAbs ) {
		atcBuzzSt = 	ATS_SOUND_PLAYLOOPING;
	}
	else {
		atcBuzzSt = 	ATS_SOUND_STOP;
	}

	// Generate Indicator
	Timeshock();
}
void CKoAtc::Timeshock()
{
	// Generate ATC limit indicator
	if ( atcEnable ) {
		int i, j;

		// calculate atc sig position
		i = atcLimitLower / 5;
		j = atcLimitUpper / 5;
		atcBits = 0;
		for ( i; i <= j; i++ ) {
			atcBits += (unsigned long)pow( 2.0f, i );		// position to bit array
		}
	}
}
int CKoAtc::ORPStart( double location, int signal, float distance, int optional )
{
	int Out;

	if ( signal == 0 ) {
		ORPdist = location + distance;
		Out = optional;
	}
	else {
		Out = KOATC_LIMIT_CANCEL_ABSOLUTE;
	}
	return Out;
}
void CKoAtc::SetNewSig( double location, int signal )
{
	kosigmgr.SetSig( signal );
}
