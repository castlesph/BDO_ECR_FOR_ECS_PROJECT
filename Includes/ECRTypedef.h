
#ifndef __ECR_TYPEDEF_H__
#define __ECR_TYPEDEF_H__

#ifdef    __cplusplus
extern "C" {
#endif

#define PUBLIC_PATH 	"/home/ap/pub/"
#define IN
#define OUT
#define INOUT

#define char    		char
#define UCHAR   		unsigned char
#define BYTE    		unsigned char

#ifndef STR
    #define STR        	unsigned char    
#endif

#define SHORT   		short                // 2byte
#define USHORT  		unsigned short       // 2byte
#define WORD    		unsigned short       // 2byte
#define int     		int                  // 4byte    
#define UINT    		unsigned int         // 4byte
#define DWORD   		unsigned int         // 4byte
#define BOOL    		unsigned char        // 1byte
#define LONG    		long                 // 4byte
#define ULONG   		unsigned long        // 4byte

#define TRUE     1
#define FALSE    0

#ifndef NULL
#define NULL    0
#endif

#ifndef CTOSNULL
#define CTOSNULL    0
#endif

typedef unsigned char VS_BOOL;

#define K_STATUS        BYTE

#define SUCCESS     0
#define FAIL       -1

#define VS_FALSE           ((VS_BOOL) 0)
#define VS_TRUE            ((VS_BOOL) 1)

#define DF_BATCH_APPEND 0
#define DF_BATCH_UPDATE 1


#define TPDU_ASC_SIZE              10
#define TPDU_BCD_SIZE           ((TPDU_ASC_SIZE+1)/2)

#define PRO_CODE_ASC_SIZE       6
#define PRO_CODE_BCD_SIZE       ((PRO_CODE_ASC_SIZE+1)/2)

#define MTI_ASC_SIZE            4
#define MTI_BCD_SIZE            ((MTI_ASC_SIZE+1)/2)

#define INVOICE_ASC_SIZE        6
#define INVOICE_BCD_SIZE        ((INVOICE_ASC_SIZE+1)/2)

#define DATE_ASC_SIZE                     4
#define DATE_BCD_SIZE                      ((DATE_ASC_SIZE+1)/2)

#define TIME_ASC_SIZE                     6
#define TIME_BCD_SIZE                      ((TIME_ASC_SIZE+1)/2)

#define EXPIRY_DATE_ASC_SIZE              4
#define EXPIRY_DATE_BCD_SIZE               ((EXPIRY_DATE_ASC_SIZE+1)/2)

#define AMT_ASC_SIZE                      12/* BCD - includes cents */
#define AMT_BCD_SIZE                       ((AMT_ASC_SIZE+1)/2)

#define STORE_ID_DIGITS            18
#define TRACK_I_BYTES              85
#define TRACK_II_BYTES             41
#define TRACK_III_BYTES            64
#define CVV2_BYTES                 4
#define CARD_HOLD_NAME_DIGITS      30
#define AUTH_CODE_DIGITS           6
#define PAN_SIZE                   19
#define RRN_BYTES                  12
#define RESP_CODE_SIZE             2
#define TERMINAL_ID_BYTES          8
#define MERCHANT_ID_BYTES          15
#define CHIP_DATA_LEN              1024
#define ADD_DATA_LEN             1024


#define TRACE_NO_ASC_SIZE   6
#define TRACE_NO_BCD_SIZE   ((TRACE_NO_ASC_SIZE+1)/2)

#define BATCH_NO_ASC_SIZE   6
#define BATCH_NO_BCD_SIZE   ((BATCH_NO_ASC_SIZE+1)/2)

typedef struct
{
    BYTE byCardTypeNum;
    BYTE byEMVTransStatus;
    BYTE T5A_len;    // PAN_len
    BYTE T5A[10];    // PAN    
    BYTE T5F2A[2];    // Currency Code
    BYTE T5F30[3];        // Service code.
    BYTE T5F34;        // PAN SEQ No.
    BYTE T5F34_len;        // PAN SEQ No len.
    BYTE T82[2];        // Application Interchange Profile (AIP)
    BYTE T84_len;    // AID Len
    BYTE T84[16];    // AID
    BYTE T8A[2];
    BYTE T91[16];    // Issuer Application data (IAD)
    BYTE T91Len;     //IAD length
    BYTE T95[5];        // Terminal Verification Results
    BYTE T9A[3];        // Transaction Date
    BYTE T9C;        // Trans Type
    BYTE T9F02[6];    // Amount, Authorised (Numeric)
    BYTE T9F03[6];    // Amount, others (Numeric)
    BYTE T9F09[2];    // ICC Application Version No.
    BYTE T9F10_len;
    BYTE T9F10[32];    // Issuer Application data
    BYTE T9F1A[2];    // Terminal Country Code
    BYTE T9F26[8];    // Application Cryptogram (AC)
    BYTE T9F27;        // Cryptogram Information Data (CID)
    BYTE T9F33[3];    // Terminal Capabilities
    BYTE T9F34[3];    // Cardholder Verification Method (CVM) Results
    BYTE T9F35;        // Terminal Type
    BYTE T9F36_len;
    BYTE T9F36[2];    // Application Transaction Counter (ATC)
    BYTE T9F37[4];    // Unpredictable Number, (random number)
    BYTE T9F41[3];    // Original Trace No (Field 11)
    BYTE T9F53;        // TCC,
    BYTE ISR[16];        // Issuer Script Result.
    BYTE ISRLen;        // Issuer Script Result.
    BYTE T9B[2];        // Transaction Status Information
    BYTE T5F24[3];    // Application Expiration Date(YYMMDD)
      
    BYTE T71Len;    // Application Interchange Profile (AIP)
    BYTE T71[258];    // Application Interchange Profile (AIP)
    BYTE T72Len;    // Application Interchange Profile (AIP)
    BYTE T72[258];    // Application Interchange Profile (AIP)
    BYTE T9F06[3];  // AID
    BYTE T9F1E[8];  // terminal hardware serial number
    BYTE T9F28[8];  // ARQC
    BYTE T9F29[8];  // TC
    BYTE szChipLabel[32];  // AP label    
}emvinfo_t;

typedef struct
{
    BYTE        byTransType;
    BYTE        byPanLen;
    BYTE        szExpireDate[EXPIRY_DATE_BCD_SIZE+1];    /* BCD YYMM        */
    BYTE        byEntryMode;
    BYTE        szTotalAmount[AMT_BCD_SIZE+1];   /* BCD total   amount    */
    BYTE        szBaseAmount[AMT_BCD_SIZE+1];
    BYTE        szTipAmount[AMT_BCD_SIZE+1];
    BYTE        szTID[TERMINAL_ID_BYTES+1];
    BYTE        szMID[MERCHANT_ID_BYTES+1];
    BYTE        szHostLabel[16];
    BYTE        szBatchNo[BATCH_NO_BCD_SIZE+1];
    BYTE        byOrgTransType ;
    BYTE        szMacBlock[8] ;
    BYTE        szYear[2];                 /* int 1997=97,2003=103    */
    BYTE        szDate[DATE_BCD_SIZE+1];     /* BCD MMDD        */
    BYTE        szTime[TIME_BCD_SIZE+1];     /* BCD hhmmss        */
    BYTE        szOrgDate[DATE_BCD_SIZE+1];     /* BCD MMDD        */
    BYTE        szOrgTime[TIME_BCD_SIZE+1];     /* BCD hhmmss        */
    BYTE        szAuthCode[AUTH_CODE_DIGITS+1];
    BYTE        szRRN[RRN_BYTES+1];
    BYTE        szInvoiceNo[INVOICE_BCD_SIZE+1];
    BYTE        szOrgInvoiceNo[INVOICE_BCD_SIZE+1];
    BYTE        byPrintType;
    BYTE        byVoided;
    BYTE        byAdjusted;
    BYTE        byUploaded;
    BYTE        byTCuploaded;    /* For upload TC*/
    BYTE        szCardholderName[CARD_HOLD_NAME_DIGITS+1];
    BYTE        szzAMEX4DBC[4+1];
    BYTE        szStoreID[STORE_ID_DIGITS+1];
    BYTE        szRespCode[RESP_CODE_SIZE+1];
    BYTE        szServiceCode[3+1];
    BYTE        byContinueTrans;
    BYTE        byOffline;
    BYTE        byReversal;
    BYTE        byEMVFallBack;
    SHORT       shTransResult;
    BYTE        szTpdu[TPDU_ASC_SIZE+1];
    BYTE        szIsoField03[PRO_CODE_BCD_SIZE+1];
    BYTE        szMassageType[MTI_BCD_SIZE+1];
    BYTE        szPAN[PAN_SIZE+1];
    BYTE        szCardLable[20+1];
    USHORT      usTrack1Len;
    USHORT      usTrack2Len;
    USHORT      usTrack3Len;
    BYTE        szTrack1Data[TRACK_I_BYTES+1];    
    BYTE        szTrack2Data[TRACK_II_BYTES+1];
    BYTE        szTrack3Data[TRACK_III_BYTES+1];
	USHORT      usChipDataLen;		//Chip Data
	BYTE        baChipData[CHIP_DATA_LEN+1];
	USHORT      usAdditionalDataLen;	//Additional Data
	BYTE        baAdditionalData[ADD_DATA_LEN+1];
	BYTE		bWaveSID;
	USHORT		usWaveSTransResult;	// Transction Result
	BYTE		bWaveSCVMAnalysis;
    ULONG       ulTraceNum; 
    ULONG       ulOrgTraceNum;
    USHORT      usTerminalCommunicationMode;
    BYTE        IITid;
    BYTE        HDTid; //kobe added
    ULONG       ulSavedIndex;// kobea added  
    BYTE        byPINEntryCapability;
    BYTE        byPackType;
	int  		inCardType;
	UINT 		MITid;
    BYTE        szOrgAmount[AMT_BCD_SIZE+1];
    BYTE        szCVV2[CVV2_BYTES+1];
    BYTE        byTCFailUpCnt;//20121204
    BYTE		szPINBlock[8+1];
    SHORT  		CDTid;
    emvinfo_t   stEMVinfo;
	
	/* Issue# 000096: BIN VER Checking - start -- jzg*/
	BOOL fBINVer;
	BYTE szBINVerSTAN[8];
	/* Issue# 000096: BIN VER Checking - end -- jzg*/

	BYTE szECRPANFormatted[PAN_SIZE+1];
	BYTE szECRRespText[40 + 1];
	BYTE szECRMerchantName[69 + 1];
	BYTE szECRRespCode[2 + 1];
	BYTE szPOSTID[10];
	BYTE bySMACPay_CardSeqNo[2+1];
	int PRMid;
	BYTE szTerms[3];
	
}TRANS_DATA_TABLE;

TRANS_DATA_TABLE srTransRec;

#define RGB(r,g,b) (unsigned long)( ((DWORD)(BYTE)r&0xff)|((DWORD)((BYTE)g&0xff)<<8)|((DWORD)((BYTE)b&0xff)<<16) )

//PAYMT_RESP PaymntResp;

#define VS_SUCCESS            0             /* General purpose error code */
#define VS_ERR              (-1)
#define VS_ERROR            (-1)            /* Operator error */
#define VS_FAILURE          (-2)            /* System-level error */
#define VS_ESCAPE           (-3)            /* Operator quit transaction */
#define VS_TIMEOUT          (-4)            /* Time out */
#define VS_CARRIER          (-5)            /* Modem Lost Carrier */
#define VS_EOB              (-6)            /* End of Batch */
#define VS_ABORT            (-99)           /* Operation Aborted (obsolete) */

#define VS_FALSE           ((VS_BOOL) 0)
#define VS_TRUE            ((VS_BOOL) 1)

/* Charatcers */
#define NUL 		0x00
#define SOH 		0x01
#define STX 		0x02
#define ETX 		0x03
#define EOT 		0x04
#define ENQ 		0x05
#define ACK 		0x06
#define BEL 		0x07
#define BS 			0x08
#define DLE 		0x10
#define DC1 		0x11
#define NAK 		0x15
#define SYN 		0x16
#define ETB 		0x17
#define CAN 		0x18
#define ESC 		0x1B
#define FS 			0x1C
#define US 			0x1F
#define SPACE 		0x20
#define FORMFEED 	0x0C
#define CR 			0x0D
#define LF 			0x0A
#define ANY         0xFF
#define PAD 		0x0FF

/* Definitions for various error types */

#define ERR_COM_OPEN                    (-10) /* port open failed                   */
#define ERR_COM_INIT                    (-11) /* Error setting port parameters      */
#define ERR_COM_CLOSE                   (-12) /* port close failed                  */
#define ERR_COM_SEND                    (-13) /* Write to Com port failed           */
#define ERR_COM_NAK                     (-14) /* Did not receive ACK for last write */
#define ERR_COM_READ                    (-15) /* Error setting port parameters      */
#define ERR_COM_WRITE                   (-16) /* Error setting port parameters      */
#define ERR_COM_TIMEOUT                 (-17) /* Error setting port parameters      */
#define ERR_COM_RECV                    (-18) /* Error setting port parameters      */

#define     BRANCH_EXIT 999

/* Results of the state */
#define ST_SUCCESS          0
#define ST_ERROR            (-1)

#define d_DEBUG_PORT d_COM1
#define d_READY_TIMEOUT		100

/* <<< end*/

//#define _TRANS_CONNECT_USE_RS232_

#ifdef    __cplusplus
}
#endif

#endif    /* __ECR_TYPEDEF_H__ */

