/*******************************************************************************

*******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctosapi.h>
#include <EMVLib.h>
#include <libxml/tree.h>

//#include "..\Includes\emvaplib.h"
//#include "..\Includes\EMVTrans.h"
#include "..\Includes\DataBaseFunc.h"
#include "..\Includes\myFileFunc.h"
#include "..\Includes\debug.h"
#include "..\Includes\utils.h"
#include "..\Includes\ECRTypedef.h"
#include "..\ECR\MultiAptrans.h"
#include "..\ECR\ECRTrans.h"
#include <MultiAplib.h>

BYTE ECRPort = d_DEBUG_PORT;
BYTE szECRRecvData[4096 + 2];
BYTE szECRSendData[4096 + 2];

USHORT lECRRecvLen;
LONG lECRSendLen;

extern char g_szAPName[25];

static CTOS_FONT_ATTRIB stgFONT_ATTRIB;
BOOL fWatsonsFlag = FALSE;


int inECR_InitCOM(void)
{
	char szBaud[3];
	int inResult = d_OK;

//	strTCT.byRS232ECRPort=2;

    vdDebug_LogPrintf("inECR_InitCOM[%s]=byRS232ECRPort[%d]", "SHARE_ECR" ,strTCT.byRS232ECRPort);
	vdDebug_LogPrintf("inECR_InitCOM[%s]=fECRBaudRate[%d]", "SHARE_ECR" ,strTCT.fECRBaudRate);

	if (strTCT.byRS232ECRPort == 8)
	{
		ECRPort = 0xFF;
		inResult = CTOS_USBOpen();
	}
	else
	{
		if (strTCT.byRS232ECRPort == 1)
			ECRPort = d_COM1;
		
		if (strTCT.byRS232ECRPort == 2) /*Added to for COM2 port -- sidumili*/
			ECRPort = d_COM2;

		// BaudRate
		if (strTCT.fECRBaudRate == 1){	
			inResult = CTOS_RS232Open(ECRPort, 9600, 'N', 8, 1);
		}

		else if (strTCT.fECRBaudRate == 2){
			inResult = CTOS_RS232Open(ECRPort, 19200, 'N', 8, 1);
		}

		else if (strTCT.fECRBaudRate == 3){	
			inResult = CTOS_RS232Open(ECRPort, 115200, 'N', 8, 1);
		}	
	}

	vdDebug_LogPrintf("::inResult[%d]::ECRPort[%d]", inResult, ECRPort);
	
	return inResult;
}

int inCTOSS_CheckECREvent(void)
{
	USHORT inRet = 0;
	ULONG tick;
	USHORT ret, lECRRecvLenTemp = 0;
    unsigned char ucTemp;
	int inLen = 0;
	BYTE szTemp[4+1];
	BYTE szSize[4+1];
	BYTE szECRRecvTempData[4096 + 2];
	BYTE szECRRecvTemp[4096 + 2] = {0};
	BYTE byDataSize[2+1] = {0};
	char szDataSize[4+1] = {0};
	BOOL fLoop = TRUE;
	int i,inRecvCtr = 0;
	char szbuff[4+1] = {0};
	
	VS_BOOL fValidate = VS_TRUE;
	lECRRecvLen = sizeof(szECRRecvData);

	vdDebug_LogPrintf("byRS232ECRPort=[%d],d_READY_TIMEOUT=[%d],ECRPort=[%d]",strTCT.byRS232ECRPort,d_READY_TIMEOUT,ECRPort);

	if (0 ==strTCT.byRS232ECRPort)
       return(ERR_COM_TIMEOUT);

RECV_AGAIN:
	
	tick = CTOS_TickGet();
	do {
		if (ECRPort == 0xFF)
		  ret = CTOS_USBRxReady(&lECRRecvLen);
		else
		  ret = CTOS_RS232RxReady(ECRPort, &lECRRecvLen);
		if (ret == d_OK)
			break;
	}while ((CTOS_TickGet() - tick) < d_READY_TIMEOUT);

	vdDebug_LogPrintf("lECRRecvLen=[%d],ret=[%d]",lECRRecvLen,ret);
	if (ret != d_OK)
		return(ERR_COM_TIMEOUT);

	if (lECRRecvLen <= 0)
		return(ERR_COM_TIMEOUT);

	memset(szECRRecvData, 0x00, sizeof(szECRRecvData));
//	lECRRecvLen = sizeof(szECRRecvData);
	
	if (ret == d_OK) {
		if (ECRPort == 0xFF)
			CTOS_USBRxData(szECRRecvData, &lECRRecvLen);
		else
		{
			CTOS_Delay(50);
			lECRRecvLen = 2048;
			CTOS_RS232RxData(ECRPort, szECRRecvData, &lECRRecvLen);
		}
		tick = CTOS_TickGet();

		//--sidunili
 		if (strTCT.fPrintISOECR == TRUE){
	 		inPrintECRPacket("TX:ECR", szECRRecvData, lECRRecvLen);
		}

		if(strTCT.inECRMode == 0)// For Windows
		{
			if(szECRRecvData[0] == ACK)
				return 0;
		}
//READ UNTIL ENTIRE DATA IS RECEIVED - START		
		inRecvCtr++;
		if(inRecvCtr == 1)
		{
			memcpy(byDataSize,&szECRRecvData[1],2);
			wub_hex_2_str(byDataSize,szDataSize,2);
			inLen = atoi(szDataSize);
		}
				
		vdDebug_LogPrintf("INDEX IS %d",lECRRecvLenTemp);
		memcpy(&szECRRecvTemp[lECRRecvLenTemp],szECRRecvData,lECRRecvLen);
		lECRRecvLenTemp += lECRRecvLen;
		
		vdDebug_LogPrintf("Data Length is %d :: Actual Recveive Len is %d :: Total Received %d",inLen,lECRRecvLen,lECRRecvLenTemp);
		
		if(inLen > (lECRRecvLenTemp - 5))//minus STX, LENGTH, STX AND LRC
		{
			vdDebug_LogPrintf("READ AGAIN");	
			goto RECV_AGAIN;
		}
	
		memset(szECRRecvData,0x00,sizeof(szECRRecvData));
		memcpy(szECRRecvData,szECRRecvTemp,lECRRecvLenTemp);
		lECRRecvLen = lECRRecvLenTemp;
		vdDebug_LogPrintf("loops %d",inRecvCtr);

//READ UNTIL ENTIRE DATA IS RECEIVED - END

		vdPCIDebug_HexPrintf("ECR Recv Data", szECRRecvData, lECRRecvLen);
		vdDebug_LogPrintf("szECRRecvData[lECRRecvLen-2]=[%02x]",szECRRecvData[lECRRecvLen-2]);
#if 0
		if (szECRRecvData[lECRRecvLen-2] == 0x03)
		{
			if (ECRPort == 0xFF)
			{
				CTOS_USBRxFlush();
				CTOS_USBTxFlush();
			}
			else
			{
				CTOS_RS232FlushRxBuffer(ECRPort);
				CTOS_RS232FlushTxBuffer(ECRPort);
			}
			//break;
		}
#endif
		if(strTCT.inECRMode == 0)// For windows
		{
			memset(szECRRecvTempData,0x00,sizeof(szECRRecvTempData));
			i = 0;
			
			while (1)
			{
				if (szECRRecvData[i] == STX)
				{
					//memcpy(szECRRecvTempData[i], &szECRRecvData[i], 1);

					szECRRecvTempData[i] = szECRRecvData[i];
				
				}
				else if (szECRRecvData[i] == ETX)
				{
					szECRRecvTempData[i] = szECRRecvData[i]; // Get EXT + LRC
					i++;
					szECRRecvTempData[i] = szECRRecvData[i];

					if (inValidatePacket(szECRRecvTempData, i+1) != VS_SUCCESS)				
					{
						szECRRecvTempData[i] = 0x00; //reset value
						i--;//return to index of ETX
					}
					else
					{
						memset(szECRRecvData, 0x00, sizeof(szECRRecvData));
						lECRRecvLen = i+1;
						memcpy(szECRRecvData, szECRRecvTempData, lECRRecvLen);
						//fLoop = FALSE;
						break;
					}
				}else{
					szECRRecvTempData[i] = szECRRecvData[i];
				}
				
				i++;

				if (lECRRecvLen == i){
					//fLoop = FALSE;
					break;
				}
				
			}
		
			 
		}

		
        if (fValidate == VS_TRUE) {
			vdDebug_LogPrintf("validate packet");
            if (inValidatePacket(szECRRecvData, lECRRecvLen) != VS_SUCCESS) {
                ucTemp = NAK;
				if (ECRPort == 0xFF)
					CTOS_USBTxData((char *) &ucTemp, 1);
				else
					CTOS_RS232TxData(ECRPort, (char *) &ucTemp, 1);
				return(ERR_COM_WRITE);
            }
			else
			{
				if(get_env_int("CREDITBUSY") == TRUE && strTCT.inECRMode == 0)//WINDOWS ONLY
				{
					return 0;			
				}
				
                vdDebug_LogPrintf("validate packet send ack");
                ucTemp = ACK;
				if (ECRPort == 0xFF)
					CTOS_USBTxData((char *) &ucTemp, 1);
				else
					CTOS_RS232TxData(ECRPort, (char *) &ucTemp, 1);

vdPCIDebug_HexPrintf("ECR Recv Data", szECRRecvData, lECRRecvLen);
vdDebug_LogPrintf("szECRRecvData[lECRRecvLen-2]=[%02x]",szECRRecvData[lECRRecvLen-2]);

#if 0

				//test
				lECRRecvLen = 48;
					szECRRecvData[0] = 0x02;
					szECRRecvData[1] = 0x00;
					szECRRecvData[2] = 0x45;
					szECRRecvData[3] = 0x36;
					szECRRecvData[4] = 0x30;
					szECRRecvData[5] = 0x30;
					szECRRecvData[6] = 0x30;
					szECRRecvData[7] = 0x30;
					szECRRecvData[8] = 0x30;
					szECRRecvData[9] = 0x30;
					szECRRecvData[10] = 0x30;
					szECRRecvData[11] = 0x30;
					szECRRecvData[12] = 0x30;
					szECRRecvData[13] = 0x31;
					szECRRecvData[14] = 0x30;
					szECRRecvData[15] = 0x32;
					szECRRecvData[16] = 0x30;
					szECRRecvData[17] = 0x30;
					szECRRecvData[18] = 0x30;
					szECRRecvData[19] = 0x30;
					szECRRecvData[20] = 0x1C;
					szECRRecvData[21] = 0x34;
					szECRRecvData[22] = 0x30;
					szECRRecvData[23] = 0x00;
					szECRRecvData[24] = 0x12;
					szECRRecvData[25] = 0x30;
					szECRRecvData[26] = 0x30;
					szECRRecvData[27] = 0x30;
					szECRRecvData[28] = 0x30;
					szECRRecvData[29] = 0x30;
					szECRRecvData[30] = 0x30;
					szECRRecvData[31] = 0x34;
					szECRRecvData[32] = 0x30;
					szECRRecvData[33] = 0x30;
					szECRRecvData[34] = 0x30;
					szECRRecvData[35] = 0x30;
					szECRRecvData[36] = 0x30;
					szECRRecvData[37] = 0x1C;
					szECRRecvData[38] = 0x44;
					szECRRecvData[39] = 0x36;
					szECRRecvData[40] = 0x00;
					szECRRecvData[41] = 0x05;
					szECRRecvData[42] = 0x31;
					szECRRecvData[43] = 0x31;
					szECRRecvData[44] = 0x36;
					szECRRecvData[45] = 0x35;
					szECRRecvData[46] = 0x34;
					szECRRecvData[47] = 0x1C;
					szECRRecvData[48] = 0x03;
					szECRRecvData[49] = 0x3D;
				
				inPrintECRPacket("TX:ECR", szECRRecvData, lECRRecvLen);
#endif
				//put_env_int("ECRBUSY",1);
				inRet=inECRReceiveAnalyse();

				if(inRet == VS_CONTINUE)//For Settle All inECRSendAnalyse is called inside inECRReceiveAnalyse.
					return d_OK;

#if 1
                if (inRet != d_OK){

					
					vdDebug_LogPrintf("SEND NAK");
					strcpy(srTransRec.szECRRespCode,"NA");
					strcpy(srTransRec.szECRRespText, ECR_OPER_CANCEL_RESP);
                }
#endif
				

				inECRSendAnalyse();
			}
        }
	}
	//CTOS_RS232Close(d_DEBUG_PORT);
	//put_env_int("ECRBUSY",0);
	lECRRecvLen = 0;
	
#if 0	
	if (ECRPort == 0xFF)
	{
		CTOS_USBRxFlush();
		CTOS_USBTxFlush();
	}
	else
	{
		CTOS_RS232FlushRxBuffer(ECRPort);
		CTOS_RS232FlushTxBuffer(ECRPort);
	}
#endif	
}

/****************************************************************************
 * Routine Name:  ucGetLRC()
 * Description :  Computes the LRC of the packet of given size.
 * Input       :  Request packet    -  pucPacket
 *                Packet size       -  inSize
 * Output      :  Computed LRC byte.
 * Notes       :  LRC is the XOR of all the bytes in the packet.
 *****************************************************************************/
unsigned char ucGetLRC(char* pucPacket,int inSize) {
    unsigned char ucLRC = *pucPacket++;  

    while (--inSize > 0)
        ucLRC ^= *pucPacket++;
    return(ucLRC);
}

