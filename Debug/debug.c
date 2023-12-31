#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctosapi.h>
#include <ctosapi.h>
#include <stdarg.h>
#include <typedef.h>

#include "../Includes/Utils.h"
#include "../Includes/MyFileFunc.h"
#include "../Includes/MultiAplib.h"
#include "../Includes/ECRTypedef.h"

static BYTE ifDebugMode = FALSE;

#define MAX_DEBUF_BUF 4096

BYTE DebugLog[4096 + 2];
LONG DebugLen;
CTOS_RTC stRTC;
INT iDebugTOTimes = 0;
BYTE DebugPort = d_DEBUG_PORT;

BYTE byGetDebugMode(void)
{
    return ifDebugMode;
}

/****************
 * if bPort == 0xFF --> USB mode
 ****************/
void SetDebugMode(BYTE bMode, BYTE bPort)
{
	if (0 ==strTCT.byRS232DebugPort)
	{
		ifDebugMode = FALSE;
		return;
	}
	else
		ifDebugMode = TRUE;
}

void DebugInit(void)
{
    
    if (!ifDebugMode) return;

    DebugLen = 0;

 //   if (DebugPort == 0xFF)
	if (8 ==strTCT.byRS232DebugPort)
	{
		DebugPort = 0xFF;
      	CTOS_USBOpen();
	}
	
    if (1 ==strTCT.byRS232DebugPort)
    {
    	DebugPort= d_COM1;
      	CTOS_RS232Open(DebugPort, 115200, 'N', 8, 1);
    }

	if (2 ==strTCT.byRS232DebugPort)
	{
		DebugPort= d_COM2;
      	CTOS_RS232Open(DebugPort, 115200, 'N', 8, 1);
	}
}

void DebugExport232(void)
{
	ULONG tick;
	USHORT ret;
	
	if (!ifDebugMode) return;
	
	tick = CTOS_TickGet();
	do {
        if (DebugPort == 0xFF)
          ret = CTOS_USBTxReady();
        else
          ret = CTOS_RS232TxReady(DebugPort);
		if (ret == d_OK)
			break;
		//CTOS_Delay(50);
	} while ((CTOS_TickGet() - tick) < d_READY_TIMEOUT);
	
	if (ret == d_OK) {
		DebugLog[DebugLen++] = 0x0D;
		DebugLog[DebugLen++] = 0x0A;
        if (DebugPort == 0xFF)
            CTOS_USBTxData(DebugLog, DebugLen);
        else
            CTOS_RS232TxData(DebugPort, DebugLog, DebugLen);
		tick = CTOS_TickGet();
		do {
          if (DebugPort == 0xFF)
            ret = CTOS_USBTxReady();
          else
			ret = CTOS_RS232TxReady(DebugPort);
			if (ret == d_OK)
				break;
			//CTOS_Delay(50);
		} while ((CTOS_TickGet() - tick) < d_READY_TIMEOUT);
	}
	//CTOS_RS232Close(d_DEBUG_PORT);
    
    DebugLen = 0;
}

void DebugAddHEX(BYTE *title, BYTE *hex, USHORT len)
{
    if (0 ==strTCT.byRS232DebugPort)
        return;
    
	SetDebugMode(1, 0xFF);
	DebugInit();
	
	if (!ifDebugMode) return;

	if (len > (sizeof (DebugLog) / 2) - 8)
		len = (sizeof (DebugLog) / 2) - 8;
	
	CTOS_RTCGet(&stRTC);
	
	memset(DebugLog, 0x00, sizeof(DebugLog));
	sprintf(DebugLog, "<%02d:%02d:%02d> ", stRTC.bHour, stRTC.bMinute, stRTC.bSecond);
	DebugLen = wub_strlen(DebugLog);
	DebugLog[DebugLen++] = '[';
	DebugLog[DebugLen] = 0x00;
	wub_strcat(&DebugLog[DebugLen], title);
	DebugLen += wub_strlen(title);
	DebugLog[DebugLen++] = ']';
	DebugLog[DebugLen++] = ' ';
	DebugLen += wub_hex_2_str(hex, &DebugLog[DebugLen], len);
	
	DebugExport232();
}

