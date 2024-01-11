/**************************************************************************************************
 *
 *      File Name       :  system_log.cpp
 *      Description     :  Blackbox Log function
 *
 *      Creator         :   tony ( DATECH Co., Ltd )
 *      Create Date     :   2019/12/29
 *      Update History  :   
 *
 *************************************************************************************************/

// STDC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#define LOG_TEST



/**********************************************************
 * log local definitions
 **********************************************************/
#define 	AKLOG_ERROR			1
#define 	AKLOG_INIT			1
#define 	AKLOG_FUNC			0
#define 	AKLOG_ENTRY			1

#define		LOG_VERSION			000

#define		POS_LOG_HDR			(0)
#define		POS_LOG_START		(POS_LOG_HDR + l_spi_log.LogHdrSize) // (POS_LOG_HDR + sizeof(ST_LOG_INFOHDR))

#define LOG_DATA_FILE_SIZE						(l_spi_log.LogMaxSize)//( 256 * 1024)


#define AK_LOG_DATA_FILE_NAME					(L"system.log")



/**********************************************************
 * log local structures & variables
 **********************************************************/

	typedef struct tagLogInformationHeader // 32 byte
	{
		u32		stsize;
		u32		ver;
		u32		maxitemcount;
		u32		itemsize;
		u32		posbegin;
		u32		posend;
		u8		reserved[4];
		u32		checksum;
	} ST_LOG_INFOHDR, * LPST_LOG_INFOHDR;

	static BOOL				l_download 	= FALSE;
	static BOOL 			l_systemstarted = FALSE;
	static BOOL				l_bFormatFlag	= FALSE;
#if AK_LOG_SAVE_USE
	static ST_LOG_INFOHDR		l_log_ihdr 	= {0,};
	static BOOL				l_log_init 	= FALSE;


	int 			l_nAk_logTaskQuit=0;
	tid_t 			l_tidAk_log = NULL;
	CLHANDLE   	 	l_hAk_logMsg = NULL;
	u8				*l_cpLogBuffer = NULL;

/**********************************************************
 * log local macro & functions
 **********************************************************/
	#define		MACROLOG_ITEM_SIZE				(sizeof(ST_LOG_ITEM)) // flash k9f1208 : 512 is 1page size

	#define		MACROLOG_ISEMPTY(x)			(x.posbegin == x.maxitemcount && x.posend == x.maxitemcount)
	#define		MACROLOG_GEPOS(pos)			( POS_LOG_START + ((pos)*MACROLOG_ITEM_SIZE) )

/**********************************************************
 * log interface functions
 **********************************************************/

/**
	\addtogroup	ak_log
	\{
*/
#define ROM_LOG_HEADER_SIZE (l_spi_log.SecSz * 2) //8K

typedef struct {
	bool	init;
	u32 	SecSz;
	u32		PageSz;
	u32 	CfgMaxSize;
	u32 	LogMaxSize;
	
	u32		itemLastSavePage;
	u32		hdrLastSavePage;

	u32 	LogHdrSize;
	u32		maxitemcount;
	u32		posend;
		
} ST_SPIROM_DATA;

typedef enum {
	LOG_ITEM = 0,
	LOG_HDR,
} LOG_TYPE;

typedef struct
{
	u32 pos;
	u8 *pData;
	u32 length;
}SpiWriteFiFo;
/*=========================================================================*/
#define SPI_WIRTE_FIFO_SIZE		(32) // 32 * 8 items

static SpiWriteFiFo	 	SpiWrite_Stack[SPI_WIRTE_FIFO_SIZE] = { 0, };
static u32				curr_write_fifo = 0;


ST_SPIROM_DATA l_spi_log = { 0, };

BOOL __spirom_write_fifo_add(u32 pos, u8 *pData, u32 length);
void __spirom_write_fifo_process(void);


BOOL __ak_log_user_data_write(u32 pos, u8 *pData, u32 length)
{
	if(l_cpLogBuffer && pData && (pos + length) <= LOG_DATA_FILE_SIZE)
	{
		memcpy((void *)&l_cpLogBuffer[pos], (void *)pData, length);
		return TRUE;
	}
	else
		DEBUGMSG(AKLOG_ERROR, ("%s(): ERROR !\r\n", __func__)); 

	return FALSE;
}

BOOL __ak_log_user_data_read(u32 pos, u8 *pData, u32 length)
{
	if(l_cpLogBuffer && pData && (pos + length) <= LOG_DATA_FILE_SIZE)
	{
		memcpy(pData, &l_cpLogBuffer[pos], length);
		return TRUE;
	}
	else
		DEBUGMSG(AKLOG_ERROR, ("%s(): ERROR !\r\n", __func__)); 
	
	return FALSE;
}


BOOL __ak_log_spirom_write(u32 pos, u8 *pData, u32 length)
{
	int result;
	DEBUGMSG(AKLOG_FUNC, ("	[%s] : pos:%d, len:%d\r\n", __func__, pos, length)); 

	semaphore_get(sem_spi_mutex, SEM_WAIT);
	result = SpiRomWriteUData(ROM_UDATA_LOG, (unsigned int)pData, pos, length);
	semaphore_put(sem_spi_mutex);
	
	if(result != 0)
	{
		DEBUGMSG(AKLOG_ERROR, ("%s(): ERROR !\r\n", __func__)); 
		return FALSE;
	}
	
	return TRUE;
}

BOOL __ak_log_spirom_read(u32 pos, u8 *pData, u32 length)
{
	int result;
	DEBUGMSG(AKLOG_FUNC, ("	[%s] : pos:%d, len:%d\r\n", __func__, pos, length)); 

	semaphore_get(sem_spi_mutex, SEM_WAIT);
	result= SpiRomReadUData(ROM_UDATA_LOG, (unsigned int)pData, pos, length);
	semaphore_put(sem_spi_mutex);
	
	if(result != 0)
	{
		DEBUGMSG(AKLOG_ERROR, ("%s(): ERROR !\r\n", __func__)); 
		return FALSE;
	}
	
	return TRUE;
}