/****************************************************************************
 * Routine Name:  inValidatePacket()
 * Description :  Validates a packet of the form <STX><Data><ETX><LRC>. 
 * Input       :  Packet      - pucPacket
 *                Packet size - inSize
 * Output      :  VS_SUCCESS     on success ( correct packet format )
 *                VS_ERR         on error.
 * Notes       :  Uses a local buffer for all calculations. Because of this 
 *                all outer layer callers can use the source and destination 
 *                as the same buffer. The packet formed will be null 
 *                terminated also.
 *****************************************************************************/
int inValidatePacket(char* pucPkt,int inSize) {
	unsigned char ucCompLRC;
	unsigned char ucLRC;
	
    if (inSize < 4)
        return(
        VS_ERR);   /* Atleast a pcaket should have <STX><1 char><ETX><LRC> */
    if (pucPkt[0] != STX)
        return(VS_ERR);
    if (pucPkt[inSize - 2] != ETX)
        return(VS_ERR);

	ucCompLRC = ucGetLRC(&(pucPkt[1]), (inSize - 2));
	ucLRC = pucPkt[inSize - 1];
	vdDebug_LogPrintf("ucCompLRC[%02x] :: ucLRC[%02x]",ucCompLRC,ucLRC);
    if (ucCompLRC != ucLRC) 
        return(VS_ERR);
   
    return(VS_SUCCESS);
}

int inCTOSS_MultiAPSaveData(int IPC_EVENT_ID)
{
	BYTE bret;
	BYTE outbuf[d_MAX_IPC_BUFFER];
	USHORT out_len = 0;
    char szDataBuf[10];
	char szAPName[25];
	int inAPPID;
	
	memset(outbuf,0x00,sizeof(outbuf));
	memset(szAPName,0x00,sizeof(szAPName));

	inMultiAP_CurrentAPNamePID(szAPName, &inAPPID);

	vdDebug_LogPrintf("szAPName[%s]=[%s]", "V5S_VisaMaster" ,szAPName);
	
//	if (strcmp (strHDT.szAPName,szAPName) == 0)
//		return d_OK;

	bret= inMultiAP_Database_BatchDelete();
	vdDebug_LogPrintf("inMultiAP_Database_BatchDelete,bret=[%d]", bret);
	if(d_OK != bret)
	{
//		vdSetErrorMessage("MultiAP BatchDelete ERR");
    	return bret;
	}
	
	bret = inMultiAP_Database_BatchInsert(&srTransRec);
	vdDebug_LogPrintf("inMultiAP_Database_BatchInsert=[%d]", bret);
	if(d_OK != bret)
	{
//		vdSetErrorMessage("MultiAP BatchInsert ERR");
    	return bret;
	}

	vdDebug_LogPrintf("szAPName[%s],bret=[%d]", g_szAPName ,bret);

    memset(szDataBuf, 0x00, sizeof(szDataBuf));
	inMultiAP_RunIPCCmdTypesEx(g_szAPName ,IPC_EVENT_ID, szDataBuf, 0, outbuf,&out_len);

	inTCTRead(1);

    vdDebug_LogPrintf("IPC_EVENT_ID [%d =? %d %d]", IPC_EVENT_ID, outbuf[0], outbuf[1]);
	
	if (outbuf[0] != IPC_EVENT_ID)
	{
//		vdSetErrorMessage("MultiAP Type ERR");
		return d_NO;
	}

	if (outbuf[1] != d_SUCCESS)
	{
//		vdSetErrorMessage("MultiAP ERR");
		return d_NO;
	}
	
	return d_OK;
}

