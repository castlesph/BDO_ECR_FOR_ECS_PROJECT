
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>

#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>
#include <pthread.h>
#include <sys/shm.h>
#include <linux/errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "MultiAptrans.h"
#include "..\Includes\MultiAplib.h"

#include "..\Includes\Debug.h"
#include "..\Includes\Utils.h"


char g_szAPName[25];
int inIPC_KEY_SEND_Parent = 0;

extern USHORT usCTOSS_ShareECRInitialize(void);


int inMultiAP_RunIPCCmdTypes(char *Appname, int IPC_EVENT_ID, BYTE *inbuf, USHORT inlen, BYTE *outbuf, USHORT *outlen)
{
    int insendloop = 0;
    int status;
    int ipc_len;
    pid_t wpid; 
    BYTE processID[100];
    pid_t pid = -1;
    USHORT IPC_IN_LEN, IPC_OUT_LEN;
    BYTE IPC_IN_BUF[d_MAX_IPC_BUFFER], IPC_OUT_BUF[d_MAX_IPC_BUFFER];
    BYTE IPC_IN_BUF_STR[d_MAX_IPC_BUFFER], IPC_OUT_BUF_STR[d_MAX_IPC_BUFFER];
    char szAppname[100+1];
	BYTE tmpbuf[100];

    strcpy(szAppname, Appname);

    memset(processID,0x00,sizeof(processID));
    vdMultiAP_getPID(szAppname,processID);
    pid = atoi(processID);

    vdDebug_LogPrintf("inMultiAP_RunIPCCmdTypes =[%d][%s]",pid, szAppname);

#ifdef ECR_FORK_MAIN_APP   
    if (pid <= 0)
    {
        inCTOS_ReForkSubAP(szAppname);
        memset(processID,0x00,sizeof(processID));
        vdMultiAP_getPID(szAppname,processID);
        pid = atoi(processID);
        //here need delay, wait sub AP initialization 
        if (pid > 0)
            CTOS_Delay(3000);
        
        vdDebug_LogPrintf("inMultiAP_RunIPCCmdTypes22 =[%d][%s]",pid, szAppname);
    }
#endif

    if (pid > 0)
    {
//      inIPC_KEY_SEND_Parent = pid;
        IPC_IN_LEN = 0;
        memset(IPC_IN_BUF_STR,0x00,sizeof(IPC_IN_BUF_STR));
        memset(IPC_OUT_BUF_STR,0x00,sizeof(IPC_OUT_BUF_STR));
        
        IPC_IN_BUF[IPC_IN_LEN ++] = IPC_EVENT_ID;
        if (inlen > 0)
        {
            memcpy(&IPC_IN_BUF[IPC_IN_LEN], inbuf, inlen);
            IPC_IN_LEN = IPC_IN_LEN + inlen;
        }
        
        do
        {
            memset(IPC_IN_BUF_STR, 0x00, sizeof(IPC_IN_BUF_STR));
            inlen = wub_hex_2_str(IPC_IN_BUF ,IPC_IN_BUF_STR ,IPC_IN_LEN);
            IPC_OUT_LEN = inMultiAP_IPCCmdParent(IPC_IN_BUF_STR, IPC_OUT_BUF_STR, pid);
            wub_str_2_hex(IPC_OUT_BUF_STR, IPC_OUT_BUF, strlen(IPC_OUT_BUF_STR));
            
            //vdDebug_LogPrintf("CloseAllDrvHandle IPC_OUT_LEN=[%d][%2X][%d]",IPC_OUT_LEN, IPC_OUT_BUF[0],IPC_IN_LEN);
			memset(tmpbuf,0x00,sizeof(tmpbuf));
			sprintf(tmpbuf,"[%d]=[%d]",insendloop,IPC_OUT_BUF[0]);
			//CTOS_LCDTPrintXY(1, 5, "                                    ");
			//CTOS_LCDTPrintXY(1, 5, tmpbuf);
			vdDebug_LogPrintf("inMultiAP_IPCCmdParent insendloop=[%s]",tmpbuf);
            if (IPC_OUT_BUF[0] == d_SUCCESS)
            {
                break;
            }
#if 0
            insendloop++;
            if (insendloop > 2)
            {
                return d_NO;

             #ifdef ECR_FORK_MAIN_APP
                if (insendloop > 3)
                    return d_NO;
                
                inCTOS_ReForkSubAP(szAppname);
                memset(processID,0x00,sizeof(processID));
                vdMultiAP_getPID(szAppname,processID);
                pid = atoi(processID);
                vdDebug_LogPrintf("inMultiAP_RunIPCCmdTypes22 =[%d][%s]",pid, szAppname);
                //here need delay, wait sub AP initialization 
                if (pid > 0)
                    CTOS_Delay(3000);
                else
                    return d_NO;
            #endif
            }
#else
insendloop++;

//CTOS_Delay(3000);

#endif
        } while (1);

        memset(IPC_OUT_BUF_STR, 0x00, sizeof(IPC_OUT_BUF_STR));
        while ( (wpid=waitpid(pid, &status, WNOHANG)) != pid )
        {           
            memset(processID,0x00,sizeof(processID));
            vdMultiAP_getPID(szAppname,processID);
            if (atoi(processID) <= 0)
            {
            #ifdef ECR_FORK_MAIN_APP
                inCTOS_ReForkSubAP(szAppname);
            #endif
                return d_NO;
            }

            CTOS_Delay(1000);

            ipc_len = d_MAX_IPC_BUFFER;
            // patrick need hendle check status if transaction fail or success.
            #if 0
            if (inMultiAP_IPCGetParent(IPC_OUT_BUF_STR, &ipc_len) == d_OK)
            {
                ipc_len = wub_str_2_hex(IPC_OUT_BUF_STR, outbuf, ipc_len);
                *outlen = ipc_len;
                return d_OK;
            }
			#else
			if (inMultiAP_GetMainroutineEx(IPC_OUT_BUF_STR, &ipc_len) == d_OK)
			{
				ipc_len = wub_str_2_hex(IPC_OUT_BUF_STR, outbuf, ipc_len);
                *outlen = ipc_len;
                return d_OK;
			}
			#endif
        }
    }
    else
    {
        return d_NO;
    }
}


