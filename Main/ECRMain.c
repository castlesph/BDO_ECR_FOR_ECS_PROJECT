/*******************************************************************************

*******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdarg.h>

//#include "../Includes/EMVTypedef.h"
#include "../Includes/Utils.h"
#include "../Includes/DataBaseFunc.h"
//#include "../Includes/EMVTrans.h"
#include "../Includes/debug.h"
#include "../ECR/MultiAptrans.h"
#include "../ECR/ECRTrans.h"

int main(int argc,char *argv[])
{
	inSetTextMode();
    inTCTRead(1);
	int inMainBusy;
    
	vdDebug_LogPrintf("-----Share ECR main-----"); 

//	inECR_InitCOM();

    while(1)
    {   
        inMultiAP_GetMainroutine();
		//CTOS_Delay(1000);
		CTOS_Delay(100);//enhance the receive time

		//inMainBusy = get_env_int("CREDITBUSY");


		//if (inMainBusy != 0)
		//			continue;

		vdDebug_LogPrintf("-----inCTOSS_CheckECREvent-----"); 
		inCTOSS_CheckECREvent();
//        usleep(100*1000);
    }
}