int inECRReceiveAnalyse(void)
{
	int inSize, inRcvLen, offset, retry, x = 0, iIndex = 0, inHostCount = 0;
	char szBuf[30];
	char chAmtFlag, chCBFlag, chInvFlag, chSettleFlag;
	char chResp;
	char szHandle[20];
	char szDSPBuf[100+1];
	char szHexBuf[100+1];
	long event;

	char stVerInfo[GEN_VER_SIZE];
	char szLogicalName[16];
	unsigned char szEventDataBuffer[512];
	int hConfFHandle = 0;
	int inOffset = 0;
	int inTestLen =0;
	int inEESLAPIReturnValue = 0;
	unsigned short ushEventDataSize = 0;
	unsigned short ushEESLAPIReturnValue = 0;
//	int inRetVal=d_OK;
	int i;
	char szFulshBuf[256+1];
	char szTempLenBuf[4+1];
	unsigned char ucTemp;
	int	inTemLen;
	int inRetVal = VS_SUCCESS;
	int inRet = VS_SUCCESS;

	BYTE szPOSTID[6+1];
	BYTE szTemp[6+1];
	BYTE byVirtualCard_No[30+1];
	BYTE byPAN[19+1];
	BOOL fVarAmtLenFlag = 0;
	int inCount = 0;
	int inPadCount = 0;

	vdDebug_LogPrintf("--=== inECRReceiveAnalyse ===--");
	chECRFlag = 0;
	
	//gcitra   
	memset(&ECRResp,0x00, sizeof(ECRResp));
	//gcitra
		
	memcpy(ECRResp.resp_code, ECR_UNKNOWN_ERR, 2);
	memcpy(ECRResp.resp_text, ECR_REMOVE_RESP, ECR_RESP_TEXT_SIZE);
	memcpy(ECRResp.date, "000000", DATE_SIZE);
	memcpy(ECRResp.time, "000000", TIME_SIZE);

	memset(ECRResp.prnt_text1, 0, ECR_PRNT_TEXT1_SIZE);
	memset(ECRResp.prnt_text2, 0, ECR_PRNT_TEXT2_SIZE);
	memset(ECRResp.prnt_text3, 0, ECR_PRNT_TEXT3_SIZE);
	memset(ECRResp.prnt_text4, 0, ECR_PRNT_TEXT4_SIZE);

	memcpy(&ECRResp.merchant_name[0], szBuf, 23);	
	memcpy(&ECRResp.merchant_name[23], szBuf, 23);	
	memcpy(&ECRResp.merchant_name[46], szBuf, 23);

	memset(szDSPBuf,0x00, sizeof(szDSPBuf));
	memset(szHexBuf,0x00, sizeof(szHexBuf));
	memset(szBuf,0x00,sizeof(szBuf));

/////////////////////////////////////////////////////////////////////////////////////////	
//	inSize = inRecvECRPacket(hECRPort,szECRRecBuf, RECV_BUF_SIZE, VS_TRUE, 2, 3);

/////////////////////////////////////////////////////////////////////////////////////////
	
	chAmtFlag = 0;
	chCBFlag = 0;
	chInvFlag = 0;
	
	offset = 1;

    wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);

	szBuf[4] = 0;
	inRcvLen = atoi(szBuf);
	vdDebug_LogPrintf("Recv Len:%d",inRcvLen);
 	
 	memset((char *)&ECRReq, 0, sizeof(ECR_REQ));
 	
	retry = 0;
	offset = 1;
	
 	offset += 2;
 	
 	memset((char *)&ECRReq, 0, sizeof(ECR_REQ)); 	
 	if(memcmp((unsigned char *)&szECRRecvData[offset],"60",TRANSPORT_HEADER_SIZE))
 	{
 		return BRANCH_EXIT;
	}
 	offset += TRANSPORT_HEADER_SIZE;
 	offset += TRANSPORT_DEST_SIZE;
 	offset += TRANSPORT_SRC_SIZE;
 	offset += FORMAT_VERSION_SIZE;
	
 	memcpy(ECRReq.req_resp, &szECRRecvData[offset], REQ_RESP_SIZE);
 	ECRReq.req_resp[REQ_RESP_SIZE] = 0;
 	offset += REQ_RESP_SIZE;
	
 	memcpy(ECRReq.txn_code, &szECRRecvData[offset], TXN_CODE_SIZE);
 	ECRReq.txn_code[TXN_CODE_SIZE] = 0;
 	offset += TXN_CODE_SIZE;
	
 	offset += ECR_RESP_CODE_SIZE;
 	offset += MORE_FOLLOW_SIZE;

	szECRRecvData[19] = 0x1C;

	if(memcmp(ECRReq.txn_code, ECR_COMM_TEST_TAG, 2) != 0)
	{
		if(szECRRecvData[offset] != ECR_SEPARATOR && szECRRecvData[offset] != WATSONS_ECR_SEPARATOR)
		{
			return BRANCH_EXIT;
		}
	}

	if(szECRRecvData[offset] == WATSONS_ECR_SEPARATOR)
		fWatsonsFlag = fVarAmtLenFlag = 1;
	
 	offset += END_PRESENT_SIZE;

	vdDebug_LogPrintf("ECRBuf:%2x %2x %2x",szECRRecvData[offset],szECRRecvData[offset+1],szECRRecvData[offset+2]);
	vdDebug_LogPrintf("Offset:%d,ECRReq.txn_code=[%s]",offset,ECRReq.txn_code);
	
 	while(1)
 	{
        if(offset >= lECRRecvLen - 1) {//do not include LRC. 
            break;
        }

		if(!memcmp(&szECRRecvData[offset], ECR_AMOUNT_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);
			szBuf[4] = 0;
			inSize = atoi(szBuf);

			vdDebug_LogPrintf("Amt Offset:%d",offset);

			offset += 2;
			if(fVarAmtLenFlag == TRUE)
			{
				inPadCount = 12-inSize;
				memset(ECRReq.amount,0x30,12);
				memcpy(&ECRReq.amount[inPadCount], &szECRRecvData[offset], inSize);
				offset += inSize;
			}
			else
			{
				memcpy(ECRReq.amount, &szECRRecvData[offset], 12);
				offset += 12;
			}
			ECRReq.amount[12] = 0;

			offset += 1;

			vdDebug_LogPrintf("Amt size:%d",inSize);
			vdDebug_LogPrintf("Amt Offset1:%d",offset);
			
			chAmtFlag = 1;
		}

		if(!memcmp(&szECRRecvData[offset], ECR_TIP_AMOUNT_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);
			szBuf[4] = 0;
			inSize = atoi(szBuf);

			vdDebug_LogPrintf("tip Amt Offset:%d",offset);

			offset += 2;
			if(fVarAmtLenFlag == TRUE)
			{
				inPadCount = 12-inSize;
				memset(ECRReq.tip_amount,0x30,12);
				memcpy(&ECRReq.tip_amount[inPadCount], &szECRRecvData[offset], inSize);
				offset += inSize;
			}
			else
			{
				memcpy(ECRReq.tip_amount, &szECRRecvData[offset], 12);
				offset += 12;
			}
			ECRReq.tip_amount[12] = 0;

			offset += 1;

			vdDebug_LogPrintf("Tip Amt size:%d",inSize);
			vdDebug_LogPrintf("Tip Amt Offset1:%d",offset);
			
			chAmtFlag = 1;
		}
		
		if(!memcmp(&szECRRecvData[offset], ECR_CARD_NO_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);

			szBuf[4] = 0;
			inSize = atoi(szBuf);

			vdDebug_LogPrintf("PAN Offset:[%d][%d]",offset,inSize);

			offset += 2;
			if (inSize > PAN_SIZE)
			{
				strcpy(ECRResp.resp_text,"PAN IS WRONG");
				return BRANCH_EXIT;
			}
			memcpy(ECRReq.Pan, &szECRRecvData[offset], inSize);
			ECRReq.Pan[inSize] = 0;
			
			offset += inSize;

			offset += 1;

			vdDebug_LogPrintf("PAN size:[%s][%d]",ECRReq.Pan,offset);

		}
		if(!memcmp(&szECRRecvData[offset], ECR_EXPIRY_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);

			szBuf[4] = 0;
			inSize = atoi(szBuf);

			vdDebug_LogPrintf("EXPIRY Offset:[%d][%d]",offset,inSize);

			offset += 2;
			memcpy(ECRReq.Expiry, &szECRRecvData[offset], inSize);
			ECRReq.Expiry[inSize] = 0;
			
			offset += inSize;
			offset += 1;

			vdDebug_LogPrintf("ECRReq.Expiry:[%s]",ECRReq.Expiry);
		}
		
		if(!memcmp(&szECRRecvData[offset], ECR_DATE_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);
			szBuf[4] = 0;
			inSize = atoi(szBuf);

			offset += 2;
			memcpy(ECRReq.date, &szECRRecvData[offset], 6);
			vdDebug_LogPrintf("ECR Date:%s",ECRReq.date);
			offset += 6;
			offset += 1;
			chAmtFlag = 1;
		}
		
		if(!memcmp(&szECRRecvData[offset], ECR_RREF_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);
			szBuf[4] = 0;
			inSize = atoi(szBuf);

			offset += 2;
			memcpy(ECRReq.rref, &szECRRecvData[offset], 12);
			vdDebug_LogPrintf("ECR RRF:%s",ECRReq.rref);
			offset += 12;
			offset += 1;
			chAmtFlag = 1;
		}
		else if(!memcmp(&szECRRecvData[offset], ECR_CASHBACK_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);

			szBuf[4] = 0;
			inSize = atoi(szBuf);
			offset += 2;
			memcpy(ECRReq.cashback, &szECRRecvData[offset], inSize);
			ECRReq.cashback[inSize] = 0;
			offset += inSize;
			offset += 1;
			chCBFlag = 1;
		}
	
		else if(!memcmp(&szECRRecvData[offset], ECR_INV_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			vdDebug_LogPrintf("ECR ECR_INV_TAG Tag");
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);

			szBuf[4] = 0;
			inSize = atoi(szBuf);
			offset += 2;
			memset(szTemp,0x00,sizeof(szTemp));
			memcpy(szTemp, &szECRRecvData[offset], inSize);
			offset += inSize;//Added for Tip Adjust
			vdDebug_LogPrintf("szTemp[%s], inSize[%d]",szTemp,inSize);
			sprintf(ECRReq.inv_no,"%06ld",atol(szTemp)); // FORMAT AMOUNT by adding pad of 0's if ECR replies with invoice number of variable length
			vdDebug_LogPrintf("ECR Inv:%s",ECRReq.inv_no);
			ECRReq.inv_no[6] = 0;

			if(!memcmp(ECRReq.txn_code, ECR_REPRINT_TAG, 2) && atol(ECRReq.inv_no) == 0)
			{
				vdDebug_LogPrintf("Reprint Last");//0 invoice is an exception. 0 invoice means reprint last.
			}
			else if (atol(ECRReq.inv_no) <= 0 || atol(ECRReq.inv_no) > 999999)
			{
				memset(ECRResp.resp_text, ' ', ECR_RESP_TEXT_SIZE);					
				strcpy(ECRResp.resp_text,"INVOICE NO. IS WRONG");
				chECRFlag = 1;
				return BRANCH_EXIT;
			}
			offset += 1;
			chInvFlag = 1;
		}
		else if(!memcmp(&szECRRecvData[offset], ECR_SMAC_NEW_DATE_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);
			szBuf[4] = 0;
			inSize = atoi(szBuf);

			offset += 2;
			memcpy(ECRReq.date, &szECRRecvData[offset], 6);
			vdDebug_LogPrintf("ECR Date:%s",ECRReq.date);
			offset += 6;
			offset += 1;
			chAmtFlag = 1;

			if(!memcmp(ECRReq.txn_code, ECR_SALE_TAG, 2) || !memcmp(ECRReq.txn_code, ECR_PTS_AWARDING_TAG, 2))//search for amount and if virtual card is sent.
			{
				x = 0;
				while(1)
				{
					if(memcmp(&szECRRecvData[x],"\x34\x30\x00\x12", 4) == 0)
					{
						vdDebug_LogPrintf("AMT TAG FOUND");
						memcpy(ECRReq.amount, &szECRRecvData[x+4], 12);
						ECRReq.amount[12] = 0;
						break;
					}					
					
					if(x > lECRRecvLen - 4)
						break;

					x++;
				}

				x = 0;
				while(1)
				{
					if(memcmp(&szECRRecvData[x],"\x53\x32\x00\x16", 4) == 0)
					{
						vdDebug_LogPrintf("VIRTUAL CARD TAG FOUND");
						memcpy(byVirtualCard_No, &szECRRecvData[x+4], 30);
						//vdTrimSpaces(byVirtualCard_No);
						vdDebug_LogPrintf("byVirtualCard_No %s",byVirtualCard_No);
						break;
					}

					if(x > lECRRecvLen - 4)
						break;

					x++;
				}
			}
			
		}
		else if(!memcmp(&szECRRecvData[offset], ECR_AUTH_CODE_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);

			szBuf[4] = 0;
			inSize = atoi(szBuf);

			vdDebug_LogPrintf("AUTHCODE Offset:[%d][%d]",offset,inSize);

			offset += 2;
			if (inSize != AUTH_CODE_SIZE)
			{
				strcpy(ECRResp.resp_text,"AUTHCODE SIZE IS WRONG");
				return BRANCH_EXIT;
			}
			memcpy(ECRReq.auth_code, &szECRRecvData[offset], inSize);
			ECRReq.auth_code[inSize] = 0;
			
			offset += inSize;

			offset += 1;

			vdDebug_LogPrintf("auth_code offset:[%s][%d]",ECRReq.auth_code,offset);

		}
		else if(!memcmp(&szECRRecvData[offset], ECR_INST_AMT_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);
			szBuf[4] = 0;
			inSize = atoi(szBuf);

			vdDebug_LogPrintf("Inst Amt Offset:%d",offset);

			offset += 2;
			if(fVarAmtLenFlag == TRUE)
			{
				inPadCount = 12-inSize;
				memset(ECRReq.amount,0x30,12);
				memcpy(&ECRReq.amount[inPadCount], &szECRRecvData[offset], inSize);
				offset += inSize;
			}
			else
			{
				memcpy(ECRReq.amount, &szECRRecvData[offset], 12);
				offset += 12;
			}
			ECRReq.amount[12] = 0;

			offset += 1;

			vdDebug_LogPrintf("Inst Amt size:%d",inSize);
			vdDebug_LogPrintf("Inst Amt Offset1:%d",offset);
			
			chAmtFlag = 1;
		}
		else if(!memcmp(&szECRRecvData[offset], ECR_INST_TERMS_TAG, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);
			szBuf[4] = 0;
			inSize = atoi(szBuf);

			vdDebug_LogPrintf("Inst terms Offset:%d",offset);

			offset += 2;
			
			memcpy(ECRReq.inst_terms,&szECRRecvData[offset],inSize);

			offset += 2;
			
			ECRReq.inst_terms[2] = 0;

			offset += 1;

			vdDebug_LogPrintf("Inst terms :%s",ECRReq.inst_terms);
			vdDebug_LogPrintf("Inst terms size:%d",inSize);
			vdDebug_LogPrintf("Inst terms Offset1:%d",offset);			
			
		}
		else if(!memcmp(&szECRRecvData[offset], ECR_INST_PROMO, 2))
		{
			memset(szBuf,0x00,sizeof(szBuf));
			
			offset += 2;
			wub_hex_2_str((char *)&szECRRecvData[offset], szBuf, 2);
			szBuf[4] = 0;
			inSize = atoi(szBuf);

			vdDebug_LogPrintf("Inst Promo Offset:%d",offset);

			offset += 2;
			
			memcpy(ECRReq.inst_promo,&szECRRecvData[offset],inSize);

			offset += 2;
			
			ECRReq.inst_promo[2] = 0;

			offset += 1;

			vdDebug_LogPrintf("Inst Promo :%s",ECRReq.inst_promo);
			vdDebug_LogPrintf("Inst Promo size:%d",inSize);
			vdDebug_LogPrintf("Inst Promo Offset1:%d",offset);
			
		}
		else
		{			
			retry ++;
		}
		
		if(retry >= 5)
		{
			if(!memcmp(ECRReq.txn_code, ECR_SALE_TAG, 2))
			{
				if(chAmtFlag)
					break;
			}

            if(!memcmp(ECRReq.txn_code, ECR_PIN_VERIFY, 2))
            {
               break;
            }

			if(!memcmp(ECRReq.txn_code, ECR_KIT_SALE_TAG, 2))
			{
				if(chAmtFlag)
					break;
			}

			if(!memcmp(ECRReq.txn_code, ECR_RENEWAL_TAG, 2))
			{
				if(chAmtFlag)
					break;
			}

			if(!memcmp(ECRReq.txn_code, ECR_PTS_AWARDING_TAG, 2))
			{
				if(chAmtFlag)
					break;
			}

			if(!memcmp(ECRReq.txn_code, ECR_REFUND_TAG, 2))
			{
				if(chAmtFlag)
					break;
			}

			if(!memcmp(ECRReq.txn_code, ECR_OCBC_IPP_TAG, 2))
			{
				if(chAmtFlag)
					break;
			}
			
			if(!memcmp(ECRReq.txn_code, ECR_SALECUP_TAG, 2))
			{
				if(chAmtFlag)
					break;
			}
			if(!memcmp(ECRReq.txn_code, ECR_VOIDCUP_TAG, 2))
			{
				vdDebug_LogPrintf("VOID SALE CUP");
					break;
			}
			if(!memcmp(ECRReq.txn_code, ECR_VOID_TAG, 2))
			{
				if(chInvFlag)
					break;
			}
			if(!memcmp(ECRReq.txn_code, ECR_REFUND_TAG, 2))
			{
				if(chAmtFlag)
					break;
			}
			if(!memcmp(ECRReq.txn_code, ECR_REFUNDCUP_TAG, 2))
			{
				if(chAmtFlag)
					break;
			}
			if(!memcmp(ECRReq.txn_code, ECR_OFFLINECUP_TAG, 2))
			{
				if(chAmtFlag)
					break;
			}
			if(!memcmp(ECRReq.txn_code, ECR_COMPLETION_TAG, 2))
			{
				vdDebug_LogPrintf("ECR_COMPLETION_TAG");
				if(chAmtFlag)
					break;			
			}
			if(!memcmp(ECRReq.txn_code, ECR_PREAUTHCUP_TAG, 2))
			{
				vdDebug_LogPrintf("ECR_PREAUTHCUP_TAG");
					break;
			}
			if(!memcmp(ECRReq.txn_code, ECR_CARDVER_TAG, 2))
			{
				vdDebug_LogPrintf("ECR_CARDVER_TAG");
				if(chAmtFlag)
					break;
			}
			if(!memcmp(ECRReq.txn_code, RETURN_LST_TXN, 2))
			{
				vdDebug_LogPrintf("RETURN_LST_TXNRetrieval");
					break;
			}
			if(!memcmp(ECRReq.txn_code, ECR_PRNT_GRS_TAG, 2))
			{
				vdDebug_LogPrintf("ECR_PRNT_GRS_TAG");
					break;
			}
			if(!memcmp(ECRReq.txn_code, ECR_TIP_ADJ_TAG, 2))
			{
				vdDebug_LogPrintf("ECR_TIP_ADJ_TAG");
				if(chAmtFlag && chInvFlag)
					break;			
			}
			if(!memcmp(ECRReq.txn_code, ECR_SETTLEMENT_ALL_TAG, 2))
			{
				vdDebug_LogPrintf("ECR_SETTLEMENT_ALL_TAG");
				chSettleFlag = 1;
					break;
			}

			if(!memcmp(ECRReq.txn_code, ECR_REPRINT_TAG, 2))
			{
				vdDebug_LogPrintf("ECR_REPRINT_TAG");
				chInvFlag = 1;
				break;
			}

			if(!memcmp(ECRReq.txn_code, ECR_INST_SALE_TAG, 2))
			{
				if(chAmtFlag)
					break;
			}

			if(!memcmp(ECRReq.txn_code, ECR_INST_VOID_TAG, 2))
			{
				if(chInvFlag)
					break;
			}

			if(!memcmp(ECRReq.txn_code, ECR_COMM_TEST_TAG, 2))
			{
				break;
			}
			
			
			return BRANCH_EXIT;
		}
	}
	
	chECRFlag = 1;
	
	memset(&srTransRec, 0x00, sizeof(TRANS_DATA_TABLE));
	wub_str_2_hex(ECRReq.amount, srTransRec.szBaseAmount, sizeof(ECRReq.amount));
	wub_str_2_hex(ECRReq.tip_amount, srTransRec.szTipAmount, sizeof(ECRReq.tip_amount));
	wub_str_2_hex(ECRReq.inv_no, srTransRec.szInvoiceNo, sizeof(ECRReq.inv_no));

	if(strlen(byVirtualCard_No) > 0)
		strcpy(srTransRec.szPAN, byVirtualCard_No);
	else
		strcpy(srTransRec.szPAN, ECRReq.Pan);
	
	wub_str_2_hex(ECRReq.Expiry, srTransRec.szExpireDate, strlen(ECRReq.Expiry));

	strcpy(srTransRec.szAuthCode,ECRReq.auth_code);

	srTransRec.PRMid = atoi(ECRReq.inst_promo);

	vdDebug_LogPrintf("PRMid %d",srTransRec.PRMid);
		
	strcpy(srTransRec.szTerms,ECRReq.inst_terms);	

	inSaveISOECRLog(VS_FALSE, szECRRecvData, inRcvLen); /*Save ECR Send/Receive data -- sidumili*/
			
	vdDebug_LogPrintf("lECRRecvLen[%d]",lECRRecvLen);

	//Get POSTID
	x = 0;
	while(1)
	{
		if(memcmp(&szECRRecvData[x],"\x44\x36\x00\x05", 4) == 0)
		{
			vdDebug_LogPrintf("POSTID TAG FOUND");
			memcpy(szPOSTID, &szECRRecvData[x+4], 5);

			vdDebug_LogPrintf("szPOSTID[%s]",szPOSTID);
			if(strTCT.fPOSTID == TRUE)
			{
				if (strlen(szPOSTID) > 0){
					inCTOSS_PutEnvDB("PTID", szPOSTID);
				}
			}
			break;
		}

		if(x > lECRRecvLen - 4)
			break;

		x++;
	}


	if(!memcmp(ECRReq.txn_code, ECR_SALE_TAG, 2))
	{
		if(chAmtFlag)
		{			    
            DebugAddHEX("ECR Send AMOUNT", srTransRec.szBaseAmount, 6);
			
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_WAVE_SALE);	
			//gcitra
			if(d_OK != inRet)
			{				
				// patrick fix code 20141205
				inMultiAP_Database_BatchDelete();
				return inRet;
			}

			
            inRet = inCTOSS_MultiAPGetData();

            vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
        	vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
        	vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
            vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);        	
        	vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
        	vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
        	vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);
			vdDebug_LogPrintf("POS TID[%s]",szPOSTID);
    
            return inRet;
		}
	}

    if(!memcmp(ECRReq.txn_code, ECR_PIN_VERIFY, 2))
    {
        inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_PIN_VERIFY);
        if(d_OK != inRet)
            return inRet;

        inRet = inCTOSS_MultiAPGetData();
            return inRet;
    }

	if(!memcmp(ECRReq.txn_code, ECR_KIT_SALE_TAG, 2))
	{
		if(chAmtFlag)
		{
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_SMAC_KIT_SALE);
			if(d_OK != inRet)
				return inRet;			

			inRet = inCTOSS_MultiAPGetData();
			return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code, ECR_REFUND_TAG, 2))
	{
		if(chAmtFlag)
		{
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_REFUND);
			if(d_OK != inRet)
				return inRet;			

			inRet = inCTOSS_MultiAPGetData();
			return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code, ECR_OCBC_IPP_TAG, 2))
	{
		if(chAmtFlag)
		{
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_IPP);
			if(d_OK != inRet)
				return inRet;						

			inRet = inCTOSS_MultiAPGetData();
			return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code, ECR_VOID_TAG, 2))
	{
		if(chInvFlag)
		{
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_VOID_SALE);
			if(d_OK != inRet)
				return inRet;									

			inRet = inCTOSS_MultiAPGetData();
			return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code, ECR_RENEWAL_TAG, 2))
	{
		if(chAmtFlag)
		{
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_SMAC_RENEWAL);
			if(d_OK != inRet)
				return inRet;			

			inRet = inCTOSS_MultiAPGetData();
			return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code, ECR_PTS_AWARDING_TAG, 2))
	{
		if(chAmtFlag)
		{
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_SMAC_PTS_AWARDING);
			if(d_OK != inRet)
				return inRet;			

			inRet = inCTOSS_MultiAPGetData();
			return inRet;
		}
	}
	
	if(!memcmp(ECRReq.txn_code, ECR_PRNT_GRS_TAG, 2))
	{
		inRetVal = d_OK;
		return inRetVal;	
	}
	
	if( (!memcmp(ECRReq.txn_code, ECR_SALECUP_TAG, 2))  || (!memcmp(ECRReq.txn_code, ECR_OFFLINECUP_TAG, 2)) ||
		(!memcmp(ECRReq.txn_code, ECR_PREAUTHCUP_TAG, 2)))
	{
		if(chAmtFlag)
		{
			inRetVal = d_OK;
			return inRetVal;	
		}
	}

	if(!memcmp(ECRReq.txn_code, ECR_REFUNDCUP_TAG, 2))
	{		
		if(chAmtFlag)
		{
			inRetVal = d_OK;
			return inRetVal;	
		}
	}

	if( (!memcmp(ECRReq.txn_code, ECR_VOIDCUP_TAG, 2))  || (!memcmp(ECRReq.txn_code,RETURN_LST_TXN, 2)) )
	{
		vdDebug_LogPrintf("Prep Send Buffer");
		inRetVal = d_OK;
		return inRetVal;	
	}

	if(!memcmp(ECRReq.txn_code, ECR_CARDVER_TAG, 2))
	{
		if(chAmtFlag)
		{
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_PRE_AUTH);
			if(d_OK != inRet)
				return inRet;			

			inRet = inCTOSS_MultiAPGetData();
				return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code,ECR_COMPLETION_TAG, 2))
	{
		if(chAmtFlag)
		{
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_OFFLINE_SALE);
			if(d_OK != inRet)
				return inRet;			

			inRet = inCTOSS_MultiAPGetData();
				return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code,ECR_TIP_ADJ_TAG, 2))
	{
		if(chAmtFlag)
		{
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_TIP_ADJUST);
			if(d_OK != inRet)
				return inRet;			

			inRet = inCTOSS_MultiAPGetData();
				return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code,ECR_SETTLEMENT_ALL_TAG, 2))
	{
		vdDebug_LogPrintf("ECR_SETTLEMENT_ALL_TAG chSettleFlag %c",chSettleFlag);
		if(chSettleFlag)
		{			
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_SETTLEMENT_ALL);

			if(d_OK != inRet)
				return inRet;			

			inRet = inCTOSS_MultiAPGetData();
				return inRet;
			
		}
	}

	if(!memcmp(ECRReq.txn_code,ECR_REPRINT_TAG, 2))
	{
		vdDebug_LogPrintf("ECR_REPRINT_TAG chInvFlag %c",chInvFlag);
		if(chInvFlag)
		{
			vdDebug_LogPrintf("ECRReq.inv_no %s",ECRReq.inv_no);
			if(!strcmp(ECRReq.inv_no,"000000"))//Reprint Last
			{
				inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_REPRINT_LAST);
				if(d_OK != inRet)
					return inRet;			
			}
			else
			{
				inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_REPRINT_ANY);
				if(d_OK != inRet)
					return inRet;	
			}

			inRet = inCTOSS_MultiAPGetData();
				return inRet;
		}
	}


	if(!memcmp(ECRReq.txn_code, ECR_INST_SALE_TAG, 2))
	{
		if(chAmtFlag)
		{			    
            DebugAddHEX("ECR Send AMOUNT", srTransRec.szBaseAmount, 6);
			
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_ONLINES_SALE);	
			//gcitra
			if(d_OK != inRet)
			{				
				// patrick fix code 20141205
				inMultiAP_Database_BatchDelete();
				return inRet;
			}

			
            inRet = inCTOSS_MultiAPGetData();

			vdDebug_LogPrintf("Resp PRMid:%d",srTransRec.PRMid);
			vdDebug_LogPrintf("Resp szTerms:%s",srTransRec.szTerms);
            vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
        	vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
        	vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
            vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);        	
        	vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
        	vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
        	vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);
			vdDebug_LogPrintf("POS TID[%s]",szPOSTID);
    
            return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code, ECR_INST_VOID_TAG, 2))
	{
		if(chInvFlag)
		{
			put_env_int("ECRINSTVOID",1);
			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_VOID_SALE);
			put_env_int("ECRINSTVOID",0);

			if(d_OK != inRet)
				return inRet;									

			inRet = inCTOSS_MultiAPGetData();
			return inRet;
		}
	}


	if(!memcmp(ECRReq.txn_code, ECR_COMM_TEST_TAG, 2))
	{
		memset(ECRResp.resp_code,0x00,sizeof(ECRResp.resp_code));
		strcpy(ECRResp.resp_code,"00");
		return d_OK;
	}
	
	vdDebug_LogPrintf("BRANCH EXIT");
	return BRANCH_EXIT;
}

