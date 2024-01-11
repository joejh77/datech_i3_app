/**************************************************************************************************
 *
 *      File Name       :  system_log.h
 *      Description     :  Blackbox Log function
 *
 *      Creator         :   tony ( DATECH Co., Ltd )
 *      Create Date     :   2019/12/29
 *      Update History  :   
 *
 *************************************************************************************************/
#ifndef __SYSTEM_LOG_H__
#define	__SYSTEM_LOG_H__

#define AK_LOG_SAVE_USE 1

extern void Log_Print(const char * fmt, ...);

#define	MAX_LOG_LABEE			37
#define	MAX_LOG_STRING			25


 /*************************************************
  * log definitions
  *************************************************/
#define 	LOG_TASK_EXIT					0

#define		LOG_SYSBOOT						1001
#define		LOG_SYSEND						1002
	 
#define		LOG_EVENT_DATETIMECHANGE		2001
		 /* sub type */
	#define		_LOG_DTCHANGE_ST_SUCCESS	0
	#define		_LOG_DTCHANGE_ST_ERROR		1
 
#define		LOG_ERRDBOVERTIME				2002
 
#define		LOG_EVENT_REC_BEGIN				2101
#define		LOG_EVENT_REC_END				2102
	 
#define		LOG_EVENT_BACKUP				2150
		 /* sub type */
	#define		_LOG_BACKUP_ST_BEGIN	0
	#define		_LOG_BACKUP_ST_END		1
		 /* data type */
	#define		__LOG_BACKUP_DT_SUCCEED	0
	#define		__LOG_BACKUP_DT_USB		0
	#define		__LOG_BACKUP_DT_JPEG	0
	#define		__LOG_BACKUP_DT_DB		1
		 /* data value */
				 /* union data : byte[0] -> error code	( __LOG_BACKUP_DT_SUCCEED or Error codes ) */
				 /* union data : byte[1] -> media type	( __LOG_BACKUP_DT_USB ) */
				 /* union data : byte[2] -> backup type ( __LOG_BACKUP_DT_JPEG or __LOG_BACKUP_DT_DB ) */
 
#define		LOG_EVENT_LOGINOUT				2151
		 /* sub type */
	#define		_LOG_LOGIN				0
	#define		_LOG_LOGOUT				1
		 /* data type */
		 /* data value */
				 /* union data : byte -> account name */
 
#define		LOG_EVENT_MEMORY_FORMAT			2152
				 /* union data : byte -> target name(ex:nand, sd) */
	 
#define		LOG_EVENT_BEGIN_SLEEP			2200
 
#define		LOG_EVENT_ACC_STATE				2201
		 /* data value */
				 /* union data : byte -> xx.xV(battery votage string) */
#define		LOG_EVENT_MP4_FILE_RECOVERY		2300
		 
#define 	LOG_DISCHARGE_SLEEP				2400
 
#define 	LOG_EVENT_MEMORY_INFO				2500
#define 	LOG_EVENT_SD_CD					2501
#define 	LOG_EVENT_USB_CD				2502

#define		LOG_EVENT_BATTCHANGE			2600
		 /* sub type = BLACKBOX_BATT_STATE */
	 
		 /* data value */
				 /* union data : byte -> xx.xV(battery votage string) */
 
#define		LOG_EVENT_EVENT_RECORDING		2700
		 /* sub type */
		 //BLACKBOX_EVENT_TYPE
		 
#define		LOG_EVENT_GSENSOR_ERROR			2702
#define 	LOG_EVENT_BUTTON_POWER			2703	// power off
 
#define		LOG_EVENT_UPDATE				2800
		 /* sub type */
	#define _LOG_FIRMWARE_FILE				0
	#define _LOG_FIRMWARE_USB				1
	#define _LOG_USERDATA_FILE				10
	#define _LOG_USERDATA_USB				11
 
	 //// error message
#define 	LOG_ERROR_BASE_NO				5000
#define		LOG_ERROR_RTC_READ_FAIL			(LOG_ERROR_BASE_NO + 0)
#define 	LOG_ERROR_SD_DISK_INIT_FAIL		(LOG_ERROR_BASE_NO + 1)
#define 	LOG_ERROR_USB_DISK_INIT_FAIL	(LOG_ERROR_BASE_NO + 2)
#define 	LOG_ERROR_MEDIA_ERROR			(LOG_ERROR_BASE_NO + 3)
#define 	LOG_ERROR_MEDIA_WARNING			(LOG_ERROR_BASE_NO + 4)
#define		LOG_ERROR_INTERNAL_ERR			(LOG_ERROR_BASE_NO + 5)

 /*************************************************
  * log structures
  *************************************************/
 #define LOG_ITEM_DATA_SIZE		24 //ViewerSystemLogWnd.cpp check
 typedef struct tagTr1LogItem {
	 time_t 	 time;		 /* log time */
	 u16		 type;		 /* log type */
	 u16		 type_sub;	 /* sub log type */
	 union
	 {
		 unsigned char		 byte[LOG_ITEM_DATA_SIZE];
		 u16	 word[LOG_ITEM_DATA_SIZE/2];
		 u32	 dword[LOG_ITEM_DATA_SIZE/4];
	 } data;
 } ST_LOG_ITEM, * LPST_LOG_ITEM;


#if AK_LOG_SAVE_USE
	 /*************************************************
	  * log functions
	  *************************************************/
	  	 extern  void 	 ak_log_TaskStart (message_t* msg_ID);
		 extern  void 	 ak_log_TaskStop(void);
		 extern  void	 ak_log_init( BOOL isForce );
		 extern  void	 ak_log_release( BOOL diskBackup );
		 extern  BOOL	 ak_log_add(LPST_LOG_ITEM pitem);
		 extern  BOOL	 ak_log_del(u32 count);
		 extern  u32	 ak_log_get_itemcount( void );
		 extern  u32	 ak_log_get_item(u32 pos, u32 count, LPST_LOG_ITEM pitemarray);
		 extern  BOOL	 ak_log_get_item_type_string(char *szbuff, ST_LOG_ITEM read_logitem);
	 
		 ////////////////////// TEST FUNCTION ///////////////
		 extern void ak_log_test_main(void);
#endif // #if AK_LOG_SAVE_USE	 
	extern  BOOL	ak_log_get_download_flag(void);
 	extern  BOOL 	ak_log_get_system_run_flag(void);
	extern 	BOOL 	ak_log_get_format_run_flag(void);
	extern  BOOL	ak_log_add_ex(u16 type, u16 type_sub, char *fmt, ...);
	extern  int 	BboxAppPowerOnLogInit(unsigned short * drive);
	extern  int 	BboxAppPowerOffLogInit(unsigned short * drive);
#endif	/* end of the __SYSTEM_LOG_H__ */