void DebugAddINT(BYTE *title, LONG value)
{
	BYTE temp[50];

    if (0 ==strTCT.byRS232DebugPort)
        return;
    
	SetDebugMode(1, 0xFF);
	DebugInit();
	
	if (!ifDebugMode) return;
	
	CTOS_RTCGet(&stRTC);
	
	memset(DebugLog, 0x00, sizeof(DebugLog));
	sprintf(DebugLog, "<%02d:%02d:%02d> ", stRTC.bHour, stRTC.bMinute, stRTC.bSecond);
	wub_strcat(DebugLog, "[");
	wub_strcat(DebugLog, title);
	wub_strcat(DebugLog, "] ");
	///wub_str_append_long_dec(DebugLog, value);
	wub_memset(temp, 0x00, sizeof (temp));
	sprintf(temp, "%ld", value);
	wub_strcat(DebugLog, temp);
	
	DebugLen = wub_strlen(DebugLog);
	DebugExport232();
}

void DebugAddIntX(BYTE *title, LONG value)
{
	BYTE temp[50];

    if (0 ==strTCT.byRS232DebugPort)
        return;
    
	SetDebugMode(1, 0xFF);
	DebugInit();

	if (!ifDebugMode) return;
	
	CTOS_RTCGet(&stRTC);
	
	memset(DebugLog, 0x00, sizeof(DebugLog));
	sprintf(DebugLog, "<%02d:%02d:%02d> ", stRTC.bHour, stRTC.bMinute, stRTC.bSecond);
	
	wub_strcat(DebugLog, "[");
	wub_strcat(DebugLog, title);
	wub_strcat(DebugLog, "] ");
	
	wub_memset(temp, 0x00, sizeof (temp));
	sprintf(temp, "0x%08lX", value);
	wub_strcat(DebugLog, temp);
	
	DebugLen = wub_strlen(DebugLog);
	DebugExport232();
}

void DebugAddSTR(BYTE *title, BYTE *baMsg, USHORT len)
{

    if (0 ==strTCT.byRS232DebugPort)
        return;
    
	SetDebugMode(1, 0xFF);
	DebugInit();
	
	if (!ifDebugMode) return;
	
	if (baMsg == NULL) return;
	
	CTOS_RTCGet(&stRTC);
	
	memset(DebugLog, 0x00, sizeof(DebugLog));
	sprintf(DebugLog, "<%02d:%02d:%02d> ", stRTC.bHour, stRTC.bMinute, stRTC.bSecond);
	DebugLen = wub_strlen(DebugLog);
	
	DebugLog[DebugLen++] = '[';
	wub_strcat(&DebugLog[DebugLen], title);
	DebugLen += wub_strlen(title);
	DebugLog[DebugLen++] = ']';
	DebugLog[DebugLen++] = ' ';
	
	wub_memcpy(&DebugLog[DebugLen], baMsg, strlen(baMsg));
	DebugLen += strlen(baMsg);
	
	DebugExport232();
}

void vdDebug_LogPrintf(const char* fmt, ...)
{
    char printBuf[2048];
	char msg[2048];
	char space[100];
	int inSendLen;
	va_list marker;
	int j = 0;
    char szAPName[25];
	int inAPPID;

    //if (0 ==strTCT.byRS232DebugPort)
        return;
    
    memset(msg, 0x00, sizeof(msg));
	memset(printBuf, 0x00, sizeof(printBuf));
	memset(space, 0x00, sizeof(space));
	
	va_start( marker, fmt );
	vsprintf( msg, fmt, marker );
	va_end( marker );
	
	memset(printBuf, 0x00, sizeof(printBuf));		
	strcat(printBuf, msg);
	strcat(printBuf, space);
	strcat(printBuf ,"\n" );
	
	inSendLen = strlen(printBuf);

	inMultiAP_CurrentAPNamePID(szAPName, &inAPPID);

    DebugAddSTR(szAPName,printBuf,inSendLen);
}

void vdPCIDebug_HexPrintf(char *display,char *szHexbuf, int len)
{
	char szbuf[MAX_DEBUF_BUF];
	memset(szbuf,0x00,sizeof(szbuf));

	if (0 ==strTCT.byRS232DebugPort)
        return;
	
	wub_hex_2_str(szHexbuf,szbuf,len);
	vdDebug_LogPrintf("[%s]=[%s]",display,szbuf);
}