void __spirom_write_fifo_process(void)
{
	u32 write_fifo_count = 0;

	if ( curr_write_fifo == 0 )
		return;

	do{
		if(SpiWrite_Stack[write_fifo_count].length)
		{
			__ak_log_spirom_write(SpiWrite_Stack[write_fifo_count].pos, SpiWrite_Stack[write_fifo_count].pData, SpiWrite_Stack[write_fifo_count].length);
		
			SpiWrite_Stack[write_fifo_count].length = 0;

			DEBUGMSG(AKLOG_FUNC, ("	[%s] : (%d)\r\n", __func__, write_fifo_count)); 
		}
		write_fifo_count++;
	}while (write_fifo_count<curr_write_fifo);

	curr_write_fifo = 0;
}

BOOL __spirom_write_fifo_add(u32 pos, u8 *pData, u32 length)
{
	if(curr_write_fifo >= SPI_WIRTE_FIFO_SIZE)
	{
		DEBUGMSG(AKLOG_FUNC, ("	[%s] : SpiWrite Stack FULL!!!!(%d)\r\n", __func__, curr_write_fifo)); 
		__spirom_write_fifo_process();
	}
	
	if (curr_write_fifo < SPI_WIRTE_FIFO_SIZE)
	{
		SpiWrite_Stack[curr_write_fifo].pos = pos;
		SpiWrite_Stack[curr_write_fifo].pData = pData;
		SpiWrite_Stack[curr_write_fifo].length = length;


		curr_write_fifo++;

		DEBUGMSG(AKLOG_FUNC, ("	[%s] : (%d)\r\n", __func__, curr_write_fifo)); 
		
		return TRUE;
	}
	
	return FALSE;
}


BOOL __ak_log_spirom_update(LOG_TYPE type, u32 pos, u8 *pData, u32 length)
{
	if(l_spi_log.init)
	{
		DEBUGMSG(AKLOG_FUNC, ("	[%s] : pos:%d, len:%d\r\n", __func__, pos, length)); 
		if(__ak_log_user_data_write( pos, pData, length))
		{
			if(type == LOG_HDR)
				pos += l_spi_log.PageSz;
			else
				pos += length;
			
			if((pos % l_spi_log.PageSz) == 0)
			{
				u32 _pos = pos - l_spi_log.PageSz;

					if(g_pBbAppInst->eBB_Mode == eBB_MODE_RECORD) // Recording 중에는 Spi Read/Write시 녹색화면 발생함
					{
						__spirom_write_fifo_add(_pos, &l_cpLogBuffer[_pos], l_spi_log.PageSz);
					}
					else
					{
						__spirom_write_fifo_process();
						__ak_log_spirom_write( _pos, &l_cpLogBuffer[_pos] , l_spi_log.PageSz);
					}
			}
			else if(type != LOG_HDR)// hdr 영역에 update
			{
				u32 _pos = (l_spi_log.posend * l_spi_log.PageSz) + sizeof(ST_LOG_INFOHDR) + ((pos - length) % l_spi_log.PageSz);
				__ak_log_user_data_write( _pos, pData, length);
				DEBUGMSG(AKLOG_FUNC, ("	hdr backup: %d(%d)\n", _pos, length)); 
			}
			
			DEBUGMSG(AKLOG_FUNC, ("%s(): %d\n", __func__, pos)); 
			return TRUE;
		}
	}

	return FALSE;
}

BOOL __ak_log_spirom_hd_read(LPST_LOG_INFOHDR hdr)
{
	if(l_spi_log.init)
	{
		u32 pos	= ((l_spi_log.posend + (l_spi_log.maxitemcount - 1)) % l_spi_log.maxitemcount) * l_spi_log.PageSz; // sizeof(ST_LOG_INFOHDR);

		DEBUGMSG(AKLOG_FUNC, ("	[%s] : pos %d\r\n", __func__, pos)); 
		
		if(__ak_log_user_data_read(pos, (u8 *)hdr, sizeof(ST_LOG_INFOHDR)))
		//if(__ak_log_spirom_read( pos, (u8 *)hdr, sizeof(ST_LOG_INFOHDR)))
		{
			DEBUGMSG(AKLOG_FUNC, ("%s(): pos:%d\n", __func__, pos)); 
			return TRUE;
		}
	}

	return FALSE;
}


BOOL __ak_log_spirom_hdr_update(LPST_LOG_INFOHDR hdr)
{
	if(l_spi_log.init)
	{
		u32 pos = l_spi_log.posend * l_spi_log.PageSz; //hdr->stsize;

		DEBUGMSG(AKLOG_FUNC, ("	[%s] : posend:%d,  pos:%d\r\n", __func__, l_spi_log.posend, pos)); 
		
		if(__ak_log_spirom_update(LOG_HDR, pos, (u8 *)hdr, sizeof(ST_LOG_INFOHDR)))
		{
			l_spi_log.posend = (l_spi_log.posend + 1) % l_spi_log.maxitemcount;
			DEBUGMSG(AKLOG_FUNC, ("%s(): %d\n", __func__, l_spi_log.posend)); 

			if((l_spi_log.posend * l_spi_log.PageSz) % l_spi_log.SecSz == 0)
			{
				__spirom_write_fifo_process();
				semaphore_get(sem_spi_mutex, SEM_WAIT);
				SpiRomSectorEraseUData(ROM_UDATA_LOG, (l_spi_log.posend * l_spi_log.PageSz), l_spi_log.SecSz);
				semaphore_put(sem_spi_mutex);
			}
			
			return TRUE;
		}
	}

	return FALSE;
}

