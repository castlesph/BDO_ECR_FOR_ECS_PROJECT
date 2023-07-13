/* 
 * File:   debug.h
 * Author: PeyJiun
 *
 */

#ifndef _DEBUG_H
#define	_DEBUG_H

#ifdef	__cplusplus
extern "C" {
#endif

/****************
* if bPort == 0xFF --> USB mode
****************/
void SetDebugMode(BYTE bMode, BYTE bPort);
void DebugInit(void);
void DebugAddSTR(BYTE *title, BYTE *baMsg, USHORT len);
void vdDebug_LogPrintf(const char* fmt, ...);
BYTE byGetDebugMode(void);
void DebugAddHEX(BYTE *title, BYTE *hex, USHORT len);
void DebugAddINT(BYTE *title, LONG value);
void vdPCIDebug_HexPrintf(char *display,char *szHexbuf, int len);

	
#ifdef	__cplusplus
}
#endif

#endif	/* _DEBUG_H */