/*================================================================
  Handle IPC function.
  [Return] IPC process status return code
  ================================================================ */
USHORT inMultiAP_HandleIPC(BYTE *inbuf, USHORT inlen, BYTE *outbuf, USHORT *outlen)
{
	USHORT ushOutLen;
    USHORT usResult;
	int inRet = d_NO;

	ushOutLen = 0;
	
	/* Process IPC command code action */
	switch (inbuf[0])
	{
	case d_IPC_CMD_ECR_Initialize:
		memset(g_szAPName, 0x00, sizeof(g_szAPName));
		strcpy(g_szAPName, &inbuf[1]);
		
        usResult = usCTOSS_ShareECRInitialize();
		outbuf[ushOutLen++] = d_IPC_CMD_ECR_Initialize;
		if (usResult == d_OK)
			outbuf[ushOutLen++] = d_SUCCESS;
		else
			outbuf[ushOutLen++] = d_FAIL;
		outbuf[ushOutLen] = 0x00;
 		break;
	default:
		outbuf[ushOutLen++] = 0xFF;
		break;	
	}

	*outlen = ushOutLen;
	return ushOutLen;
}

int inMultiAP_SendChild(BYTE *inbuf, USHORT inlen)
{
	BYTE bret;
	BYTE outbuf[d_MAX_IPC_BUFFER];
	int out_len = 0;
	memset(outbuf,0x00,sizeof(outbuf));
	
	out_len = wub_hex_2_str(inbuf ,outbuf ,inlen);
	
	bret = inMultiAP_IPCSendChild(outbuf , out_len);
	//vdDebug_LogPrintf("inMultiAP_IPCSendChild=[%s] [%ld] [%d]",outbuf, out_len,inlen);
}