BOOL __ak_log_spirom_ready_save(void)
{
	if(l_spi_log.init)
	{
		u32 pos = MACROLOG_GEPOS(l_log_ihdr.posend + 1); // log item save
		
		DEBUGMSG(AKLOG_FUNC, ("	[%s]\r\n", __func__)); 

		if((pos % l_spi_log.PageSz) != 0) 
		{
#if 0			
			u32 length = (pos % l_spi_log.SecSz);

			DEBUGMSG(AKLOG_FUNC, ("	[%s] : pos:%d, length:%d\r\n", __func__, pos, length)); 
			
			length -= (length % l_spi_log.PageSz);
			
			pos -= (pos % l_spi_log.SecSz);
			//SpiRomSectorEraseUData(ROM_UDATA_LOG, pos, l_spi_log.SecSz);
			if(length)
				__ak_log_spirom_write( pos, &l_cpLogBuffer[pos] , length);	
#else
			u32 _pos	= ((l_spi_log.posend + (l_spi_log.maxitemcount - 1)) % l_spi_log.maxitemcount) * l_spi_log.PageSz  + sizeof(ST_LOG_INFOHDR);
			u32 length = (pos % l_spi_log.PageSz);

			//hdr update
			__ak_log_user_data_write( pos - length, &l_cpLogBuffer[_pos], length); 	
			DEBUGMSG(AKLOG_FUNC, ("	[%s] : pos:%d, length:%d, _pos:%d \r\n", __func__, pos, length, _pos)); 

			//current spi hdr update
			pos = l_spi_log.posend * l_spi_log.PageSz  + sizeof(ST_LOG_INFOHDR);
			__ak_log_user_data_write( pos, &l_cpLogBuffer[_pos], length); 	
#endif
		}			
		return TRUE;
	}

	return FALSE;
}

#if 0
BOOL __ak_log_spirom_last_save(void)
{
	if(l_spi_log.init)
	{
		u32 i;
		DEBUGMSG(AKLOG_FUNC, ("	[%s]\r\n", __func__)); 
		
		for( i=0; i < 2; i++)
		{
			u32 pos = 0;
			
			if(i==0)
				pos = l_spi_log.posend * l_spi_log.PageSz; //l_log_ihdr.stsize; // log header save
			else
				pos = MACROLOG_GEPOS(l_log_ihdr.posend); // log item save
			
			if((pos % l_spi_log.PageSz) != 0) 
			{
				pos -= (pos % l_spi_log.PageSz);
				__ak_log_spirom_write( pos, &l_cpLogBuffer[pos] , l_spi_log.PageSz);
			}
		}
		return TRUE;
	}
	return FALSE;
}
#endif