int inECRSendAnalyse(void)
{
	int inRetval = d_NO;
	int iRetVal = 0;
	int currGrp = 0;
	int inLen = 0;
	unsigned char szTempBuff[50 + 1];
	
	vdDebug_LogPrintf("RecvPayment");

	strcpy(ECRResp.resp_code, srTransRec.szECRRespCode);
	//strncpy(ECRResp.resp_code, srTransRec.szECRRespCode, ECR_RESP_CODE_SIZE);
    wub_hex_2_str(srTransRec.szInvoiceNo, ECRResp.inv_no, 3);
	strncpy(ECRResp.tid,srTransRec.szTID, TID_SIZE);
	#if 0
		strncpy(ECRResp.mid, srTransRec.szMID, MID_SIZE);
	#else
		inLen = strlen(srTransRec.szMID);
		memset(ECRResp.mid,0x20,MID_SIZE); 		//add padding of zeros if MID is less than MID_SIZE.
		memcpy(ECRResp.mid,srTransRec.szMID,inLen);
	#endif
	pad_chr(' ', SRIGHT, 10, srTransRec.szCardLable); // BDO: Pad card label with space -- sidumili
	strcpy(ECRResp.issuer, srTransRec.szCardLable);
    wub_hex_2_str(srTransRec.szExpireDate, ECRResp.exp_date, 2);
    wub_hex_2_str(srTransRec.szBatchNo, ECRResp.batch_no, 3);
    wub_hex_2_str(srTransRec.szTime, ECRResp.time, 3);
    wub_hex_2_str(srTransRec.szDate, szTempBuff, 3);
	strcpy(ECRResp.date,"16");
	memcpy(&ECRResp.date[2],szTempBuff,4);
	wub_hex_2_str(srTransRec.szTotalAmount, ECRResp.Trac_amout, 6);
	wub_hex_2_str(srTransRec.szBatchNo, ECRResp.batch_no, 3);
	strncpy(ECRResp.rref, srTransRec.szRRN, RET_REF_SIZE);

	if(srTransRec.HDTid == 53 && strTCT.fSMMode == TRUE){
		
		//add flag to send black auth code - SM FRG requirement
		if(get_env_int("BLNKAUTH") == TRUE)
			memset(ECRResp.auth_code, 0x20, sizeof(ECRResp.auth_code));
		else
			strncpy(ECRResp.auth_code, "APPROV", AUTH_CODE_SIZE);
	}else
		strncpy(ECRResp.auth_code, srTransRec.szAuthCode, AUTH_CODE_SIZE);

		if(strlen(srTransRec.szCardholderName) < CARD_NAME_SIZE)
			strcpy(ECRResp.card_name, srTransRec.szCardholderName);
		else
			memcpy(ECRResp.card_name, srTransRec.szCardholderName,CARD_NAME_SIZE);
	
	inHDTRead(srTransRec.HDTid);
	sprintf(ECRResp.entry_code,"%02d",strHDT.inIssuerID);
	
	/* Added */
    // Response Text
	memset(ECRResp.resp_text, 0x00, sizeof(ECRResp.resp_text));
	strcpy(ECRResp.resp_text, srTransRec.szECRRespText);

	// Merchant Name
	memset(ECRResp.merchant_name, 0x00, sizeof(ECRResp.merchant_name));
	strcpy(ECRResp.merchant_name, srTransRec.szECRMerchantName);

	// Formatted PAN
	memset(ECRResp.card_no, 0x00, sizeof(ECRResp.card_no));
	strcpy(ECRResp.card_no, srTransRec.szECRPANFormatted);
	
	// Expiry Date
	strcpy(ECRResp.exp_date, "****"); // BDO: Expiry date requirements -- sidumili
	/* Added */

	//SMACPay_CardSeqNo
	memset(ECRResp.szSMACPay_CardSeqNo, 0x00, sizeof(ECRResp.szSMACPay_CardSeqNo));
	memcpy(ECRResp.szSMACPay_CardSeqNo,srTransRec.bySMACPay_CardSeqNo,2);
	
	vdDebug_LogPrintf("response text:%s",ECRResp.resp_text);
	vdDebug_LogPrintf("tag02 Resp DE39:%s",ECRResp.resp_code);

	vdDebug_LogPrintf("tagD0 merchant_name:%s",ECRResp.merchant_name);
	vdDebug_LogPrintf("tag03 date:%s",ECRResp.date);
	vdDebug_LogPrintf("tag04 time:%s",ECRResp.time);
	vdDebug_LogPrintf("tag40 Amount:%s",ECRResp.Trac_amout);
	vdDebug_LogPrintf("tag01 Resp auth_code:%s",ECRResp.auth_code);
	vdDebug_LogPrintf("tag65 Inv Num:%s",ECRResp.inv_no);
	vdDebug_LogPrintf("tag16 Resp MID:%s",ECRResp.tid);
	vdDebug_LogPrintf("tagD1 Resp TID:%s",ECRResp.mid);
	vdDebug_LogPrintf("tagD2 card issuer name:%s",ECRResp.issuer);
	vdDebug_LogPrintf("tag30 Resp PAN:%s",ECRResp.card_no);
	vdDebug_LogPrintf("tag31 Resp ExpDate:%s",ECRResp.exp_date);
	vdDebug_LogPrintf("tag50 batch_no:%s",ECRResp.batch_no);
	vdDebug_LogPrintf("tagD3 Resp RREF:%s",ECRResp.rref);
	vdDebug_LogPrintf("tagD4 card issuer ID:%s",ECRResp.entry_code);
	vdDebug_LogPrintf("tagD5 card_name:%s",ECRResp.card_name);
   
	
	
	inECRSendResponse();
	return d_OK;
}

void vdSetLength(int inLength, char *out)
{
	char szTmp[5],szHex[5];
	
	sprintf((char *)szTmp, "%04d", inLength);
//	SVC_DSP_2_HEX((char *)szTmp ,(char *)szHex, 2);
	wub_str_2_hex((char *)szTmp ,(char *)szHex, 4);

	memcpy(out, szHex, 2);
}

int inECRSendComPacket(char* pchMsg, int inMsgSize, VS_BOOL fPadStxEtx,int inACKTimeOut,VS_BOOL fWaitForAck,int inMaxRetries) {
    char chResp;
    int inSize = 0;
    int inNumRetry = 0;
    char szECRSendBuf[4096 + 1];
    int inSendsize=0;	
    USHORT ret;
	ULONG tick;

	// patrick fix terminal send ECR resign command
	if (ECRPort == d_COM1 || ECRPort == d_COM2)
	{
		CTOS_RS232FlushRxBuffer(ECRPort);
		CTOS_RS232FlushTxBuffer(ECRPort);
	}
	
    memset(szECRSendBuf, 0, (int)sizeof(szECRSendBuf));
    if (fPadStxEtx == VS_TRUE) {
    	
        szECRSendBuf[inSize] = STX; 
        inSize++;
        memcpy(&(szECRSendBuf[1]), pchMsg, inMsgSize); 
        inSize += inMsgSize;
        szECRSendBuf[inSize] = ETX; 
        inSize++;
    } else {
        memcpy(szECRSendBuf, pchMsg, inMsgSize); 
        inSize = inMsgSize;
    }

//    szECRSendBuf[inSize] = (char) SVC_CRC_CALC(0, &(szECRSendBuf[1]), (inSize - 1));
    szECRSendBuf[inSize] = (char) ucGetLRC(&(szECRSendBuf[1]), (inSize - 1));
    inSize++;

	do {
		if (ECRPort == 0xFF)
		  ret = CTOS_USBTxReady();
		else
		  ret = CTOS_RS232TxReady(ECRPort);
		if (ret == d_OK)
			break;
	}while ((CTOS_TickGet() - tick) < d_READY_TIMEOUT);

	if (ret == d_OK) {
		if (ECRPort == 0xFF)
			CTOS_USBTxData(szECRSendBuf, inSize);
		else
			CTOS_RS232TxData(ECRPort, szECRSendBuf, inSize);

		if (fWaitForAck == VS_TRUE) {
			 chResp = NAK;				/* Wait for reply till Timeout */

			 if (ECRPort == 0xFF)
				 CTOS_USBRxData(&chResp, &lECRRecvLen);
			 else
				 CTOS_RS232RxData(ECRPort, &chResp, &lECRRecvLen);
		 } else
			 chResp = ACK;
	}

	
	//--sidunili
	if (strTCT.fPrintISOECR == TRUE){
		inPrintECRPacket("RX:ECR", szECRSendBuf, inSize);
	}
	
	
//	vdPCIDebug_HexPrintf("ECR Send Response", szECRSendBuf, inSize);
	
	
    if (chResp == ACK)
        return (VS_SUCCESS);
    else
        return (ERR_COM_NAK);  /* Too many NAKs, so indicate to the app */   	
}