#if 0
int inMultiAP_GetMainroutine(void)
{
	BYTE ipc[d_MAX_IPC_BUFFER] , str[d_MAX_IPC_BUFFER];
	int ipc_len;
	BYTE bret;
	BYTE outbuf[d_MAX_IPC_BUFFER];
	USHORT out_len = 0;

	ipc_len = sizeof(ipc);
	memset(str,0x00,sizeof(str));
	memset(outbuf,0x00,sizeof(outbuf));

	if (inMultiAP_IPCGetChild(str, &ipc_len) == d_OK)
	{
		outbuf[out_len++] = d_SUCCESS;
		outbuf[out_len] = 0x00;
		inMultiAP_SendChild(outbuf,out_len);

		memset(outbuf,0x00,sizeof(outbuf));
		memset(ipc,0x00,sizeof(ipc));		
		ipc_len = wub_str_2_hex(str , ipc , ipc_len);
		ipc_len = inMultiAP_HandleIPC(ipc , ipc_len, outbuf, &out_len);  //Do IPC request
		if (out_len == 0)
		{
			out_len = 0;
			outbuf[out_len++] = d_FAIL;
			outbuf[out_len] = 0x00;
		}

		inMultiAP_SendChild(outbuf,out_len);
	}
}
#else
int inMultiAP_GetMainroutine(void)
{
	BYTE ipc[d_MAX_IPC_BUFFER] , str[d_MAX_IPC_BUFFER];
	int ipc_len;
	BYTE bret = d_NO;
	BYTE outbuf[d_MAX_IPC_BUFFER];
	USHORT out_len = 0;
	pid_t my_pid;
		
	ipc_len = sizeof(ipc);
	memset(str,0x00,sizeof(str));
	memset(outbuf,0x00,sizeof(outbuf));

	vdDebug_LogPrintf("inMultiAP_GetMainroutine");

	if (inMultiAP_IPCGetChildEx(str, &ipc_len) == d_OK)
	{
		outbuf[out_len++] = d_SUCCESS;
		outbuf[out_len] = 0x00;
		bret = inMultiAP_SendChild(outbuf,out_len);
		if (bret != d_OK)
			return d_OK;

		vdDebug_LogPrintf("bret 1 = %d", bret);

		memset(outbuf,0x00,sizeof(outbuf));
		memset(ipc,0x00,sizeof(ipc));
		out_len = 0;
		ipc_len = wub_str_2_hex(str , ipc , ipc_len);
		ipc_len = inMultiAP_HandleIPC(ipc , ipc_len, outbuf, &out_len);//Do IPC request

		if (out_len == 0)
		{
			out_len = 0;
			outbuf[out_len++] = d_FAIL;
			outbuf[out_len] = 0x00;
			
			vdDebug_LogPrintf("IPC fail");
		}

		
		bret = inMultiAP_SendChild(outbuf,out_len);
		
		vdDebug_LogPrintf("bret 2 = %d", bret);
//		bret = d_NO;
		if (bret != d_OK)
		{
		    
			vdDebug_LogPrintf("inMultiAP_GetMainroutine exit(1)");
			exit(1);
			/*
			my_pid = getpid();
			memset(outbuf,0x00,sizeof(outbuf));
			sprintf(outbuf,"kill -9 %d",my_pid);
			vdDebug_LogPrintf("kill,[%d][%s]",my_pid,outbuf);
			system(outbuf);
			*/
			return d_OK;
		}
	}
}

#endif

int inCTOS_MultiAPALLAppEventID(int IPC_EVENT_ID)
{
	int inResult = d_NO, inRetVal = d_NO;
	int inLoop = 0;
	CTOS_stCAPInfo stinfo;
	BYTE byStr[25];
	USHORT Outlen; 
    char szDataBuf[10];
    char szAPName[25];
	int inAPPID;

	for (inLoop = 0; d_MAX_APP > inLoop; inLoop++)
	{
		memset(&stinfo, 0x00, sizeof(CTOS_stCAPInfo));
		if (CTOS_APGet(inLoop, &stinfo) != d_OK)
		{
			vdDebug_LogPrintf("inLoop11[%d]", inLoop);
			memset(&stinfo, 0x00, sizeof(CTOS_stCAPInfo));
			CTOS_APGet(inLoop, &stinfo);
		}

		vdDebug_LogPrintf("bFlag[%2X]", stinfo.bFlag);
		vdDebug_LogPrintf("baName[%s][%2X] ", stinfo.baName, stinfo.baName[0]);

		inMultiAP_CurrentAPNamePID(szAPName, &inAPPID);

		if (strcmp(szAPName, stinfo.baName)!=0)
		{	
			if (stinfo.baName[0] !=0x00)
			{
				vdDebug_LogPrintf("[%ld]baName[%s] ", inLoop, stinfo.baName);
				if (memcmp(stinfo.baName, "SHARLS_", 7)==0)
					continue;

                memset(szDataBuf, 0x00, sizeof(szDataBuf));
				inRetVal = inMultiAP_RunIPCCmdTypes(stinfo.baName, IPC_EVENT_ID, szDataBuf, 0, byStr,&Outlen);    
				if ((byStr[1] == d_SUCCESS)||(byStr[1] == d_FAIL))
					return byStr[1];
				else
					continue;
			}
		}
	}
	return d_NOT_RECORD;
}