BOOL __ak_log_spirom_init(void)
{
	int result;
	l_spi_log.init = FALSE;

	semaphore_get(sem_spi_mutex, SEM_WAIT);
	result = SpiRomInitUData(&l_spi_log.SecSz, &l_spi_log.PageSz, &l_spi_log.CfgMaxSize, &l_spi_log.LogMaxSize);
	semaphore_put(sem_spi_mutex);
	
	if(result == 0)
	{
		DEBUGMSG(AKLOG_INIT, ("%s(): OK (SecSz:%d,  CfgMaxSize:%d, LogMaxSize:%d)\n", __func__, l_spi_log.SecSz, l_spi_log.CfgMaxSize, l_spi_log.LogMaxSize)); 

		l_spi_log.LogHdrSize = ROM_LOG_HEADER_SIZE;
		l_spi_log.maxitemcount = l_spi_log.LogHdrSize / l_spi_log.PageSz; //sizeof(ST_LOG_INFOHDR);
		l_spi_log.posend = 0;
		
		if(l_spi_log.LogHdrSize && l_spi_log.maxitemcount)
		{
			 unsigned char	*logHdrBuffer = ASAL_OS_AllocateMem(l_spi_log.LogHdrSize);
			 LPST_LOG_INFOHDR pLogInfoHdr = (LPST_LOG_INFOHDR)logHdrBuffer;
			 
			if(__ak_log_spirom_read( 0, logHdrBuffer, l_spi_log.LogHdrSize))
			{
				for( l_spi_log.posend = 0; l_spi_log.posend < l_spi_log.maxitemcount; l_spi_log.posend++)
				{
					if(pLogInfoHdr->stsize != sizeof(ST_LOG_INFOHDR) || util_GetCheckSum32((u32 *)pLogInfoHdr, sizeof(ST_LOG_INFOHDR)/4) != 0)
					{
						DEBUGMSG(AKLOG_INIT, (" %s(): spi log hdr pos check(%d)\n", __func__, l_spi_log.posend)); 
						break;
					}
					//pLogInfoHdr++;
					pLogInfoHdr += (l_spi_log.PageSz / sizeof(ST_LOG_INFOHDR));
				}

				if(l_spi_log.posend == l_spi_log.maxitemcount)
					l_spi_log.posend = 0;

				DEBUGMSG(AKLOG_INIT, (" %s(): spi log hdr pos(%d)\n", __func__, l_spi_log.posend)); 
			}

			ASAL_OS_FreeMem(logHdrBuffer);
		}

		if(l_spi_log.LogMaxSize)
		{
			l_spi_log.init = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL __ak_log_ihdr_update( void )
{
	l_log_ihdr.checksum 	= 0;
	l_log_ihdr.checksum 	= util_GetCheckSum32Value((u32 *)&l_log_ihdr, sizeof(ST_LOG_INFOHDR)/4);

	if(l_spi_log.init) {
		// PageSz 마다 hdr update
		if((MACROLOG_GEPOS(l_log_ihdr.posend + 1) % l_spi_log.PageSz) == 0)
			__ak_log_spirom_hdr_update(&l_log_ihdr);
	
		return TRUE;
	}
	else
		return __ak_log_user_data_write(POS_LOG_HDR, (char *)&l_log_ihdr, sizeof(ST_LOG_INFOHDR));
}
	
BOOL	__ak_log_add_item(LPST_LOG_ITEM pitem)
{
	u32 t_begin;
	u32 t_end;
	
	if( !l_log_init )	
	{
		return FALSE;
	}

	t_begin = l_log_ihdr.posbegin;
	t_end	= l_log_ihdr.posend;
	
	/* find position */
	if( MACROLOG_ISEMPTY(l_log_ihdr) )
	{
		t_begin = t_end = 0;
	}
	else if( ak_log_get_itemcount() == l_log_ihdr.maxitemcount )
	{
		if(l_spi_log.init == TRUE)
		{
			if( !ak_log_del((l_spi_log.SecSz * 2 / MACROLOG_ITEM_SIZE)))
				return FALSE;
		}
		else if( !ak_log_del(1) )
			return FALSE;

		t_begin = l_log_ihdr.posbegin;
		t_end	= l_log_ihdr.posend;

		if( ++t_end == l_log_ihdr.maxitemcount )
			t_end = 0;
	}
	else
	{
		if( ++t_end == l_log_ihdr.maxitemcount )
			t_end = 0;
	}

	/* write item */
	if(!__ak_log_spirom_update( LOG_ITEM, MACROLOG_GEPOS(t_end), (char *)pitem, MACROLOG_ITEM_SIZE) )	
	{
		if( !__ak_log_user_data_write( MACROLOG_GEPOS(t_end), (char *)pitem, MACROLOG_ITEM_SIZE) )
			return FALSE;
	}

	/* update information header */
	l_log_ihdr.posbegin = t_begin;
	l_log_ihdr.posend	= t_end;
	
//// debug 
#if AKLOG_FUNC
	{
		char str_type[64];
		ak_log_get_item_type_string(str_type, *pitem);
		DEBUGMSG(AKLOG_FUNC, ("LOG ADD : %s 	posbegin[%d], posend[%d], Time : %s\r\n", str_type, t_begin, t_end, asctime(localtime(&pitem->time)) ));
	}
#endif
	return __ak_log_ihdr_update();
}

#if 0
BOOL	__ak_log_add_item(LPST_LOG_ITEM pitem)
{
	int nRetCnt = 3;
	BOOL ret;
	do {
		ret = _ak_log_add( pitem );
	}while(ret == FALSE && (nRetCnt--));

	return ret;
}
#endif


int ak_log_load(unsigned short* path, void * logBuf)
{
    int dwLength;
    unsigned char type;
	unsigned int *fileHandle;
	
	DEBUGMSG(AKLOG_INIT, (" %s()\n", __func__));

	if(logBuf == NULL)
		logBuf = l_cpLogBuffer;
		
	if(logBuf)
	{
#if AKLOG_FUNC
		char asc_str[128];
		Uni2Ascii(PATH_CONFIG_FOLDER(path, AK_LOG_DATA_FILE_NAME), asc_str);
		DEBUGMSG(AKLOG_FUNC, ("%s(): %s\n", __func__, asc_str));
#endif	

	    fileHandle=ClFs_fopen(PATH_CONFIG_FOLDER(path, AK_LOG_DATA_FILE_NAME), "r");

	    if (fileHandle)
	    {
		    ClFs_fread(logBuf, LOG_DATA_FILE_SIZE, 1, fileHandle);
			ClFs_fclose(fileHandle);
			return TRUE;
	    }
	    else
	   	 	DEBUGMSG(AKLOG_ERROR,("Log File Open Error!!!\n"));

	}
	
	return FALSE;
}

int ak_log_backup(unsigned short* path)
{
    int dwLength;
    unsigned char type;
	unsigned int *fileHandle;
	
	DEBUGMSG(AKLOG_INIT, (" %s()\n", __func__));

	if(l_cpLogBuffer)
	{
		fileHandle=ClFs_fopen(PATH_CONFIG_FOLDER(path, AK_LOG_DATA_FILE_NAME), "w+");

	    if (fileHandle)
	    {
		    //ClFs_ftruncate(fileHandle, LOG_DATA_FILE_SIZE); //limjh_20111208 : 조각화방지
		    //ClFs_fseek(fileHandle,0,0);
		 	ClFs_fwrite(l_cpLogBuffer, LOG_DATA_FILE_SIZE, 1, fileHandle);
		   	ClFs_fclose(fileHandle);
	    }
	    else
	   	 	DEBUGMSG(AKLOG_ERROR,("Log File Open Error!!!\n"));
	}
	
	return FALSE;
}
	

static void ak_log_Task(void)
{
	ASAL_Error err;
	CLSINT32 waitTime = (60 * 1000);
	int msg_size;
	ST_LOG_ITEM 	logitem;

	l_nAk_logTaskQuit = 0;
	
	while(!l_nAk_logTaskQuit)
	{
		err = ASAL_OS_TimedReceiveMessage(l_hAk_logMsg, (char *)&logitem, sizeof(ST_LOG_ITEM), waitTime, &msg_size);

		if(err != ASAL_OSErr_None) //timeout
		{
		}
		else if(msg_size)
		{
		   	BOOL impossible_log_add = FALSE;
	     	DEBUGMSG(0, ("%s Receive Queue %d\r\n", __func__, logitem.type));

			if(logitem.type == LOG_EVENT_UPDATE && l_download == FALSE)
			{
				impossible_log_add = FALSE;
				l_download = TRUE;
			}
			else if(logitem.type == LOG_EVENT_UPDATE && l_download == TRUE)
				impossible_log_add = l_download = FALSE;
			else
				impossible_log_add = l_download;
			
			if(logitem.type == LOG_SYSBOOT)
				l_systemstarted = TRUE;
			else if(logitem.type == LOG_EVENT_MEMORY_FORMAT && logitem.type_sub == 0)
				l_bFormatFlag	= TRUE;
				
			if(l_log_init == FALSE && l_systemstarted && impossible_log_add == FALSE)
				ak_log_init(0);

			DEBUGMSG(l_download, ("%s(): Dwonloading...", __func__));
			DEBUGMSG((l_systemstarted==FALSE), ("%s(): system not started...", __func__));
			
			if(l_systemstarted && impossible_log_add == FALSE)
	       		__ak_log_add_item(&logitem);
			else
				DEBUGMSG(AKLOG_ERROR, (" log add(%d) Skip\r\n", logitem.type));

			if(logitem.type == LOG_SYSEND)
			{
				ak_log_release(1);
				l_systemstarted = FALSE;
		  	}
			else if(logitem.type == LOG_EVENT_MEMORY_FORMAT && logitem.type_sub == 1)
				l_bFormatFlag = FALSE;
		}		
	}
	
	ASAL_OS_ExitThread();
}

void ak_log_TaskStart (message_t* msg_ID)
{
	DEBUGMSG( AKLOG_INIT, ("*** %s() ***\n", __func__));

	
	if(l_hAk_logMsg == NULL)
	{
		ASAL_Error iRet;
		iRet = ASAL_OS_CreateMessageQueue(&l_hAk_logMsg ,"AkLog_Msg", 10, sizeof(ST_LOG_ITEM));

		if(iRet != ASAL_OSErr_None)
			DEBUGMSG( AKLOG_ERROR, ("%s(): Message Queue Create Failed\n", __func__));
	}
	
	if (l_tidAk_log == NULL)
	   l_tidAk_log = thread_start("AkLog_Task", UI_APP_PRIORITY + 4 , NULL, (4*1024), (func_t)ak_log_Task, NULL);
}
void ak_log_TaskStop(void)
{
	if (l_tidAk_log)
	{
		int delay = 5000;

		l_nAk_logTaskQuit = TRUE;
		ak_log_add_ex(LOG_TASK_EXIT, NULL, NULL);
		while(l_nAk_logTaskQuit == 0 && delay--) 
			thread_delay(10);
		
		thread_delay(10);
		thread_delete(l_tidAk_log);
		l_tidAk_log = NULL;
	}
	
	if (l_hAk_logMsg)
	{
		ASAL_Error	iRet;
		iRet = ASAL_OS_DeleteMessageQueue(l_hAk_logMsg);
		if(iRet != ASAL_OSErr_None)
			DEBUGMSG( AKLOG_ERROR, ("%s() : Queue Delete Failed\n", __func__));
		else
			l_hAk_logMsg = NULL;
	}

	DEBUGMSG( AKLOG_ERROR, ("*** %s() ***\n", __func__));
}

	
void	ak_log_init( BOOL isForce)
{
	BOOL	isInitDefault	= FALSE;
	u32		maxitemcount	= 0;
	u32		addr = 0, size = 0;
	
	int dummy;
	unsigned int pBytesRead;

	if(l_cpLogBuffer == NULL)
	{
		l_cpLogBuffer = ASAL_OS_AllocateMem(LOG_DATA_FILE_SIZE);

		if(l_cpLogBuffer == NULL)
		{
			DEBUGMSG(AKLOG_ERROR, ("%s(): mv_MemAlloc FAIL !\r\n", __func__)); 
			return;
		}
		if(__ak_log_spirom_init())
		{
			if(l_spi_log.LogMaxSize)
			{
				l_cpLogBuffer = ASAL_OS_AllocateMem(l_spi_log.LogMaxSize);
				
				__ak_log_spirom_read(0, l_cpLogBuffer, l_spi_log.LogMaxSize);
			}
		}

		if(l_cpLogBuffer == NULL)
		{
			l_spi_log.LogMaxSize = ( 256 * 1024); //256K
			l_spi_log.LogHdrSize = sizeof(ST_LOG_INFOHDR);

			l_cpLogBuffer = ASAL_OS_AllocateMem(l_spi_log.LogMaxSize);
			
			ak_log_load(SDCARD_PATH, l_cpLogBuffer); // temp
		}
	}

	maxitemcount	= (LOG_DATA_FILE_SIZE - l_spi_log.LogHdrSize) / sizeof(ST_LOG_ITEM);
	
	if(l_log_init == NULL)
	{
		bool ret;

		curr_write_fifo = 0;
		memset((void*)SpiWrite_Stack, 0x00, sizeof(SpiWriteFiFo) * SPI_WIRTE_FIFO_SIZE);
		
		if(l_spi_log.init)
		{
			ret = __ak_log_spirom_hd_read(&l_log_ihdr);
			__ak_log_spirom_ready_save();
		}
		else
			ret = __ak_log_user_data_read(POS_LOG_HDR, (char *)&l_log_ihdr, sizeof(ST_LOG_INFOHDR));
			
		if(ret)
		{
			if( l_log_ihdr.stsize != sizeof(ST_LOG_INFOHDR) ||
				l_log_ihdr.ver != LOG_VERSION ||
				l_log_ihdr.itemsize != sizeof(ST_LOG_ITEM) ||
				l_log_ihdr.maxitemcount != maxitemcount )
			{
				DEBUGMSG(AKLOG_ERROR, ("%s(): HEADER ERROR!\r\n", __func__)); 
				DEBUGMSG(AKLOG_ERROR, ("stsize:%d, ver:%d, itemsize:%d, maxitemcount:%d\n", l_log_ihdr.stsize, l_log_ihdr.ver, l_log_ihdr.itemsize, l_log_ihdr.maxitemcount)); 
				isInitDefault = TRUE;
			}
			else
			{
				if( util_GetCheckSum32((u32 *)&l_log_ihdr, sizeof(ST_LOG_INFOHDR)/4) != 0 )
				{
					isInitDefault = TRUE;
					DEBUGMSG(AKLOG_ERROR, ("%s(): CHECKSUM ERROR!\r\n", __func__)); 
				}
			}
		}
		else
		{
			isInitDefault = TRUE;
			DEBUGMSG(AKLOG_ERROR, ("%s(): HEADER READ ERROR!\r\n", __func__)); 
		}
	}
	
	if( isForce || isInitDefault )
	{
		DEBUGMSG(AKLOG_FUNC, ("%s(): DEFAULT SET (isForce=%d, inInitDefault=%d)\r\n", __func__, isForce, isInitDefault)); 

		memset((void*)l_cpLogBuffer, 0x00, LOG_DATA_FILE_SIZE);
	
		memset((void*)&l_log_ihdr, 0x00, sizeof(ST_LOG_INFOHDR));

		l_log_ihdr.stsize		= sizeof(ST_LOG_INFOHDR);
		l_log_ihdr.ver			= LOG_VERSION;
		l_log_ihdr.maxitemcount	= maxitemcount;
		l_log_ihdr.itemsize		= MACROLOG_ITEM_SIZE;
		l_log_ihdr.posbegin		= maxitemcount;
		l_log_ihdr.posend		= maxitemcount;

		if( !__ak_log_ihdr_update() )
			return;

		__ak_log_spirom_hdr_update(&l_log_ihdr);
	}

	DEBUGMSG(AKLOG_FUNC, ("%s(): INIT OK (posbigin=%d, posend=%d)\r\n", __func__, l_log_ihdr.posbegin, l_log_ihdr.posend)); 
	l_log_init = TRUE;
}

void	ak_log_release( BOOL diskBackup )
{
	if(l_log_init)
	{
		if(l_spi_log.init == TRUE)
		{
			
			if((MACROLOG_GEPOS(l_log_ihdr.posend + 1) % l_spi_log.PageSz) != 0)
			{
				__ak_log_spirom_hdr_update(&l_log_ihdr);
				thread_delay(100);
			}
			
//			__ak_log_spirom_last_save();
			__spirom_write_fifo_process();
		}
			
		if(diskBackup)
		{	
			if(IsReadySdcardFS)
				ak_log_backup(SDCARD_PATH);
			
			if(IsReadyHmscFS)
				ak_log_backup(MSC_PATH);
			
		}
		l_log_init = FALSE;

		DEBUGMSG(AKLOG_INIT, ("%s(): RELEASE OK (posbigin=%d, posend=%d)\r\n", __func__, l_log_ihdr.posbegin, l_log_ihdr.posend)); 
	}

	if(l_cpLogBuffer)
	{
		ASAL_OS_FreeMem(l_cpLogBuffer);
		l_cpLogBuffer = NULL;
	}
}

BOOL ak_log_add(LPST_LOG_ITEM pitem)
{
	if(l_hAk_logMsg)
	{
		ASAL_OS_SendMessage(l_hAk_logMsg,(const char *)pitem, sizeof(ST_LOG_ITEM), enASAL_OS_MsgPrioNormal, enASAL_OS_MsgWaitOptNowait);
		return TRUE;
	}
	
	return FALSE;	
}


BOOL	ak_log_del(u32 count)
{
	if( !l_log_init )	
	{
		return FALSE;
	}

	DEBUGMSG(AKLOG_FUNC, ("++%s(): count%d (posbigin=%d, posend=%d)\r\n", __func__, count, l_log_ihdr.posbegin, l_log_ihdr.posend)); 
	
	if( MACROLOG_ISEMPTY(l_log_ihdr) )
		return FALSE;

	if( ak_log_get_itemcount() <= count )
	{
		ak_log_release(0);
		ak_log_init(TRUE);
		return TRUE;
	}
	else if( (l_log_ihdr.posbegin + count) >= l_log_ihdr.maxitemcount )
		l_log_ihdr.posbegin = count - (l_log_ihdr.maxitemcount - l_log_ihdr.posbegin);
	else
		l_log_ihdr.posbegin += count;

	DEBUGMSG(AKLOG_FUNC, ("--%s(): count%d (posbigin=%d, posend=%d)\r\n", __func__, count, l_log_ihdr.posbegin, l_log_ihdr.posend)); 
	
	return __ak_log_ihdr_update();
}

u32		ak_log_get_itemcount( void )
{
	if( !l_log_init )	
	{
		return FALSE;
	}
	
	if( MACROLOG_ISEMPTY(l_log_ihdr) )
		return 0;

	if( l_log_ihdr.posbegin > l_log_ihdr.posend )
		return l_log_ihdr.maxitemcount - l_log_ihdr.posbegin + l_log_ihdr.posend + 1;

	return l_log_ihdr.posend - l_log_ihdr.posbegin + 1;
}

u32	ak_log_get_item(u32 pos, u32 count, LPST_LOG_ITEM pitemarray)
{
	u32 _startpos	= 0;
	u32 _readcount	= 0;
	u32 _itemcount 	= 0;
	//u8	i			= 0;
	u32 arraypos	= 0;
	
	if( !l_log_init )	
	{
		return FALSE;
	}

	if( MACROLOG_ISEMPTY(l_log_ihdr) )
		return FALSE;

	_itemcount = ak_log_get_itemcount();
	
	if( _itemcount - pos >= count )
		_readcount	= count;
	else
		_readcount	= _itemcount - pos;

	if(l_log_ihdr.posbegin + pos + _readcount <= l_log_ihdr.maxitemcount)
		_startpos = l_log_ihdr.posbegin + pos;//(l_log_ihdr.maxitemcount - 1) - (pos + _readcount - l_log_ihdr.posend);
	else if( l_log_ihdr.posend >= (_readcount-1) )
		_startpos = l_log_ihdr.posend - (_readcount - 1);
	else
		_startpos = l_log_ihdr.posbegin + pos;//(l_log_ihdr.maxitemcount - 1) - (pos + _readcount - l_log_ihdr.posend);


	DEBUGMSG(0, ("pos=%d, _itemcount=%d, _startpos=%d, _readcount=%d, count = %d (posbigin=%d, posend=%d)\r\n", pos, _itemcount, _startpos, _readcount, count, l_log_ihdr.posbegin, l_log_ihdr.posend)); 

	if( (_startpos + _readcount) > l_log_ihdr.maxitemcount )
	{
		arraypos = l_log_ihdr.maxitemcount - _startpos;
		ak_log_get_item(	pos,
							arraypos,
							pitemarray);
		ak_log_get_item(	pos + arraypos,
							_readcount-arraypos,
							&pitemarray[arraypos] ); //pitemarray );//&pitemarray[ITEM_SIZE-arraypos] );
	}
	else
	{
		if( !__ak_log_user_data_read( MACROLOG_GEPOS(_startpos), (char *)pitemarray, _readcount * MACROLOG_ITEM_SIZE) )
			return 0;
	}
	DEBUGMSG(0, ("_startpos=%d, _readcount=%d (posbigin=%d, posend=%d)\r\n", _startpos, _readcount, l_log_ihdr.posbegin, l_log_ihdr.posend)); 
//	DEBUGMSG(1, (" pos:%d, get log itime Time:%d \r\n",pos, pitemarray->time));
	return _readcount;
}

BOOL ak_log_get_item_type_string(char *szbuff, ST_LOG_ITEM read_logitem)
{
	switch(read_logitem.type)
	{
	case LOG_SYSBOOT:
		sprintf(szbuff,"SYSTEM START");
		break;
	case LOG_SYSEND:
		sprintf(szbuff,"SYSTEM END");
		break;
	case LOG_EVENT_DATETIMECHANGE:
		/* sub type */
		if( read_logitem.type_sub == RTC_SRC_GPS )
			sprintf((char*)szbuff, "TIME SETUP(GPS)");
		else if( read_logitem.type_sub == RTC_SRC_SETUP )
			sprintf((char*)szbuff, "TIME SETUP(PC)");
		else
			sprintf((char*)szbuff, "TIME SETUP");
		
		break;
	case LOG_ERRDBOVERTIME:
		sprintf((char*)szbuff, "WRITE ERROR - OVER TIME");
		break;
	case LOG_EVENT_REC_BEGIN:
		sprintf((char*)szbuff, "RECORD(BEGIN)");
		break;
	case LOG_EVENT_REC_END:
		sprintf((char*)szbuff, "RECORD(END)");
		break;
	case LOG_EVENT_LOGINOUT:
		/* sub type */
		if( read_logitem.type_sub == _LOG_LOGIN )
			sprintf((char*)szbuff, "LOG IN  ");
		else if( read_logitem.type_sub == _LOG_LOGOUT )
			sprintf((char*)szbuff, "LOG OUT ");

		break;
	case LOG_EVENT_MEMORY_FORMAT:
		sprintf((char*)szbuff, "FORMAT");
		break;
	case LOG_EVENT_BEGIN_SLEEP:
		sprintf((char*)szbuff, "SLEEP MODE");
		break;
	case LOG_EVENT_ACC_STATE:
		sprintf((char*)szbuff, "ACC");
		break;
	case LOG_EVENT_MP4_FILE_RECOVERY:
		sprintf((char*)szbuff, "RECOVERY");
		break;
	case LOG_DISCHARGE_SLEEP :
		sprintf((char*)szbuff, "DISCHARGE");
		break;
	case LOG_EVENT_MEMORY_INFO:
		sprintf((char*)szbuff, "MEMORY INFO");
		break;
	case LOG_EVENT_SD_CD:
		sprintf((char*)szbuff, "SD CARD");
		break;
	case LOG_EVENT_USB_CD:
		sprintf((char*)szbuff, "USB MEMORY");
		break;
	case LOG_EVENT_BATTCHANGE:
		if( read_logitem.type_sub == eBB_BATT_STATE_USB_NORMAL )
			sprintf((char*)szbuff, "BATT USB NORMAL");
		else if( read_logitem.type_sub == eBB_BATT_STATE_USB_FALL )
			sprintf((char*)szbuff, "BATT USB FALL");
		else if( read_logitem.type_sub == eBB_BATT_STATE_NORMAL )
			sprintf((char*)szbuff, "BATT NORMAL");
		else if( read_logitem.type_sub == eBB_BATT_STATE_PARKING)
			sprintf((char*)szbuff, "BATT PARKING");
		else if( read_logitem.type_sub == eBB_BATT_STATE_LOW)
			sprintf((char*)szbuff, "BATT LOW");
		else if( read_logitem.type_sub == eBB_BATT_STATE_EMER_LOW)
			sprintf((char*)szbuff, "BATT EMER LOW");
		else if( read_logitem.type_sub == eBB_BATT_STATE_FALL)
			sprintf((char*)szbuff, "BATT FALL");
		else if(read_logitem.type_sub == eBB_BATT_STATE_POWER_SWITCH_OFF)
			sprintf((char*)szbuff, "SWITCH OFF");
		else
			sprintf((char*)szbuff, "BATT");
		
		break;
	case LOG_EVENT_EVENT_RECORDING:
		sprintf((char*)szbuff, "EVENT");
		break;

	case LOG_EVENT_GSENSOR_ERROR:
		sprintf((char*)szbuff, "G_SENSOR");
		break;
	case LOG_EVENT_BUTTON_POWER:
		sprintf((char*)szbuff, "KEY");
		break;
	case LOG_EVENT_UPDATE:
		sprintf((char*)szbuff, "S/W UPDATE");
		break;

	//// error message
	case LOG_ERROR_RTC_READ_FAIL :
		sprintf((char*)szbuff, "[ERROR] RTC");
		break;
	case LOG_ERROR_SD_DISK_INIT_FAIL :
		sprintf((char*)szbuff, "[ERROR] SD");
		break;
	case LOG_ERROR_USB_DISK_INIT_FAIL:
		sprintf((char*)szbuff, "[ERROR] USB");
		break;
	case LOG_ERROR_MEDIA_ERROR :
		sprintf((char*)szbuff, "[ERROR] MEDIA");
		break;
	case LOG_ERROR_MEDIA_WARNING :
		sprintf((char*)szbuff, "[ERROR] MEDIA WARNING");
		break;
	case LOG_ERROR_INTERNAL_ERR	:
		sprintf((char*)szbuff, "[ERROR] INTERNAL");
		break;
	
	default:
		sprintf(szbuff,"%d", read_logitem.type);
		break;
	}

	return TRUE;
}
////////////////////// TEST FUNCTION ///////////////
void ak_log_test_main(void)
{
	#define MACRO_TEST_ITEM_SIZE		10
	
	int i, read_count;
	ST_LOG_ITEM 	read_logitem[MACRO_TEST_ITEM_SIZE+5];
	char str[64];
	stOSAL_TimeSpec sTime_t;
	
	/* add log */
	ST_LOG_ITEM 	logitem;
	memset((void*)&logitem, 0x00, sizeof(ST_LOG_ITEM));
	logitem.type	= LOG_DISCHARGE_SLEEP;
	
	OSAL_GetClockTime(&sTime_t);
	logitem.time	= sTime_t.tv_sec;
	
	for( i = 0; i< MACRO_TEST_ITEM_SIZE; i++)
	{
		sprintf((char *)logitem.data.byte, "COUNT %d", ak_log_get_itemcount());
		ak_log_add(&logitem);
	}

	/* load log */
	read_count = ak_log_get_item(0, MACRO_TEST_ITEM_SIZE, read_logitem);

		
	DEBUGMSG(1, (" type = %d \r\n", read_logitem[0].type));
	DEBUGMSG(1, (" time = %s \r\n", asctime(localtime (&read_logitem[0].time))));
	for( i = 0; i< read_count; i++)
	{
		DEBUGMSG(1, (" data = %s \r\n", read_logitem[i].data.byte));
		DEBUGMSG(1, (" itemcount = %d \r\n\r\n", ak_log_get_itemcount()- read_count + i));
	}	
}

#endif	//AK_LOG_SAVE_USE
BOOL ak_log_get_download_flag(void)
{
	return l_download;
}

BOOL ak_log_get_system_run_flag(void)
{
	return l_systemstarted;
}

BOOL ak_log_get_format_run_flag(void)
{
	return l_bFormatFlag;
}

BOOL	ak_log_add_ex(u16 type, u16 type_sub, char *fmt, ...)
{
#if AK_LOG_SAVE_USE
	/* add log */
	ST_LOG_ITEM 	logitem;
	stOSAL_TimeSpec sTime_t;
	
	va_list argP;
	char string[255] = {0,};
	
	va_start(argP, fmt);
	vsprintf(string, fmt, argP);
	va_end(argP);

	OSAL_GetClockTime(&sTime_t);
	
	memset((void*)&logitem, 0x00, sizeof(ST_LOG_ITEM));

	logitem.type	= type;
	logitem.type_sub= type_sub;
	logitem.time	= sTime_t.tv_sec;
	strncpy((char *)logitem.data.byte, string, LOG_ITEM_DATA_SIZE);

	return ak_log_add(&logitem);
#else
	return TRUE;
#endif
}


#define POWER_ON_LOG_USE 0
//! ----------------------------------------------------------------------------
/** \Functions  : BboxAppBootInit
 *	\Brief      : Boot Status Check
 *	\Param      :
 *	\Remarks    :
 *	\Return     : 0 : NORMAL , 1 : ABNORMAL
 */
//! ----------------------------------------------------------------------------
int BboxAppPowerOnLogInit(unsigned short * drive)
{
    int             ret_val = 0;
#if POWER_ON_LOG_USE	
    unsigned short      filename[PATH_MAX_LENGTH]={0,};
    CLFS_handle    *fileHandle=NULL;
    unsigned char  *message;
	char strTime[PATH_MAX_LENGTH]={0,};
	DEBUGMSG(AKLOG_INIT, (" ++%s()\n", __func__));
    StrCpy_Uni(filename, PATH_CONFIG_FOLDER((drive == NULL) ? GetRecordingRootDrive():drive, L"poweron.log"));

    fileHandle = ClFs_fopen(filename, "r");//파일이 있습니까 ?
    if(fileHandle)// exist
    {
    	ClFs_fread(strTime, PATH_MAX_LENGTH, 1, fileHandle);
		ClFs_fclose(fileHandle);
		
    	DEBUGMSG(AKLOG_ERROR, (" LAST Power Save Error!\n %s\n", strTime));
        //ClFs_fclose(fileHandle);
        //ret_val = 1;// Abnormal Power reset
        if(ClFs_remove(filename) == 0)
			fileHandle = NULL;
		
		DEBUGMSG(AKLOG_ERROR, (" LAST Power Save remove File\n %s\n", strTime));
    }

    if(!fileHandle)// anyway
    {
        fileHandle = ClFs_fopen(filename, "w+");//파일이 있습니까 ?
        if(fileHandle)// ok
        {
			stOSAL_TM sTime;
			
			get_local_seconds(&sTime);
			strcpy( strTime, asctime(&sTime));
			
            ClFs_fwrite(strTime, strlen(strTime), 1, fileHandle);
            ClFs_fclose(fileHandle);
//	            ClFs_setAttribute(filename, 0x02);// ATTR_HIDDEN
//limjh_20111211 : 짧은 파일임에도 FS에서 저장할때 LFN속성 부여
//limjh_20111211 : 현재 chkdsk 에서 LFN 이면서  hidden 이면 속성을 에러로 인식함
        }
    }

	DEBUGMSG(AKLOG_INIT, (" --%s():%s\n", __func__, strTime));
#endif	
	return ret_val;
}

//! ----------------------------------------------------------------------------
/** \Functions  : BboxAppBootInit
 *	\Brief      : Boot Status Check
 *	\Param      :
 *	\Remarks    :
 *	\Return     : 0 : NORMAL , 1 : ABNORMAL
 */
//! ----------------------------------------------------------------------------
int BboxAppPowerOffLogInit(unsigned short * drive)
{
    int             ret_val = 0;
#if POWER_ON_LOG_USE		
    unsigned short      filename[PATH_MAX_LENGTH]={0,};

	DEBUGMSG(AKLOG_INIT, (" ++%s()\n", __func__));
    StrCpy_Uni(filename, PATH_CONFIG_FOLDER((drive == NULL) ? GetRecordingRootDrive():drive, L"poweron.log"));
    ret_val = ClFs_remove(filename);//파일이 있습니까 ?
	DEBUGMSG(AKLOG_INIT, (" --%s():(%d)\n", __func__, ret_val));
#endif	
	return ret_val;
}


void Log_Print(const char * fmt, ...)
{

}


/** \} */
