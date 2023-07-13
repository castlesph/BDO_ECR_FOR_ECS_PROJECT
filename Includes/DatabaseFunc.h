
#ifndef _DATABASEFUNC__H
#define	_DATABASEFUNC__H

#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <stdarg.h>

#include <typedef.h>

#include "myFileFunc.h"
#include "../Includes/ECRTypedef.h"

//��ע�⣬��Ҫ����ļ���Ĵ�Сд
#define DB_TERMINAL     "/home/ap/pub/TERMINAL.S3DB"
#define DB_MULTIAP	"/home/ap/pub/MULTIAP.S3DB"

#define ECRISOLOG_FILE "/home/ap/pub/ECRISO.LOG"

int inTCTRead(int inSeekCnt);
int inMultiAP_Database_BatchDelete(void);
int inMultiAP_Database_BatchInsert(TRANS_DATA_TABLE *transData);
int inCTOSS_MultiAPGetData(void);
int inIITRead(int inSeekCnt);
int inHDTRead(int inSeekCnt);
int inMultiAP_Database_BatchReadEx(TRANS_DATA_TABLE *transData);
int inDatabase_TerminalOpenDatabase(void);
int inDatabase_TerminalCloseDatabase(void);

#endif	/* _DATABASEFUNC__H */