int inMultiAP_GetMainroutineEx(BYTE *str , int *len)
{
	BYTE bret = d_NO;
	BYTE outbuf[d_MAX_IPC_BUFFER];
	USHORT out_len = 0;

	memset(outbuf,0x00,sizeof(outbuf));

	if (inMultiAP_IPCGetChildEx(str, len) == d_OK)
	{
		outbuf[out_len++] = d_SUCCESS;
		outbuf[out_len] = 0x00;
		bret = inMultiAP_SendChild(outbuf,out_len);

		return d_OK;
	}
	return d_NO;
}


int inMultiAP_RunIPCCmdTypesEx(char *Appname, int IPC_EVENT_ID, BYTE *inbuf, USHORT inlen, BYTE *outbuf, USHORT *outlen)
{
    int insendloop = 0;
	int insendlooplimit = 0;
    int status;
    int ipc_len;
    pid_t wpid; 
    BYTE processID[100];
    pid_t pid = -1;
    USHORT IPC_IN_LEN, IPC_OUT_LEN;
    BYTE IPC_IN_BUF[d_MAX_IPC_BUFFER], IPC_OUT_BUF[d_MAX_IPC_BUFFER];
    BYTE IPC_IN_BUF_STR[d_MAX_IPC_BUFFER], IPC_OUT_BUF_STR[d_MAX_IPC_BUFFER];
    char szAppname[100+1];
	int inFristtime = 0;
	
	BYTE tmpbuf[100];

    strcpy(szAppname, Appname);

    memset(processID,0x00,sizeof(processID));
    vdMultiAP_getPID(szAppname,processID);
    pid = atoi(processID);

    vdDebug_LogPrintf("inMultiAP_RunIPCCmdTypesEX =[%d][%s]",pid, szAppname);

	if(IPC_EVENT_ID == d_IPC_CMD_SETTLEMENT_ALL)
		insendlooplimit = 100;
	else
		insendlooplimit = 2;
	
#ifdef ECR_FORK_MAIN_APP   
    if (pid <= 0)
    {
        inCTOS_ReForkSubAP(szAppname);
        memset(processID,0x00,sizeof(processID));
        vdMultiAP_getPID(szAppname,processID);
        pid = atoi(processID);
        //here need delay, wait sub AP initialization 
        if (pid > 0)
            CTOS_Delay(3000);
        
        vdDebug_LogPrintf("inMultiAP_RunIPCCmdTypes22 =[%d][%s]",pid, szAppname);
    }
#endif

    if (pid > 0)
    {
//      inIPC_KEY_SEND_Parent = pid;
        IPC_IN_LEN = 0;
        memset(IPC_IN_BUF_STR,0x00,sizeof(IPC_IN_BUF_STR));
        memset(IPC_OUT_BUF_STR,0x00,sizeof(IPC_OUT_BUF_STR));
        
        IPC_IN_BUF[IPC_IN_LEN ++] = IPC_EVENT_ID;
        if (inlen > 0)
        {
            memcpy(&IPC_IN_BUF[IPC_IN_LEN], inbuf, inlen);
            IPC_IN_LEN = IPC_IN_LEN + inlen;
        }
        
        do
        {
            memset(IPC_IN_BUF_STR, 0x00, sizeof(IPC_IN_BUF_STR));
            inlen = wub_hex_2_str(IPC_IN_BUF ,IPC_IN_BUF_STR ,IPC_IN_LEN);
            IPC_OUT_LEN = inMultiAP_IPCCmdParent(IPC_IN_BUF_STR, IPC_OUT_BUF_STR, pid);
            wub_str_2_hex(IPC_OUT_BUF_STR, IPC_OUT_BUF, strlen(IPC_OUT_BUF_STR));
            
            //vdDebug_LogPrintf("CloseAllDrvHandle IPC_OUT_LEN=[%d][%2X][%d]",IPC_OUT_LEN, IPC_OUT_BUF[0],IPC_IN_LEN);

			memset(tmpbuf,0x00,sizeof(tmpbuf));
			sprintf(tmpbuf,"[%d]=[%d]",insendloop,IPC_OUT_BUF[0]);
			//CTOS_LCDTPrintXY(1, 5, "                                    ");
			//CTOS_LCDTPrintXY(1, 5, tmpbuf);
			vdDebug_LogPrintf("inMultiAP_IPCCmdParent insendloop=[%s]",tmpbuf);

            if (IPC_OUT_BUF[0] == d_SUCCESS)
            {
                break;
            }

			//if (inFristtime == 0)
			//if (inCheckUSBorRS232RxReady() == d_OK)
			//{
			//	vdDebug_LogPrintf("MultiECRflag =[%d]",MultiECRflag);
			//	CTOS_KBDBufPut(d_KBD_CANCEL);
			//	MultiECRflag = 1;
			//	CTOS_Delay(500);
			//	return d_NO;
			//}
			
            insendloop++;
            if (insendloop > insendlooplimit)
            {
                return d_NO;

             #ifdef ECR_FORK_MAIN_APP
                if (insendloop > 3)
                    return d_NO;
                
                inCTOS_ReForkSubAP(szAppname);
                memset(processID,0x00,sizeof(processID));
                vdMultiAP_getPID(szAppname,processID);
                pid = atoi(processID);
                vdDebug_LogPrintf("inMultiAP_RunIPCCmdTypes22 =[%d][%s]",pid, szAppname);
                //here need delay, wait sub AP initialization 
                if (pid > 0)
                    CTOS_Delay(3000);
                else
                    return d_NO;
            #endif
            }
        } while (1);

		//vdDebug_LogPrintf("inWaitTime =[%d]",strTCT.inWaitTime);
		//if (strTCT.inWaitTime <= 0)
		//	strTCT.inWaitTime = 150;

        memset(IPC_OUT_BUF_STR, 0x00, sizeof(IPC_OUT_BUF_STR));
        while ( (wpid=waitpid(pid, &status, WNOHANG)) != pid )
        {           
            memset(processID,0x00,sizeof(processID));
            vdMultiAP_getPID(szAppname,processID);
            if (atoi(processID) <= 0)
            {
            #ifdef ECR_FORK_MAIN_APP
                inCTOS_ReForkSubAP(szAppname);
            #endif
                return d_NO;
            }

			//if (inCheckUSBorRS232RxReady() == d_OK)
			//{
			//	vdDebug_LogPrintf("MultiECRflag22 =[%d]",MultiECRflag);
			//	CTOS_KBDBufPut(d_KBD_CANCEL);
			//	MultiECRflag = 1;
			//	CTOS_Delay(1000);
			//	return d_NO;
			//}

            CTOS_Delay(150);

            ipc_len = d_MAX_IPC_BUFFER;
            // patrick need hendle check status if transaction fail or success.
            #if 0
            if (inMultiAP_IPCGetParent(IPC_OUT_BUF_STR, &ipc_len) == d_OK)
            {
                ipc_len = wub_str_2_hex(IPC_OUT_BUF_STR, outbuf, ipc_len);
                *outlen = ipc_len;
                return d_OK;
            }
			#else
			if (inMultiAP_GetMainroutineEx(IPC_OUT_BUF_STR, &ipc_len) == d_OK)
			{
				ipc_len = wub_str_2_hex(IPC_OUT_BUF_STR, outbuf, ipc_len);
                *outlen = ipc_len;
				vdDebug_LogPrintf("inMultiAP_GetMainroutineEx =[%d]",ipc_len);
                return d_OK;
			}
			#endif
        }
    }
    else
    {
        return d_NO;
    }
}