int inECRSendResponse(void)
{
	int offset, inSize;
	int inSerialPort=0;
	char szDate[DATE_SIZE + 1];
    char szTime[TIME_SIZE + 1];
    CTOS_RTC SetRTC;
	char szTemp[AUTH_CODE_SIZE+ 1];
	char szTemp2[RET_REF_SIZE + 1];

	memset(szECRSendData,0x00,sizeof(szECRSendData));
	
	offset = 2;
	memcpy(&szECRSendData[offset], "60", TRANSPORT_HEADER_SIZE);
	offset += TRANSPORT_HEADER_SIZE;
	memcpy(&szECRSendData[offset], "00000000", TRANSPORT_DEST_SIZE + TRANSPORT_SRC_SIZE);
	offset += TRANSPORT_DEST_SIZE;
	offset += TRANSPORT_SRC_SIZE;
	memcpy(&szECRSendData[offset], "1", FORMAT_VERSION_SIZE);
	offset += FORMAT_VERSION_SIZE;
	memcpy(&szECRSendData[offset], "1", REQ_RESP_SIZE);
	offset += REQ_RESP_SIZE;
	memcpy(&szECRSendData[offset], ECRReq.txn_code, TXN_CODE_SIZE);
	offset += TXN_CODE_SIZE;
	memcpy(&szECRSendData[offset], ECRResp.resp_code, ECR_RESP_CODE_SIZE);
	offset += ECR_RESP_CODE_SIZE;
	memcpy(&szECRSendData[offset], "0", MORE_FOLLOW_SIZE);
	offset += MORE_FOLLOW_SIZE;
	szECRSendData[offset] = ECR_SEPARATOR;
	offset += END_PRESENT_SIZE;

/* - gcitra - remove - only sale should have Approval response test	
	memcpy(&szECRSendData[offset], ECR_RESP_TEXT_TAG, TAG_SIZE);
	offset += TAG_SIZE;
	vdSetLength(ECR_RESP_TEXT_SIZE, &szECRSendData[offset]);
	offset += LENGTH_SIZE;	
	if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))
		memcpy(&szECRSendData[offset], ECR_APPROVED_RESP, ECR_RESP_TEXT_SIZE);
	else 
		memcpy(&szECRSendData[offset], ECRResp.resp_text, ECR_RESP_TEXT_SIZE);
		
	offset += ECR_RESP_TEXT_SIZE;
	szECRSendData[offset] = ECR_SEPARATOR;
	offset += END_PRESENT_SIZE;
*/ 

    //-----------------------------------------------------------------------------------------------------------------------------------
	// UNSUCCESSFULL ECR TRXN
	//-----------------------------------------------------------------------------------------------------------------------------------
	if(memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))
	{
	    vdDebug_LogPrintf("txn Failed");
//gcitra
		memcpy(&szECRSendData[offset], ECR_RESP_TEXT_TAG, TAG_SIZE);
		offset += TAG_SIZE;
		vdSetLength(ECR_RESP_TEXT_SIZE, &szECRSendData[offset]);
		offset += LENGTH_SIZE;
		//gcitra
		memset(&ECRResp.resp_code[strlen(ECRResp.resp_code)],0x20,ECR_RESP_TEXT_SIZE-strlen(ECRResp.resp_code));
		//gcitra

		if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))
			memcpy(&szECRSendData[offset], ECR_APPROVED_RESP, ECR_RESP_TEXT_SIZE);
		if ((!memcmp(ECRResp.resp_code, ECR_UNKNOWN_ERR, ECR_RESP_CODE_SIZE)) ||
		   (!memcmp(ECRResp.resp_code, ECR_DECLINED_ERR, ECR_RESP_CODE_SIZE)))	
		{
			strcpy(ECRResp.resp_text,srTransRec.szECRRespText);
			memcpy(&szECRSendData[offset], ECRResp.resp_text, ECR_RESP_TEXT_SIZE);
		}
		else 
		{
			if(strlen(ECRResp.resp_code) == 0)
				strcpy(ECRResp.resp_text,srTransRec.szECRRespText);
			
			memcpy(&szECRSendData[offset], ECRResp.resp_text, ECR_RESP_TEXT_SIZE);
		}
			
		offset += ECR_RESP_TEXT_SIZE;
		szECRSendData[offset] = ECR_SEPARATOR;
		offset += END_PRESENT_SIZE;
