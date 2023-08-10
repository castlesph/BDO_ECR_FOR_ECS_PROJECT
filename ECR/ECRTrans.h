#ifndef __ECR_TRANS_H__
#define	__ECR_TRANS_H__

#ifdef	__cplusplus
extern "C" {
#endif

#define ECR_SEPARATOR			0x1C
#define WATSONS_ECR_SEPARATOR	0x09

#define ECR_AMOUNT_TAG			"40"
#define ECR_TIP_AMOUNT_TAG		"41"

#define ECR_CASHBACK_TAG		"42"	
//#define ECR_INV_TAG					"02"
#define ECR_INV_TAG				"65" 
#define ECR_PAN_TAG				"66" 
#define ECR_EXPDATE_TAG			"67" 

#define ECR_SALECUP_TAG			"31" 
#define ECR_VOIDCUP_TAG			"32" 
#define ECR_REFUNDCUP_TAG		"33" 
#define ECR_VOIDRFNDCUP_TAG		"34" 
#define ECR_OFFLINECUP_TAG  	"35"
//#define ECR_PRECOMPCUP_TAG		"37"
//#define ECR_BATCH_RETCUP_TAG	"38"
#define ECR_PREAUTHCUP_TAG		"36"
#define ECR_CARDVER_TAG			"37"
#define ECR_COMPLETION_TAG		"38"
#define RETURN_LST_TXN			"39"
#define RETURN_BAT_TOT			"40"

#define ECR_SALE_TAG			"20"
#define ECR_INST_SALE_TAG		"22"
#define ECR_INST_VOID_TAG		"23"
#define ECR_VOID_TAG			"26"
#define ECR_REFUND_TAG		 	"27"
#define ECR_TIP_ADJ_TAG		 	"28"
#define ECR_CASHBACK_TXN_TAG	"36"
#define ECR_COMM_TEST_TAG		"D0"
#define ECR_CANCEL_TAG			"D1"

#define ECR_OCBC_IPP_TAG		"21" 
#define ECR_KIT_SALE_TAG		"11" 
#define ECR_RENEWAL_TAG			"12" 
#define ECR_PTS_AWARDING_TAG	"13" 
#define ECR_PIN_VERIFY          "ZZ"


#define ECR_PRNT_GRS_TAG		"57" 

#define ECR_PRNT_DATA_TAG		"5D" 
#define ECR_PRNT_DATA_NO_TAG	"5E" 
#define ECR_CARD_ENQUIRY_TAG	"6A" 
#define ECR_CARD_SHA1_TAG		"6B" 
#define ECR_PRNT_NO_TAG			"5C" 
#define ECR_PRNT_HOST_TAG		"5A" 
#define ECR_PRNT_INV_TAG		"5B" 

#define ECR_AUTH_CODE_TAG		"01"
#define ECR_RESP_TEXT_TAG		"02"
#define ECR_MERCHANT_NAME_TAG	"D0"
#define ECR_RESP_INV_TAG		"65"
#define ECR_TID_TAG				"16"
#define ECR_MID_TAG				"D1"
#define ECR_ISSUER_TAG			"D2"
#define ECR_CARD_NO_TAG			"30"
#define ECR_EXPIRY_TAG			"31"
#define ECR_BATCH_NO_TAG		"50"
#define ECR_DATE_TAG			"03"
#define ECR_TIME_TAG			"04"
#define ECR_RREF_TAG			"D3"
#define ECR_ISSUERID_TAG		"D4"
#define ECR_CARD_NAME_TAG		"D5"
//wubin 090826 start
#define	ECR_AID_TAG				"9A"
#define	ECR_AP_TAG				"9B"
#define	ECR_CRY_INFO_TAG		"9C"
#define	ECR_APP_CRY_TAG			"9D"
#define	ECR_TVR_TAG				"9E"
//wubin 090826 end

#define	ECR_ENTRY_CODE_TAG		"7C"
#define	ECR_QPS_STT_TAG			"7D"

#define	ECR_PRNT_TEXT1_TAG		"7E"
#define	ECR_PRNT_TEXT2_TAG		"7F"
#define	ECR_PRNT_TEXT3_TAG		"7G"
#define	ECR_PRNT_TEXT4_TAG		"7H"

#define	ECR_TRAC_AMOUT_TAG		"40"
#define	ECR_CURRENCY_TAG		"2A"

#define	ECR_GRS_CODE_TAG		"8A"
#define	ECR_GRS_ORGAMT_TAG		"8B"
#define	ECR_GRS_REDAMT_TAG		"8C"
#define	ECR_GRS_MERAMT_TAG		"8D"
#define	ECR_GRS_DISCOUNT_TAG	"8H"

#define	ECR_DCC_CURRENCY_TAG	"8E"
#define	ECR_DCC_CONVRATE_TAG	"8F"
#define	ECR_DCC_CONVAMT_TAG		"8G"

#define	ECR_CARD_SEQ_NO_TAG		"S3"
#define	ECR_ENTRY_MODE_TAG		"S4"

#define ECR_SMAC_NEW_DATE_TAG	"T1"
#define ECR_SETTLEMENT_ALL_TAG	"50"
#define ECR_REPRINT_TAG			"71"

#define ECR_INST_AMT_TAG		"GA"
#define ECR_INST_TERMS_TAG		"GB"
#define ECR_INST_PROMO			"GC"
#define ECR_INST_MO_AMT			"GD"


#define ECR_PRNT_TEXT1_SIZE		40
#define ECR_PRNT_TEXT2_SIZE		40
#define ECR_PRNT_TEXT3_SIZE		40
#define ECR_PRNT_TEXT4_SIZE		40


#define ECR_APPROVED			"00"
#define ECR_DECLINED_ERR		"ND"
#define ECR_REFERAL_ERR			"01"
#define ECR_TIMEOUT_ERR			"TO"
#define ECR_CANNOT_ERR			"ED"
#define ECR_COMMS_ERR			"EN"
#define ECR_UNKNOWN_ERR			"NA"

#define ECR_REMOVE_RESP			"TRANSACTION NOT SUCCESS                 "
#define ECR_APPROVED_RESP		"APPROVED                                "

#define ECR_OPER_CANCEL_RESP    "OPERATOR CANCEL                         "
#define ECR_TIMEOUT_RESP    	"TIMEOUT                                 "
#define ECR_COMM_ERROR_RESP    	"COMM ERROR                              "


#define TRANSPORT_HEADER_SIZE	2
#define TRANSPORT_DEST_SIZE		4
#define TRANSPORT_SRC_SIZE		4

#define FORMAT_VERSION_SIZE		1
#define REQ_RESP_SIZE			1
#define TXN_CODE_SIZE			2
#define MORE_FOLLOW_SIZE		1
#define END_PRESENT_SIZE		1
#define ECR_ENTRY_CODE_SIZE		3
#define ECR_QPS_STT_SIZE		1


#define AMOUNT_VALUE_SIZE		12
#define CASHBACK_VALUE_SIZE		12
#define INVOICE_NUMBER_SIZE		6
#define TRACE_NUMBER_SIZE       6
#define MID_SIZE				15
#define ISSUER_SIZE				10
#define ISSUERID_SIZE			2
#define TAG_SIZE				2
#define LENGTH_SIZE				2
#define CARD_NAME_SIZE			26
#define ECR_RESP_CODE_SIZE		2
#define ECR_RESP_TEXT_SIZE		40
#define ECR_MERCHANT_NAME_SIZE	69

#define ECR_2GENAC_DECLINED    -99

#define DATE_SIZE              	6            /* Date in MMDDYY format        */
#define TIME_SIZE              	6            /* Time in HHMMSS format        */
#define RET_REF_SIZE            12       /* Retrieval reference num size */
#define EXP_DATE_SIZE          	4            /* Expiry date MMYY             */
#define AUTH_CODE_SIZE          6        /* Authorization code size      */
#define TID_SIZE                8        /* Terminal Identification size */
#define BATCH_NUM_SIZE     		6              /* current largest batch number possible          */
#define OPERATOR_CANCEL_SIZE    16

#define GEN_VER_SIZE          	16           /* information in config files   */
#define CARD_SEQ_NO_SIZE		2
#define CARD_ENTRY_MODE_SIZE	2


#define CARD_ENTRY_MSR          1
#define CARD_ENTRY_MANUAL       2
#define CARD_ENTRY_ICC          3
#define CARD_ENTRY_WAVE         5

#define VS_CONTINUE				99

#define MIN_RESPONSE_CODE       0
#define MAX_RESPONSE_CODE       99

typedef struct tagECR_REQ
{
	char req_resp[REQ_RESP_SIZE + 1];
	char txn_code[TXN_CODE_SIZE + 1];
	char amount[AMOUNT_VALUE_SIZE + 1];
	char cashback[CASHBACK_VALUE_SIZE + 1];
	char inv_no[INVOICE_NUMBER_SIZE + 1];
	char date[DATE_SIZE + 1];
	char rref[RET_REF_SIZE+ 1];
	char Pan[PAN_SIZE + 1];
	char Expiry[EXP_DATE_SIZE+ 1];
	char prnt_no[3+ 1];
	char auth_code[AUTH_CODE_SIZE + 1];
	char tip_amount[AMOUNT_VALUE_SIZE + 1];
	char inst_terms[2 + 1];
	char inst_promo[2 + 1];
} ECR_REQ;

typedef struct tagECR_RESP
{
	char resp_code[ECR_RESP_CODE_SIZE + 1];
	char resp_text[ECR_RESP_TEXT_SIZE + 1];
	char merchant_name[ECR_MERCHANT_NAME_SIZE + 1];
	char auth_code[AUTH_CODE_SIZE  + 1];
	char inv_no[INVOICE_NUMBER_SIZE + 1];
	char tid[TID_SIZE + 1];
	char mid[MID_SIZE + 1];
	char issuer[ISSUER_SIZE + 1];
	char card_no[PAN_SIZE + 1];
	char exp_date[EXP_DATE_SIZE + 1];
	char batch_no[BATCH_NUM_SIZE + 1];
	char date[DATE_SIZE + 1];
    char time[TIME_SIZE + 1];
    char rref[RET_REF_SIZE+ 1];
    char card_name[CARD_NAME_SIZE + 1];
	char entry_code[ECR_ENTRY_CODE_SIZE + 1];
	char qps_stt[ECR_QPS_STT_SIZE + 1];
	char prnt_text1[ECR_PRNT_TEXT1_SIZE + 1];
	char prnt_text2[ECR_PRNT_TEXT2_SIZE + 1];
	char prnt_text3[ECR_PRNT_TEXT3_SIZE + 1];
	char prnt_text4[ECR_PRNT_TEXT4_SIZE + 1];
	char Trac_amout[12+1];
	char Currency[3+1];
	char GRS_code[16+1];
	char GRS_orgAmt[12+1];
	char GRS_RedAmt[12+1];
	char GRS_MerAmt[12+1];
	char GRS_DiscAmt[12+1];
	char DCC_Currency[3+1];
	char DCC_ConvRate[13+1];
	char DCC_ConvAmt[10+1];	
	char szSMACPay_CardSeqNo[2+1];
	char inst_terms[2 + 1];
	char inst_promo[2 + 1];
	char trace_no[TRACE_NUMBER_SIZE+1];
} ECR_RESP;

/*
typedef struct tagPAYMT_REQ
{
	unsigned char    	txnCode[2+1];
	unsigned  char 		txnAmount[12+1];
	signed char         issName[10+1];
	signed char         numInvoice[6+1];
	unsigned short  	crdLen;
	unsigned char    	crdPAN[19+1];
	unsigned char    	numBatch[6+1]; 
	unsigned char    	szRREF[12+1];
	unsigned char    	szDate[6+1];
	unsigned char    	rfu[10+1];
	unsigned char   	szOutEcrBuf1[20+1];
	unsigned char   	szOutEcrBuf2[20+1];
	unsigned char   	szOutEcrBuf3[20+1];
}PAYMT_REQ;

typedef struct tagPAYMT_RESP
{
	unsigned char    	txnCode[2+1];
	unsigned  char 		szDispRespText[25+1];
	signed char         ApprovalCode[6+1];
	unsigned char  		szRespCode[2+1];   
	signed char         numInvoice[6+1];
	unsigned char    	txnAmount[12+1];
	unsigned char  		OperatorID[2+1];
	unsigned char 		szTID[8+1];
	unsigned char 		szMID[15+1];
	unsigned char 		szPAN[19+1];
	unsigned short 		shPANlen;
	unsigned char 		szExpDate[6+1];
	unsigned char 		szTxnDate[6+1];
	unsigned char 		szTxnTime[6+1];
	unsigned char 		szBatchNum[6+1];
	unsigned char 		CHName[26+1];
	unsigned char 		issName[26+1];
	unsigned char    	szRREF[12+1];
	unsigned char    	szCardIssID[2+1];
	unsigned short shSalesCnt;
	double dbSaleTotal;
	unsigned short shVoidCnt;
	double dbVoidTotal;
	unsigned short shRefundCnt;
	double dbRefundTotal;
	unsigned short shVoidRefundCnt;
	double dbVoidRefundTotal;
	unsigned char szEMVAID[33+1];
	unsigned char szEMVTC[16+1];
	unsigned char szEMVTVR[10+1];
	unsigned char szEMVTSI[10+1];
	unsigned char szEMVAppName[15+1];
	unsigned char   szEMVCRY [2+1];
	unsigned char   szInEcrBuf1[20+1];
	unsigned char   szInEcrBuf2[20+1];
	unsigned char   szInEcrBuf3[20+1];
	unsigned char   szInEcrBuf4[20+1];
	unsigned char   szInEcrBuf5[20+1];
}PAYMT_RESP;
*/
	
ECR_REQ ECRReq;
ECR_RESP ECRResp;
char chECRFlag;

int inPrintECRPacket(char *szTitle, unsigned char *pucMessage, int inLen);
int inValidatePacket(char* pucPkt,int inSize);
int inECRReceiveAnalyse(void);
int inECRSendAnalyse(void);
int inECRSendResponse(void);
USHORT usCTOSS_ShareECRInitialize(void);
int inCTOSS_CheckECREvent(void);
int get_env_int (char *tag);
int inCTOS_SelectFont(int inFontMode,int inFontSize ,int inFontStyle,char * szFontName);
void vdSetGolbFontAttrib(USHORT FontSize, USHORT X_Zoom, USHORT Y_Zoom, USHORT X_Space, USHORT Y_Space);

void put_env_int(char *tag, int value);
int inSaveISOECRLog(BOOL fSendPacket, unsigned char *pucMessage, int inLen); /*Save ECR Send/Receive data -- sidumili*/
void vdTrimSpaces(BYTE *pchString);

#ifdef	__cplusplus
}
#endif

#endif	/* __ECR_TRANS_H__ */

