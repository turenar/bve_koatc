#include "stdafx.h"
#include "atsplugin.h"
#include "KoAtc.h"

void CKoAtc::CBarLimit::Reset(void)
{
	startLoc = 0.0f;
	coupleNum = 0;
	tooFast = false;
	
}
void CKoAtc::CBarLimit::BarLimit( double location, float speed, ATS_BEACONDATA *data)
{
	if ( speed != 0 && data->Signal == 0 && data->Optional != 0 ) {
		if ( coupleNum == data->Optional ) {
			double limit;
			limit = ( location - startLoc )* 3600 / data->Optional;
			if ( limit < speed ) tooFast = true;
			else tooFast = false;
		}
		else tooFast = false;
		startLoc = location;
		coupleNum = data->Optional;
	}
}