//gcitra
		
		memcpy(&szECRSendData[offset], ECR_MERCHANT_NAME_TAG, TAG_SIZE);
		offset += TAG_SIZE;
		vdSetLength(ECR_MERCHANT_NAME_SIZE, &szECRSendData[offset]);
		offset += LENGTH_SIZE;
		//gcitra
		memset(&ECRResp.merchant_name[strlen(ECRResp.merchant_name)],0x20,ECR_MERCHANT_NAME_SIZE-strlen(ECRResp.merchant_name));
		//gcitra

		memset(ECRResp.merchant_name, 0x00, sizeof(ECRResp.merchant_name));
		strcpy(ECRResp.merchant_name, srTransRec.szECRMerchantName);
		memcpy(&szECRSendData[offset], ECRResp.merchant_name, ECR_MERCHANT_NAME_SIZE);
		offset += ECR_MERCHANT_NAME_SIZE;
		szECRSendData[offset] = ECR_SEPARATOR;
		offset += END_PRESENT_SIZE;

        //Read the date and the time //
        CTOS_RTCGet(&SetRTC);  

		// BDO: Requirements YYMMDD/HHMMSS -- sidumili
		/* Set Month & Day*/ 
		sprintf(ECRResp.date, "%02d%02d%02d", (int)SetRTC.bYear, (int)SetRTC.bMonth, (int)SetRTC.bDay);
		//sprintf(ECRResp.time,"%02d%02d%02d",SetRTC.bHour,SetRTC.bMinute,SetRTC.bSecond);
		
		memcpy(&szECRSendData[offset], ECR_DATE_TAG, TAG_SIZE);
		offset += TAG_SIZE;
		vdSetLength(DATE_SIZE, &szECRSendData[offset]);
		offset += LENGTH_SIZE;
		memcpy(&szECRSendData[offset], ECRResp.date, DATE_SIZE);
		offset += DATE_SIZE;
		szECRSendData[offset] = ECR_SEPARATOR;
		offset += END_PRESENT_SIZE;
			
		memcpy(&szECRSendData[offset], ECR_TIME_TAG, TAG_SIZE);
		offset += TAG_SIZE;
		vdSetLength(TIME_SIZE, &szECRSendData[offset]);
		offset += LENGTH_SIZE;
		memcpy(&szECRSendData[offset], ECRResp.time, TIME_SIZE);
		offset += TIME_SIZE;
		szECRSendData[offset] = ECR_SEPARATOR;
		offset += END_PRESENT_SIZE;

    }
    //-----------------------------------------------------------------------------------------------------------------------------------
	// UNSUCCESSFULL ECR TRXN
	//-----------------------------------------------------------------------------------------------------------------------------------
	
    //-----------------------------------------------------------------------------------------------------------------------------------
	// SUCCESSFULL ECR TRXN
	//-----------------------------------------------------------------------------------------------------------------------------------
	if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))
	{
	    vdDebug_LogPrintf("txn Success");

//gcitra    
		/*BDO: As per request VOID has complete data same with SALE -- sidumili*/
		if ((!memcmp(ECRReq.txn_code, ECR_SALE_TAG, 2)) || (!memcmp(ECRReq.txn_code, ECR_VOID_TAG, 2)) 
			|| (!memcmp(ECRReq.txn_code, ECR_CARDVER_TAG, 2)) || (!memcmp(ECRReq.txn_code, ECR_COMPLETION_TAG, 2))
			|| (!memcmp(ECRReq.txn_code, ECR_TIP_ADJ_TAG, 2)) || (!memcmp(ECRReq.txn_code, ECR_REPRINT_TAG, 2))
			|| (!memcmp(ECRReq.txn_code, ECR_INST_SALE_TAG, 2)) || (!memcmp(ECRReq.txn_code, ECR_INST_VOID_TAG, 2)))
		{
			//gcitra
			memcpy(&szECRSendData[offset], ECR_RESP_TEXT_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ECR_RESP_TEXT_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;		
			//gcitra
			memset(&ECRResp.resp_code[strlen(ECRResp.resp_code)],0x20,ECR_RESP_TEXT_SIZE-strlen(ECRResp.resp_code));
			//gcitra
			if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))
				memcpy(&szECRSendData[offset], ECR_APPROVED_RESP, ECR_RESP_TEXT_SIZE);
			else 
				memcpy(&szECRSendData[offset], ECRResp.resp_text, ECR_RESP_TEXT_SIZE);
				offset += ECR_RESP_TEXT_SIZE;
				szECRSendData[offset] = ECR_SEPARATOR;
				offset += END_PRESENT_SIZE;
			//gcitra

			
			memcpy(&szECRSendData[offset], ECR_MERCHANT_NAME_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ECR_MERCHANT_NAME_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			//gcitra
			memset(&ECRResp.merchant_name[strlen(ECRResp.merchant_name)],0x20,ECR_MERCHANT_NAME_SIZE-strlen(ECRResp.merchant_name));
			//gcitra
			memcpy(&szECRSendData[offset], ECRResp.merchant_name, ECR_MERCHANT_NAME_SIZE);
			offset += ECR_MERCHANT_NAME_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;

			//Read the date and the time //
	        CTOS_RTCGet(&SetRTC);  
		
			// BDO: Requirements YYMMDD/HHMMSS -- sidumili
			/* Set Month & Day*/
			sprintf(ECRResp.date, "%02d%02d%02d", (int)SetRTC.bYear, (int)SetRTC.bMonth, (int)SetRTC.bDay);
			//sprintf(ECRResp.time,"%02d%02d%02d",SetRTC.bHour,SetRTC.bMinute,SetRTC.bSecond);
			
			memcpy(&szECRSendData[offset], ECR_DATE_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(DATE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.date, DATE_SIZE);
			offset += DATE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
				
			memcpy(&szECRSendData[offset], ECR_TIME_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(TIME_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			
			memcpy(&szECRSendData[offset], ECRResp.time, TIME_SIZE);
			offset += TIME_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;

			/*Do not send amount on void trxn -- sidumili*/
			if(memcmp(ECRReq.txn_code, ECR_VOID_TAG, 2)){
				memcpy(&szECRSendData[offset], ECR_AMOUNT_TAG, TAG_SIZE);
				offset += TAG_SIZE;
				vdSetLength(AMOUNT_VALUE_SIZE, &szECRSendData[offset]);
				offset += LENGTH_SIZE;
				memcpy(&szECRSendData[offset], ECRReq.amount, AMOUNT_VALUE_SIZE);
				offset += AMOUNT_VALUE_SIZE;
				szECRSendData[offset] = ECR_SEPARATOR;
				offset += END_PRESENT_SIZE;
			}
			/*Do not send amount on void trxn -- sidumili*/

			memcpy(&szECRSendData[offset], ECR_AUTH_CODE_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(AUTH_CODE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			if(memcmp(ECRResp.auth_code,"Y1",2) == 0)
			{
				memset(szTemp,0x20,AUTH_CODE_SIZE);
				memcpy(szTemp,ECRResp.auth_code,2); //copy Y1
				memcpy(&szECRSendData[offset], szTemp, AUTH_CODE_SIZE);
			}
			else
				memcpy(&szECRSendData[offset], ECRResp.auth_code, AUTH_CODE_SIZE);

			offset += AUTH_CODE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_RESP_INV_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(INVOICE_NUMBER_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.inv_no, INVOICE_NUMBER_SIZE);
			offset += INVOICE_NUMBER_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
	/*
			memcpy(&szECRSendData[offset], ECR_MERCHANT_NAME_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ECR_MERCHANT_NAME_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.merchant_name, ECR_MERCHANT_NAME_SIZE);
			offset += ECR_MERCHANT_NAME_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
	*/		
			memcpy(&szECRSendData[offset], ECR_TID_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(TID_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.tid, TID_SIZE);
			offset += TID_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_MID_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(MID_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.mid, MID_SIZE);
			offset += MID_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_ISSUER_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ISSUER_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.issuer, ISSUER_SIZE);
			offset += ISSUER_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_CARD_NO_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			inSize = strlen(ECRResp.card_no);

			vdSetLength(inSize, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.card_no, inSize);
			offset += inSize;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_EXPIRY_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(EXP_DATE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.exp_date, EXP_DATE_SIZE);
			memcpy(&szECRSendData[offset], ECRResp.exp_date, EXP_DATE_SIZE);
			offset += EXP_DATE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_BATCH_NO_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(BATCH_NUM_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.batch_no, BATCH_NUM_SIZE);
			offset += BATCH_NUM_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
	/*
			memcpy(&szECRSendData[offset], ECR_DATE_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(DATE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.date, DATE_SIZE);
			offset += DATE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
				
			memcpy(&szECRSendData[offset], ECR_TIME_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(TIME_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.time, TIME_SIZE);
			offset += TIME_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
	*/		
			memcpy(&szECRSendData[offset], ECR_RREF_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(RET_REF_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			if(memcmp(ECRResp.auth_code,"Y1",2) == 0)
			{
				memset(szTemp2,0x20,RET_REF_SIZE);
				memcpy(&szECRSendData[offset], szTemp2, RET_REF_SIZE);
			}
			else
				memcpy(&szECRSendData[offset], ECRResp.rref, RET_REF_SIZE);

			offset += RET_REF_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;

			memcpy(&szECRSendData[offset], ECR_ISSUERID_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ISSUERID_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.entry_code, ISSUERID_SIZE);
			offset += ISSUERID_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			if(strlen(ECRResp.card_name) > 0)
			{
				memcpy(&szECRSendData[offset], ECR_CARD_NAME_TAG, TAG_SIZE);
				offset += TAG_SIZE;
				vdSetLength(CARD_NAME_SIZE, &szECRSendData[offset]);
				offset += LENGTH_SIZE;
				//gcitra
				memset(&ECRResp.card_name[strlen(ECRResp.card_name)],0x20,CARD_NAME_SIZE-strlen(ECRResp.card_name));
				//gcitra
				memcpy(&szECRSendData[offset], ECRResp.card_name, CARD_NAME_SIZE);
				offset += CARD_NAME_SIZE;
				szECRSendData[offset] = ECR_SEPARATOR;
				offset += END_PRESENT_SIZE;
			}
			else
			{
				if(fWatsonsFlag == TRUE)
				{
					memset(ECRResp.card_name,0x20,CARD_NAME_SIZE);//Set Cardholder name to all blanks.
					memcpy(&szECRSendData[offset], ECR_CARD_NAME_TAG, TAG_SIZE);
					offset += TAG_SIZE;
					vdSetLength(CARD_NAME_SIZE, &szECRSendData[offset]);
					offset += LENGTH_SIZE;
					//gcitra
					memset(&ECRResp.card_name[strlen(ECRResp.card_name)],0x20,CARD_NAME_SIZE-strlen(ECRResp.card_name));
					//gcitra
					memcpy(&szECRSendData[offset], ECRResp.card_name, CARD_NAME_SIZE);
					offset += CARD_NAME_SIZE;
					szECRSendData[offset] = ECR_SEPARATOR;
					offset += END_PRESENT_SIZE;
					fWatsonsFlag = FALSE;
				}
			}

			if(!memcmp(ECRReq.txn_code, ECR_PTS_AWARDING_TAG, 2))
			{
				memcpy(&szECRSendData[offset], ECR_CARD_SEQ_NO_TAG, TAG_SIZE);
				offset += TAG_SIZE;
				vdSetLength(CARD_SEQ_NO_SIZE, &szECRSendData[offset]);
				offset += LENGTH_SIZE;
				memcpy(&szECRSendData[offset], ECRResp.szSMACPay_CardSeqNo, CARD_SEQ_NO_SIZE);
				offset += CARD_SEQ_NO_SIZE;
				szECRSendData[offset] = ECR_SEPARATOR;
				offset += END_PRESENT_SIZE;				
			}

			if(get_env_int("NEWECR") == TRUE)
			{
				if(!memcmp(ECRReq.txn_code, ECR_SALE_TAG, 2) && srTransRec.byEntryMode == CARD_ENTRY_WAVE)
				{
					memcpy(&szECRSendData[offset], ECR_CARD_SEQ_NO_TAG, TAG_SIZE);
					offset += TAG_SIZE;
					vdSetLength(CARD_SEQ_NO_SIZE, &szECRSendData[offset]);
					offset += LENGTH_SIZE;
					memcpy(&szECRSendData[offset], ECRResp.szSMACPay_CardSeqNo, CARD_SEQ_NO_SIZE);
					offset += CARD_SEQ_NO_SIZE;
					szECRSendData[offset] = ECR_SEPARATOR;
					offset += END_PRESENT_SIZE;				
				}
				
				vdDebug_LogPrintf("srTransRec.byEntryMode is %d",srTransRec.byEntryMode);
				memcpy(&szECRSendData[offset], ECR_ENTRY_MODE_TAG, TAG_SIZE);
				offset += TAG_SIZE;
				vdSetLength(CARD_ENTRY_MODE_SIZE, &szECRSendData[offset]);
				offset += LENGTH_SIZE;
				if(srTransRec.byEntryMode == CARD_ENTRY_MANUAL)
					memcpy(&szECRSendData[offset], "12", CARD_ENTRY_MODE_SIZE);
				else if(srTransRec.byEntryMode == CARD_ENTRY_MSR)
					memcpy(&szECRSendData[offset], "22", CARD_ENTRY_MODE_SIZE);
				else if(srTransRec.byEntryMode == CARD_ENTRY_WAVE)
					memcpy(&szECRSendData[offset], "72", CARD_ENTRY_MODE_SIZE);
				else if(srTransRec.byEntryMode == CARD_ENTRY_ICC)
					memcpy(&szECRSendData[offset], "52", CARD_ENTRY_MODE_SIZE);
				
				offset += CARD_ENTRY_MODE_SIZE;
				szECRSendData[offset] = ECR_SEPARATOR;
				offset += END_PRESENT_SIZE;
			}
			
		}
		else if(!memcmp(ECRReq.txn_code, ECR_KIT_SALE_TAG, 2) || !memcmp(ECRReq.txn_code, ECR_RENEWAL_TAG, 2))
		{
//RESPONSE TEXT
			memcpy(&szECRSendData[offset], ECR_RESP_TEXT_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ECR_RESP_TEXT_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;		
			
			memset(&ECRResp.resp_code[strlen(ECRResp.resp_code)],0x20,ECR_RESP_TEXT_SIZE-strlen(ECRResp.resp_code));
		
			if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))
				memcpy(&szECRSendData[offset], ECR_APPROVED_RESP, ECR_RESP_TEXT_SIZE);
			else 
				memcpy(&szECRSendData[offset], ECRResp.resp_text, ECR_RESP_TEXT_SIZE);
			
			offset += ECR_RESP_TEXT_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
		

//AUTH CODE
			memcpy(&szECRSendData[offset], ECR_AUTH_CODE_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(AUTH_CODE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			if(memcmp(ECRResp.auth_code,"Y1",2) == 0)
			{
				memset(szTemp,0x20,AUTH_CODE_SIZE);
				memcpy(szTemp,ECRResp.auth_code,2); //copy Y1
				memcpy(&szECRSendData[offset], szTemp, AUTH_CODE_SIZE);
			}
			else
				memcpy(&szECRSendData[offset], ECRResp.auth_code, AUTH_CODE_SIZE);

			offset += AUTH_CODE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;

//INVOICE NUMBER
			memcpy(&szECRSendData[offset], ECR_RESP_INV_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(INVOICE_NUMBER_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.inv_no, INVOICE_NUMBER_SIZE);
			offset += INVOICE_NUMBER_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;


//MERCHANT ADDRESS
			memcpy(&szECRSendData[offset], ECR_MERCHANT_NAME_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ECR_MERCHANT_NAME_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memset(&ECRResp.merchant_name[strlen(ECRResp.merchant_name)],0x20,ECR_MERCHANT_NAME_SIZE-strlen(ECRResp.merchant_name));
			memcpy(&szECRSendData[offset], ECRResp.merchant_name, ECR_MERCHANT_NAME_SIZE);
			offset += ECR_MERCHANT_NAME_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;


//TID
			memcpy(&szECRSendData[offset], ECR_TID_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(TID_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.tid, TID_SIZE);
			offset += TID_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;


//MID			
			memcpy(&szECRSendData[offset], ECR_MID_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(MID_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.mid, MID_SIZE);
			offset += MID_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;


//ISSUER NAME
			memcpy(&szECRSendData[offset], ECR_ISSUER_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ISSUER_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.issuer, ISSUER_SIZE);
			offset += ISSUER_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;


//CARD NO
			memcpy(&szECRSendData[offset], ECR_CARD_NO_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			inSize = strlen(ECRResp.card_no);
			vdSetLength(inSize, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.card_no, inSize);
			offset += inSize;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;


//EXPIRY DATE
			memcpy(&szECRSendData[offset], ECR_EXPIRY_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(EXP_DATE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.exp_date, EXP_DATE_SIZE);
			memcpy(&szECRSendData[offset], ECRResp.exp_date, EXP_DATE_SIZE);
			offset += EXP_DATE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			

			memcpy(&szECRSendData[offset], ECR_BATCH_NO_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(BATCH_NUM_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.batch_no, BATCH_NUM_SIZE);
			offset += BATCH_NUM_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;			


//TXN DATE
			//Read the date and the time //
	        CTOS_RTCGet(&SetRTC);
			sprintf(ECRResp.date, "%02d%02d%02d", (int)SetRTC.bYear, (int)SetRTC.bMonth, (int)SetRTC.bDay);
			memcpy(&szECRSendData[offset], ECR_DATE_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(DATE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.date, DATE_SIZE);
			offset += DATE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;


//TXN TIME				
			memcpy(&szECRSendData[offset], ECR_TIME_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(TIME_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.time, TIME_SIZE);
			offset += TIME_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;


//RRN
			memcpy(&szECRSendData[offset], ECR_RREF_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(RET_REF_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			if(memcmp(ECRResp.auth_code,"Y1",2) == 0)
			{
				memset(szTemp2,0x20,RET_REF_SIZE);
				memcpy(&szECRSendData[offset], szTemp2, RET_REF_SIZE);
			}
			else
				memcpy(&szECRSendData[offset], ECRResp.rref, RET_REF_SIZE);
			offset += RET_REF_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;

			
//ISSUER ID
			memcpy(&szECRSendData[offset], ECR_ISSUERID_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ISSUERID_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.entry_code, ISSUERID_SIZE);
			offset += ISSUERID_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;


//CARDHOLDER NAME
			if(strlen(ECRResp.card_name) > 0)
			{
				memcpy(&szECRSendData[offset], ECR_CARD_NAME_TAG, TAG_SIZE);
				offset += TAG_SIZE;
				vdSetLength(CARD_NAME_SIZE, &szECRSendData[offset]);
				offset += LENGTH_SIZE;
				//gcitra
				memset(&ECRResp.card_name[strlen(ECRResp.card_name)],0x20,CARD_NAME_SIZE-strlen(ECRResp.card_name));
				//gcitra
				memcpy(&szECRSendData[offset], ECRResp.card_name, CARD_NAME_SIZE);
				offset += CARD_NAME_SIZE;
				szECRSendData[offset] = ECR_SEPARATOR;
				offset += END_PRESENT_SIZE;
			}
			else
			{
				if(fWatsonsFlag == TRUE)
				{
					memset(ECRResp.card_name,0x20,CARD_NAME_SIZE);//Set Cardholder name to all blanks.
					memcpy(&szECRSendData[offset], ECR_CARD_NAME_TAG, TAG_SIZE);
					offset += TAG_SIZE;
					vdSetLength(CARD_NAME_SIZE, &szECRSendData[offset]);
					offset += LENGTH_SIZE;
					//gcitra
					memset(&ECRResp.card_name[strlen(ECRResp.card_name)],0x20,CARD_NAME_SIZE-strlen(ECRResp.card_name));
					//gcitra
					memcpy(&szECRSendData[offset], ECRResp.card_name, CARD_NAME_SIZE);
					offset += CARD_NAME_SIZE;
					szECRSendData[offset] = ECR_SEPARATOR;
					offset += END_PRESENT_SIZE;
					fWatsonsFlag = FALSE;
				}
			}
			

//CARD SEQ NO
			memcpy(&szECRSendData[offset], ECR_CARD_SEQ_NO_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(CARD_SEQ_NO_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.szSMACPay_CardSeqNo, CARD_SEQ_NO_SIZE);
			offset += CARD_SEQ_NO_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
		}
		else if(!memcmp(ECRReq.txn_code, ECR_PTS_AWARDING_TAG, 2))
		{
			//gcitra
			memcpy(&szECRSendData[offset], ECR_RESP_TEXT_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ECR_RESP_TEXT_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;		
			//gcitra
			memset(&ECRResp.resp_code[strlen(ECRResp.resp_code)],0x20,ECR_RESP_TEXT_SIZE-strlen(ECRResp.resp_code));
			//gcitra
			if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))
				memcpy(&szECRSendData[offset], ECR_APPROVED_RESP, ECR_RESP_TEXT_SIZE);
			else 
				memcpy(&szECRSendData[offset], ECRResp.resp_text, ECR_RESP_TEXT_SIZE);
				offset += ECR_RESP_TEXT_SIZE;
				szECRSendData[offset] = ECR_SEPARATOR;
				offset += END_PRESENT_SIZE;
			//gcitra

			
			memcpy(&szECRSendData[offset], ECR_MERCHANT_NAME_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ECR_MERCHANT_NAME_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			//gcitra
			memset(&ECRResp.merchant_name[strlen(ECRResp.merchant_name)],0x20,ECR_MERCHANT_NAME_SIZE-strlen(ECRResp.merchant_name));
			//gcitra
			memcpy(&szECRSendData[offset], ECRResp.merchant_name, ECR_MERCHANT_NAME_SIZE);
			offset += ECR_MERCHANT_NAME_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;

			//Read the date and the time //
	        CTOS_RTCGet(&SetRTC);  
		
			// BDO: Requirements YYMMDD/HHMMSS -- sidumili
			/* Set Month & Day*/
			sprintf(ECRResp.date, "%02d%02d%02d", (int)SetRTC.bYear, (int)SetRTC.bMonth, (int)SetRTC.bDay);
			//sprintf(ECRResp.time,"%02d%02d%02d",SetRTC.bHour,SetRTC.bMinute,SetRTC.bSecond);
			
			memcpy(&szECRSendData[offset], ECR_DATE_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(DATE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.date, DATE_SIZE);
			offset += DATE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
				
			memcpy(&szECRSendData[offset], ECR_TIME_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(TIME_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;

			if(strlen(ECRResp.time) <= 0)
				memset(ECRResp.time,0x20,sizeof(ECRResp.time));
				
			memcpy(&szECRSendData[offset], ECRResp.time, TIME_SIZE);
			offset += TIME_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;

			/*Do not send amount on void trxn -- sidumili*/
			if(memcmp(ECRReq.txn_code, ECR_VOID_TAG, 2)){
				memcpy(&szECRSendData[offset], ECR_AMOUNT_TAG, TAG_SIZE);
				offset += TAG_SIZE;
				vdSetLength(AMOUNT_VALUE_SIZE, &szECRSendData[offset]);
				offset += LENGTH_SIZE;
				memcpy(&szECRSendData[offset], ECRReq.amount, AMOUNT_VALUE_SIZE);
				offset += AMOUNT_VALUE_SIZE;
				szECRSendData[offset] = ECR_SEPARATOR;
				offset += END_PRESENT_SIZE;
			}
			/*Do not send amount on void trxn -- sidumili*/

			memcpy(&szECRSendData[offset], ECR_AUTH_CODE_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(AUTH_CODE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;

			if(strlen(ECRResp.auth_code) <= 0)
				memset(ECRResp.auth_code,0x20,sizeof(ECRResp.auth_code));
			
			if(memcmp(ECRResp.auth_code,"Y1",2) == 0)
			{
				memset(szTemp,0x20,AUTH_CODE_SIZE);
				memcpy(szTemp,ECRResp.auth_code,2); //copy Y1
				memcpy(&szECRSendData[offset], szTemp, AUTH_CODE_SIZE);
			}
			else
				memcpy(&szECRSendData[offset], ECRResp.auth_code, AUTH_CODE_SIZE);

			offset += AUTH_CODE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_RESP_INV_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(INVOICE_NUMBER_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.inv_no, INVOICE_NUMBER_SIZE);
			offset += INVOICE_NUMBER_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
	/*
			memcpy(&szECRSendData[offset], ECR_MERCHANT_NAME_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ECR_MERCHANT_NAME_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.merchant_name, ECR_MERCHANT_NAME_SIZE);
			offset += ECR_MERCHANT_NAME_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
	*/		
			memcpy(&szECRSendData[offset], ECR_TID_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(TID_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.tid, TID_SIZE);
			offset += TID_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_MID_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(MID_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.mid, MID_SIZE);
			offset += MID_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_ISSUER_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ISSUER_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.issuer, ISSUER_SIZE);
			offset += ISSUER_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_CARD_NO_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			inSize = strlen(ECRResp.card_no);

			vdSetLength(inSize, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.card_no, inSize);
			offset += inSize;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_EXPIRY_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(EXP_DATE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.exp_date, EXP_DATE_SIZE);
			memcpy(&szECRSendData[offset], ECRResp.exp_date, EXP_DATE_SIZE);
			offset += EXP_DATE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			memcpy(&szECRSendData[offset], ECR_BATCH_NO_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(BATCH_NUM_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.batch_no, BATCH_NUM_SIZE);
			offset += BATCH_NUM_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
	/*
			memcpy(&szECRSendData[offset], ECR_DATE_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(DATE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.date, DATE_SIZE);
			offset += DATE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
				
			memcpy(&szECRSendData[offset], ECR_TIME_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(TIME_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.time, TIME_SIZE);
			offset += TIME_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
	*/		
			memcpy(&szECRSendData[offset], ECR_RREF_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(RET_REF_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;

			if(strlen(ECRResp.rref) <= 0)
				memset(ECRResp.rref,0x20,sizeof(ECRResp.rref));
			
			if(memcmp(ECRResp.auth_code,"Y1",2) == 0)
			{
				memset(szTemp2,0x20,RET_REF_SIZE);
				memcpy(&szECRSendData[offset], szTemp2, RET_REF_SIZE);
			}
			else
				memcpy(&szECRSendData[offset], ECRResp.rref, RET_REF_SIZE);

			offset += RET_REF_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;

			memcpy(&szECRSendData[offset], ECR_ISSUERID_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ISSUERID_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.entry_code, ISSUERID_SIZE);
			offset += ISSUERID_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
			if(strlen(ECRResp.card_name) > 0)
			{
				memcpy(&szECRSendData[offset], ECR_CARD_NAME_TAG, TAG_SIZE);
				offset += TAG_SIZE;
				vdSetLength(CARD_NAME_SIZE, &szECRSendData[offset]);
				offset += LENGTH_SIZE;
				//gcitra
				memset(&ECRResp.card_name[strlen(ECRResp.card_name)],0x20,CARD_NAME_SIZE-strlen(ECRResp.card_name));
				//gcitra
				memcpy(&szECRSendData[offset], ECRResp.card_name, CARD_NAME_SIZE);
				offset += CARD_NAME_SIZE;
				szECRSendData[offset] = ECR_SEPARATOR;
				offset += END_PRESENT_SIZE;
			}
			else
			{
				if(fWatsonsFlag == TRUE)
				{
					memset(ECRResp.card_name,0x20,CARD_NAME_SIZE);//Set Cardholder name to all blanks.
					memcpy(&szECRSendData[offset], ECR_CARD_NAME_TAG, TAG_SIZE);
					offset += TAG_SIZE;
					vdSetLength(CARD_NAME_SIZE, &szECRSendData[offset]);
					offset += LENGTH_SIZE;
					//gcitra
					memset(&ECRResp.card_name[strlen(ECRResp.card_name)],0x20,CARD_NAME_SIZE-strlen(ECRResp.card_name));
					//gcitra
					memcpy(&szECRSendData[offset], ECRResp.card_name, CARD_NAME_SIZE);
					offset += CARD_NAME_SIZE;
					szECRSendData[offset] = ECR_SEPARATOR;
					offset += END_PRESENT_SIZE;
					fWatsonsFlag = FALSE;
				}
			}

			if(!memcmp(ECRReq.txn_code, ECR_PTS_AWARDING_TAG, 2) 
				|| (!memcmp(ECRReq.txn_code, ECR_SALE_TAG, 2) && srTransRec.byEntryMode == CARD_ENTRY_WAVE) )
			{
				memcpy(&szECRSendData[offset], ECR_CARD_SEQ_NO_TAG, TAG_SIZE);
				offset += TAG_SIZE;
				vdSetLength(CARD_SEQ_NO_SIZE, &szECRSendData[offset]);
				offset += LENGTH_SIZE;
				memcpy(&szECRSendData[offset], ECRResp.szSMACPay_CardSeqNo, CARD_SEQ_NO_SIZE);
				offset += CARD_SEQ_NO_SIZE;
				szECRSendData[offset] = ECR_SEPARATOR;
				offset += END_PRESENT_SIZE;				
			}

			vdDebug_LogPrintf("srTransRec.byEntryMode is %d",srTransRec.byEntryMode);
			memcpy(&szECRSendData[offset], ECR_ENTRY_MODE_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(CARD_ENTRY_MODE_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			if(srTransRec.byEntryMode == CARD_ENTRY_MANUAL)
				memcpy(&szECRSendData[offset], "12", CARD_ENTRY_MODE_SIZE);
			else if(srTransRec.byEntryMode == CARD_ENTRY_MSR)
				memcpy(&szECRSendData[offset], "22", CARD_ENTRY_MODE_SIZE);
			else if(srTransRec.byEntryMode == CARD_ENTRY_WAVE)
				memcpy(&szECRSendData[offset], "72", CARD_ENTRY_MODE_SIZE);
			
			offset += CARD_ENTRY_MODE_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
			
		}
		else if(!memcmp(ECRReq.txn_code, ECR_SETTLEMENT_ALL_TAG, 2))
		{
			//gcitra
			memcpy(&szECRSendData[offset], ECR_RESP_TEXT_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ECR_RESP_TEXT_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;		
			//gcitra
			memset(&ECRResp.resp_code[strlen(ECRResp.resp_code)],0x20,ECR_RESP_TEXT_SIZE-strlen(ECRResp.resp_code));
			//gcitra
			if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))
				memcpy(&szECRSendData[offset], ECR_APPROVED_RESP, ECR_RESP_TEXT_SIZE);
			else 
				memcpy(&szECRSendData[offset], ECRResp.resp_text, ECR_RESP_TEXT_SIZE);
				offset += ECR_RESP_TEXT_SIZE;
				szECRSendData[offset] = ECR_SEPARATOR;
				offset += END_PRESENT_SIZE;
			//gcitra

			
			memcpy(&szECRSendData[offset], ECR_MERCHANT_NAME_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(ECR_MERCHANT_NAME_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			//gcitra
			memset(&ECRResp.merchant_name[strlen(ECRResp.merchant_name)],0x20,ECR_MERCHANT_NAME_SIZE-strlen(ECRResp.merchant_name));
			//gcitra
			memcpy(&szECRSendData[offset], ECRResp.merchant_name, ECR_MERCHANT_NAME_SIZE);
			offset += ECR_MERCHANT_NAME_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
            offset += END_PRESENT_SIZE;
		}
        else if (!memcmp(ECRReq.txn_code, ECR_PIN_VERIFY, 2) )
        {
            //RESPONSE TEXT
            memcpy(&szECRSendData[offset], ECR_RESP_TEXT_TAG, TAG_SIZE);
            offset += TAG_SIZE;
            vdSetLength(ECR_RESP_TEXT_SIZE, &szECRSendData[offset]);
            offset += LENGTH_SIZE;		

            memset(&ECRResp.resp_code[strlen(ECRResp.resp_code)],0x20,ECR_RESP_TEXT_SIZE-strlen(ECRResp.resp_code));

            if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))
            memcpy(&szECRSendData[offset], ECR_APPROVED_RESP, ECR_RESP_TEXT_SIZE);
            else 
            memcpy(&szECRSendData[offset], ECRResp.resp_text, ECR_RESP_TEXT_SIZE);

            offset += ECR_RESP_TEXT_SIZE;
            szECRSendData[offset] = ECR_SEPARATOR;
            offset += END_PRESENT_SIZE;


            //AUTH CODE
            memcpy(&szECRSendData[offset], ECR_AUTH_CODE_TAG, TAG_SIZE);
            offset += TAG_SIZE;
            vdSetLength(AUTH_CODE_SIZE, &szECRSendData[offset]);
            offset += LENGTH_SIZE;
            if(memcmp(ECRResp.auth_code,"Y1",2) == 0)
            {
            memset(szTemp,0x20,AUTH_CODE_SIZE);
            memcpy(szTemp,ECRResp.auth_code,2); //copy Y1
            memcpy(&szECRSendData[offset], szTemp, AUTH_CODE_SIZE);
            }
            else
            memcpy(&szECRSendData[offset], ECRResp.auth_code, AUTH_CODE_SIZE);
            
            offset += AUTH_CODE_SIZE;
            szECRSendData[offset] = ECR_SEPARATOR;
            offset += END_PRESENT_SIZE;

            //INVOICE NUMBER
            memcpy(&szECRSendData[offset], ECR_RESP_INV_TAG, TAG_SIZE);
            offset += TAG_SIZE;
            vdSetLength(INVOICE_NUMBER_SIZE, &szECRSendData[offset]);
            offset += LENGTH_SIZE;
            memcpy(&szECRSendData[offset], ECRResp.inv_no, INVOICE_NUMBER_SIZE);
            offset += INVOICE_NUMBER_SIZE;
            szECRSendData[offset] = ECR_SEPARATOR;
            offset += END_PRESENT_SIZE;


            //TID
            memcpy(&szECRSendData[offset], ECR_TID_TAG, TAG_SIZE);
            offset += TAG_SIZE;
            vdSetLength(TID_SIZE, &szECRSendData[offset]);
            offset += LENGTH_SIZE;
            memcpy(&szECRSendData[offset], ECRResp.tid, TID_SIZE);
            offset += TID_SIZE;
            szECRSendData[offset] = ECR_SEPARATOR;
            offset += END_PRESENT_SIZE;


            //MID			
            memcpy(&szECRSendData[offset], ECR_MID_TAG, TAG_SIZE);
            offset += TAG_SIZE;
            vdSetLength(MID_SIZE, &szECRSendData[offset]);
            offset += LENGTH_SIZE;
            memcpy(&szECRSendData[offset], ECRResp.mid, MID_SIZE);
            offset += MID_SIZE;
            szECRSendData[offset] = ECR_SEPARATOR;
            offset += END_PRESENT_SIZE;

            //CARD NO
            memcpy(&szECRSendData[offset], ECR_CARD_NO_TAG, TAG_SIZE);
            offset += TAG_SIZE;
            inSize = strlen(ECRResp.card_no);
            vdSetLength(inSize, &szECRSendData[offset]);
            offset += LENGTH_SIZE;
            memcpy(&szECRSendData[offset], ECRResp.card_no, inSize);
            offset += inSize;
            szECRSendData[offset] = ECR_SEPARATOR;
            offset += END_PRESENT_SIZE;
		}
		else if (!memcmp(ECRReq.txn_code, ECR_COMM_TEST_TAG, 2))
		{
			//Do not send body. Send only the header.
		}
		
		/*BDO: As per request VOID has complete data same with SALE -- sidumili*/
		#if 0
		//add response for void - gcitra
		else if(!memcmp(ECRReq.txn_code, ECR_VOID_TAG, 2)){
			memcpy(&szECRSendData[offset], ECR_RESP_INV_TAG, TAG_SIZE);
			offset += TAG_SIZE;
			vdSetLength(INVOICE_NUMBER_SIZE, &szECRSendData[offset]);
			offset += LENGTH_SIZE;
			memcpy(&szECRSendData[offset], ECRResp.inv_no, INVOICE_NUMBER_SIZE);
			offset += INVOICE_NUMBER_SIZE;
			szECRSendData[offset] = ECR_SEPARATOR;
			offset += END_PRESENT_SIZE;
		}
		//add response for void - gcitra
		#endif
		/*BDO: As per request VOID has complete data same with SALE -- sidumili*/
		
	}
    //-----------------------------------------------------------------------------------------------------------------------------------
	// SUCCESSFULL ECR TRXN
	//-----------------------------------------------------------------------------------------------------------------------------------
#if 0
	// Debug
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::::::::::LEN[%d]|ECRReq.txn_code[%s]", strlen(ECRReq.txn_code), ECRReq.txn_code);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|srTransRec.szECRRespText[%s]", ECR_RESP_TEXT_TAG, strlen(srTransRec.szECRRespText), srTransRec.szECRRespText);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|merchant_name[%s]", ECR_MERCHANT_NAME_TAG, strlen(ECRResp.merchant_name), ECRResp.merchant_name);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|date[%s]", ECR_DATE_TAG, strlen(ECRResp.date), ECRResp.date);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|time[%s]", ECR_TIME_TAG, strlen(ECRResp.time), ECRResp.time);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|ECRReq.amount[%s]", ECR_AMOUNT_TAG, strlen(ECRReq.amount), ECRReq.amount);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|auth_code[%s]", ECR_AUTH_CODE_TAG, strlen(ECRResp.auth_code), ECRResp.auth_code);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|inv_no[%s]", ECR_RESP_INV_TAG, strlen(ECRResp.inv_no), ECRResp.inv_no);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|tid[%s]", ECR_TID_TAG, strlen(ECRResp.tid), ECRResp.tid);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|mid[%s]", ECR_MID_TAG, strlen(ECRResp.mid), ECRResp.mid);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|issuer[%s]", ECR_ISSUER_TAG, strlen(ECRResp.issuer), ECRResp.issuer);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|card_no[%s]", ECR_CARD_NO_TAG, strlen(ECRResp.card_no), ECRResp.card_no);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|exp_date[%s]", ECR_EXPIRY_TAG, strlen(ECRResp.exp_date), ECRResp.exp_date);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|batch_no[%s]", ECR_BATCH_NO_TAG, strlen(ECRResp.batch_no), ECRResp.batch_no);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|rref[%s]", ECR_RREF_TAG, strlen(ECRResp.rref), ECRResp.rref);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|entry_code[%s]", ECR_ISSUERID_TAG, strlen(ECRResp.entry_code), ECRResp.entry_code);
	//vdDebug_LogPrintf("DEBUG ECR RESPONSE::TAG[%s]|LEN[%d]|card_name[%s]", ECR_CARD_NAME_TAG, strlen(ECRResp.card_name), ECRResp.card_name);
	// Debug
#endif
	
	vdDebug_LogPrintf("Response Len[%d]", offset);	

	vdSetLength(offset-2, &szECRSendData[0]);
		
	vdDebug_LogPrintf("ECR send offset[%d]", offset);

	inECRSendComPacket(szECRSendData, offset,VS_TRUE, 5,VS_TRUE, 3); 

	inSaveISOECRLog(VS_TRUE, szECRSendData, offset); /*Save ECR Send/Receive data -- sidumili*/

	return VS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DISPLAY_POSITION_LEFT 0
#define DISPLAY_POSITION_CENTER 1
#define DISPLAY_POSITION_RIGHT 2
#define DISPLAY_LINE_SIZE 16

void setLCDPrint(int line,int position, char *pbBuf)
{
    int iInitX = 0;
    int lens = 0;
    switch(position)
    {
        case DISPLAY_POSITION_LEFT:
            CTOS_LCDTPrintXY(1, line, pbBuf);
            break;
        case DISPLAY_POSITION_CENTER:
            lens = strlen(pbBuf);
            iInitX = (16 - lens) / 2 + 1;
            CTOS_LCDTPrintXY(iInitX, line, pbBuf);
            break;
        case DISPLAY_POSITION_RIGHT:
            lens = strlen(pbBuf);
            iInitX = 16 - lens + 1;
            CTOS_LCDTPrintXY(iInitX, line, pbBuf);
            break;
    }
}

void vduiWarningSound(void)
{
	CTOS_LEDSet(d_LED1, d_ON);
	CTOS_LEDSet(d_LED2, d_ON);
	CTOS_LEDSet(d_LED3, d_ON);
//	CTOS_BackLightSet (d_BKLIT_LCD, d_ON);
	//CTOS_Sound(6000, 50); // in 10 ms
	//CTOS_Delay(500);
	//CTOS_Sound(6000, 50); // in 10 ms
	CTOS_Beep();
	CTOS_Delay(300);
	CTOS_Beep();
	CTOS_LEDSet(d_LED1, d_OFF);
	CTOS_LEDSet(d_LED2, d_OFF);
	CTOS_LEDSet(d_LED3, d_OFF);
//	CTOS_BackLightSet (d_BKLIT_LCD, d_OFF);
}

void vduiLightOn(void)
{
  	CTOS_BackLightSetEx(d_BKLIT_LCD,d_ON,80000);
}

USHORT usCTOSS_ShareECRInitialize(void)
{
    BYTE	byEMVConfig[30];
    BYTE    bStatus[4];
    USHORT  usResult;
    USHORT  usLen;

	usResult = inECR_InitCOM();
    
    return usResult;
}


/////////////////////////////////////////////////////////////////////////////////////////
short shPrintCheckPaper(void)
{
	unsigned short inRet;
	unsigned char key;
	
	while(1)
	{
		inRet = CTOS_PrinterStatus();
		if (inRet==d_OK)
			return 0;
		else if(inRet==d_PRINTER_PAPER_OUT)
		{
			return -1;	
		}		
	}	
}

int inPrintECRPacket(char *pucTitle, unsigned char *pucMessage, int inLen)
{
	char ucLineBuffer[44 + 4];
	unsigned char *pucBuff;
	int inBuffPtr = 0;
	BYTE baTemp[384 * 64];
	char szStr[44 + 4];
	

	if (inLen <= 0)
		return(ST_SUCCESS);

	
	CTOS_PrinterSetWorkTime(50000,1000);
	inCTOS_SelectFont(d_FONT_FNT_MODE,d_FONT_16x16,0," ");
	vdSetGolbFontAttrib(d_FONT_16x16, 1, 2, 0, 0);
	
	memset(szStr, 0x00, sizeof(szStr));
	memset(baTemp, 0x00, sizeof(baTemp));
	sprintf(szStr,"[%s] [%d] \n", pucTitle, inLen);
	CTOS_PrinterBufferPutString((BYTE *)baTemp, 1, 1, szStr, &stgFONT_ATTRIB);
	CTOS_PrinterBufferOutput((BYTE *)baTemp, 3);

		
	CTOS_PrinterFline(12); 
	
	
	pucBuff = pucMessage + inLen;
	while (pucBuff > pucMessage)
	{
	memset(ucLineBuffer,0x00, sizeof(ucLineBuffer));
	for (inBuffPtr = 0; (inBuffPtr < 44) && (pucBuff > pucMessage); inBuffPtr += 3)
	{
	sprintf(&ucLineBuffer[inBuffPtr], "%02X ", *pucMessage);
	pucMessage++;
	}
	ucLineBuffer[44] = '\n';
	memset (baTemp, 0x00, sizeof(baTemp));		
	CTOS_PrinterBufferPutString((BYTE *)baTemp, 1, 1, ucLineBuffer, &stgFONT_ATTRIB);
	CTOS_PrinterBufferOutput((BYTE *)baTemp, 3);
	
	} 
	CTOS_PrinterFline(12 * 2); 
	
	
	return (ST_SUCCESS);
}


int get_env_int (char *tag)
{
	int     ret = -1;
	char    buf[6];

    memset (buf, 0, sizeof (buf));
    if ( inCTOSS_GetEnvDB (tag, buf) == d_OK )
    {
        ret = atoi (buf);
    }

	vdDebug_LogPrintf("get_env_int [%s]=[%d]", tag, ret);

    return ret;
}

int inCTOS_SelectFont(int inFontMode,int inFontSize ,int inFontStyle,char * szFontName)
{
	if(inFontMode == d_FONT_TTF_MODE)
	{
		CTOS_PrinterFontSelectMode(d_FONT_TTF_MODE);	//set the printer with TTF Mode
		CTOS_PrinterTTFSelect("times.ttf", inFontStyle);
	}
	else
	{
	
		CTOS_PrinterFontSelectMode(d_FONT_FNT_MODE);	//set the printer with default Mode
		CTOS_LanguagePrinterFontSize(inFontSize, 0, TRUE);		
	}
	return d_OK;
	
}

void vdSetGolbFontAttrib(USHORT FontSize, USHORT X_Zoom, USHORT Y_Zoom, USHORT X_Space, USHORT Y_Space)
{
    memset(&stgFONT_ATTRIB, 0x00, sizeof(stgFONT_ATTRIB));
    
    stgFONT_ATTRIB.FontSize = FontSize;      // Font Size = 12x24
	stgFONT_ATTRIB.X_Zoom = X_Zoom;		    // The width magnifies X_Zoom diameters
	stgFONT_ATTRIB.Y_Zoom = Y_Zoom;		    // The height magnifies Y_Zoom diameters

    stgFONT_ATTRIB.X_Space = X_Space;      // The width of the space between the font with next font
    stgFONT_ATTRIB.Y_Space = Y_Space;      // The Height of the space between the font with next font      
    
}


void put_env_int(char *tag, int value)
{
	int     ret = -1;
	char    buf[6];

    memset (buf, 0, sizeof (buf));
    //int2str (buf, value);
    sprintf(buf, "%d", value);
    ret = inCTOSS_PutEnvDB (tag, buf);

	vdDebug_LogPrintf("put_env_int [%s]=[%d] ret[%d]", tag, value, ret);
}

/*Save ECR Send/Receive data -- sidumili*/
int inSaveISOECRLog(BOOL fSendPacket, unsigned char *pucMessage, int inLen)
{
	char ucLineBuffer[80] = {0};
	unsigned char *pucBuff;
	int inBuffPtr = 0;
	FILE *ISOFile;
	char szBuf[100] = {0};
	int inRecCnt = 0;
	int inResult = 0;
	CTOS_RTC SetRTC;
	BYTE szCurrentTime[20] = {0};
	
	inTCTRead(1);
	if((inLen > 0) && (strTCT.fECRISOLogging == TRUE))
	{
		ISOFile = fopen(ECRISOLOG_FILE, "r+t");
		
		if(ISOFile == NULL)
		{
			ISOFile = fopen(ECRISOLOG_FILE, "w+t");
		}
		else
		{
			fseek(ISOFile, 0, SEEK_SET);
			memset(szBuf, 0, sizeof(szBuf));
			fgets(szBuf, 80, ISOFile);
			inRecCnt = atoi(szBuf);
			fclose(ISOFile);

			if(inRecCnt >= 1) // # of ecr trxn to be save
			{
				if((inResult = CTOS_FileDelete("ISO.LOG")) != d_OK)
				{
					//vdDisplayErrorMsg(1, 6, "Error deleting ISOLOG!");
					vdDebug_LogPrintf("Unable to delete ecr log");
				}  
				ISOFile = fopen(ECRISOLOG_FILE, "w+t");
				inRecCnt = 0;
			}
			else
				ISOFile = fopen(ECRISOLOG_FILE, "r+t");
		}
		
		fseek(ISOFile, 0, SEEK_SET);
		if(fSendPacket == TRUE)
			++inRecCnt;
		fprintf(ISOFile, "%d\n", inRecCnt);
		fclose(ISOFile);


		ISOFile = fopen(ECRISOLOG_FILE, "a+t");
		
	  memset(szCurrentTime, 0x00, sizeof(szCurrentTime));
	  CTOS_RTCGet(&SetRTC);
		sprintf(szCurrentTime, "%02d/%02d/%02d %02d:%02d\n", SetRTC.bMonth, SetRTC.bDay, SetRTC.bYear, SetRTC.bHour, SetRTC.bMinute);

		memset(szBuf, 0, sizeof(szBuf));
	
		if (fSendPacket)
		{
			fputs("Send ECR ISO Packet:\n", ISOFile);
			sprintf(szBuf, "%s\n", g_szAPName);
		}
		else
		{
			fputs("Receive ECR ISO Packet:\n", ISOFile);
			sprintf(szBuf, "%s\n", g_szAPName);
		}

		fputs(szCurrentTime, ISOFile);
		fputs(szBuf, ISOFile);
		fputs("-------------------------------------\n", ISOFile);

		pucBuff = pucMessage + inLen;
		while (pucBuff > pucMessage)
		{
			memset(ucLineBuffer,0x00, sizeof(ucLineBuffer));
			for (inBuffPtr = 0; (inBuffPtr < 32) && (pucBuff > pucMessage); inBuffPtr += 3)
			{
				sprintf(&ucLineBuffer[inBuffPtr], "%02X ", *pucMessage);
				pucMessage++;
			}
			ucLineBuffer[32] = '\n';
			fputs(ucLineBuffer, ISOFile);
		} 
		fputs("\n\n", ISOFile);

		fclose(ISOFile);
		
	}
}


void vdTrimSpaces(BYTE *pchString) {
    int inIndex;

    while (1) {
        inIndex = strlen(pchString);
        if (inIndex) {
            if (pchString[inIndex - 1] == 0x20) {
                pchString[inIndex - 1] = ((char) 0);
                continue;
            }
        }
        break;
    }
}

