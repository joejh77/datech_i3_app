// pai_r_data.cpp: implementation of the CSystemlog class.
//
//////////////////////////////////////////////////////////////////////
#include <stdarg.h>
#include <fcntl.h>
#include <thread>
#include "daappconfigs.h"
#include "datools.h"
#include "sysfs.h"
#include "SB_System.h"
//#include "tinyxml.h"
#include "data_log.h"

#define DBG_DATA_LOG_FUNC 	1 // DBG_MSG
#define DBG_DATA_LOG_ERR  		DBG_ERR
#define DBG_DATA_LOG_WRN		DBG_WRN

#if ENABLE_DATA_LOG

#define LOG_MSG_POWER_OFF		"D09\r\n"


CDatalog::CDatalog()
{
	m_is_init = false;
	m_log_position = 0;
	m_file_count = 0;
	m_file_serial_no = 0;

	m_log_file_max_count = DATALOG_FILE_MAX_COUNT;
		
	m_serial_number = 0;
	m_last_add_time = 0;
	m_event_recording = false;
	m_event_cause_category = EVENT_CAUSE_CATEGORY_NO_EVENT;
	m_event_cause_difference = 0;
		
	m_pre_operating_time = 0;
	m_log_add_count = 0;

	strcpy(m_driver_code, "0");
	
	m_fp = NULL;
}

CDatalog::~CDatalog()
{
	deinit();
}

int CDatalog::add_log(LPST_DATA_LOG_ITEM pLog)
{
	//m_last_add_time = pLog->Time;
	
#ifdef ENABLE_DATA_LOG_UNI
	if(m_log_add_count == 0){
		if(pLog->milisec_time > 100)
			return 0;
	}
	m_log_add_count++;
	
	if(m_data_log_list.size() > 901) {// test 200
		m_data_log_list.pop_front();
	}
#else
	if(m_data_log_list.size() > 61) {
		m_data_log_list.pop_front();
	}
#endif
	m_data_log_list.push_back(*pLog);

	return m_data_log_list.size();
}

int CDatalog::save_log(bool shutdown)
{
	ST_DATA_LOG_ITEM pLog; // 

	int log_count = m_data_log_list.size();
	int log_length = 0;
	int i;
	
	if(log_count) {
		struct tm pre_tm_t;
		ITER_DATALOG iLog = m_data_log_list.begin();
		u32 size = log_count * DEF_DATA_LOG_STRING_MAX_SIZE + 256;
		char* log_string = new char[  size ];

		if(!m_is_init){
			if(init(iLog->Time) == 0){
				delete [] log_string;
				return 0;
			}
		}
/*		
#ifdef ENABLE_DATA_LOG_UNI		
		if(m_data_log_list.size() > 150) {// test
			m_data_log_list.pop_front();
			log_count -= 1;
			iLog = m_data_log_list.begin();
		}
#endif	
*/
		localtime_r(&m_pre_log_time, &pre_tm_t);
		
		for( i=0; i < log_count; i++) {
			ST_DATA_LOG_ITEM log = *iLog;
			struct tm tm_t;
			
			localtime_r(&iLog->Time, &tm_t);			

			//if(tm_t.tm_min != pre_tm_t.tm_min) { // test code min change
			if(tm_t.tm_mday != pre_tm_t.tm_mday) { // day change
				if(log_length){
					write_log((void *)log_string, log_length);
					log_length = 0;
				}
#ifdef ENABLE_DATA_LOG_UNI

#else
				const char * power_off_log = LOG_MSG_POWER_OFF; //power off
				write_log((void *)power_off_log, sizeof(power_off_log));
#endif
				deinit();
				if(init(iLog->Time) == 0){
					delete [] log_string;
					return 0;
				}
			}
#ifdef ENABLE_DATA_LOG_UNI

			m_pre_operating_time = iLog->OperatingTime;
			log_length += make_log_string(&log_string[log_length], size - log_length , &log, tm_t);

#else
			if(m_pre_operating_time != iLog->OperatingTime){
				m_pre_operating_time = iLog->OperatingTime;
				log_length += make_log_string(&log_string[log_length], size - log_length , &log, tm_t);
			}
#endif			
			m_data_log_list.erase(iLog);
			
			iLog = m_data_log_list.begin();				
		}
		
		if(log_length){
			write_log((void *)log_string, log_length);
		}

		delete [] log_string;
	}

	if(shutdown){
#ifdef ENABLE_DATA_LOG_UNI
		if(pLog.milisec_time < 1000)
			return 0;
#else		
		const char * power_off_log = LOG_MSG_POWER_OFF; //power off
		write_log((void *)power_off_log, sizeof(power_off_log));
#endif
#if 0		
		u32 size = size= 32 * 1024;  //offs_write_alignemnt_size
		char* dummy = new char[  size ];

		memset((void *)dummy, 0, size);
		write_log((void *)dummy, size);
		delete [] dummy;
#endif
		deinit();
	}
	
	return log_count;
}

///////////////////////////////////////////horiba
int CDatalog::Horiba_Header_Creation(time_t time)
{
	int ret;
	u32 size = 64 * 1024;
	char data_path[256];
	struct tm tm_t;
	int length;
	
	char* dummy = new char[size];
	memset((void *)dummy, 0x00, size);
	fwrite((void *)dummy, 1, size, m_fp);

	fseek( m_fp, 0, SEEK_END );
	length = ftell( m_fp );
	fseek( m_fp, 0, SEEK_SET );

	dbg_printf(DBG_DATA_LOG_FUNC, "%s file created( %d : %d KB).\n", data_path, DATALOG_FILE_SIZE / 1024, length/1024);

	localtime_r(&time, &tm_t);

	m_log_position = sprintf(dummy, "H01,FWVersion,\"%s\"\r\n", __FW_VERSION__ );
	m_log_position += sprintf(&dummy[m_log_position], "H02,SerialNumber,\"%u\"\r\n", m_serial_number );
	m_log_position += sprintf(&dummy[m_log_position], "H03,Date,\"%04d-%02d-%02d\"\r\n", tm_t.tm_year + 1900, tm_t.tm_mon + 1, tm_t.tm_mday );
	m_log_position += sprintf(&dummy[m_log_position], "H04,DriverCode,\"%s\"\r\n", m_driver_code );
#ifdef ENABLE_DATA_LOG_UNI
	m_log_position += sprintf(&dummy[m_log_position], "D01,Date,Time,GpsConnectionState,GpsSignalState,Latitude,Longitude,Speed(Km/h),Distance(Km),RPM,GS_Event,GS_X,GS_Y,GS_Z,Brake,Winker_L,Winker_R,InputTrigger1,InputTrigger2,Volt" );
#else
	m_log_position += sprintf(&dummy[m_log_position], "D01,Date,Time,OperatingTime,GpsConnectionState,GpsSignalState,Latitude,Longitude,Speed(Km/h),Distance(Km),RPM,GS_Event,GS_X,GS_Y,GS_Z,Brake,Winker_L,Winker_R,InputTrigger1,InputTrigger2,Volt,Cause,Difference" );
#endif
#ifdef DEF_SAFE_DRIVING_MONITORING
	m_log_position += sprintf(&dummy[m_log_position], ",Highway\r\n");
#else
	m_log_position += sprintf(&dummy[m_log_position], "\r\n");
#endif

	ret = fwrite((void *)dummy, 1, strlen(dummy) + 1, m_fp);

	dbg_printf(DBG_DATA_LOG_FUNC, "%s() :  %d (%d : %s) \n", __func__, m_log_position, ret, dummy);

}
int CDatalog::Dummy_File_Move(char *data_path)
{
	int dummy_file_count;
	
	for(dummy_file_count = 0; dummy_file_count < 30; dummy_file_count++){
		char tmp_path[256]; 
		
		sprintf(tmp_path, DEF_DATA_LOG_TMP_DIR "/%d.tmp", dummy_file_count);
		if(access(tmp_path, R_OK) == 0){
			char mvCmd[256];

			sprintf(mvCmd, "mv %s %s", tmp_path, data_path);
			system(mvCmd);
		
			return 1;
		}
	}

	return 0;
}

int CDatalog::Dummy_File_Make(void)
{
	int dummy_file_count;		

	for(dummy_file_count = 0; dummy_file_count < 30; dummy_file_count++){
		char tmp_path[256]; 
		
		sprintf(tmp_path, DEF_DATA_LOG_TMP_DIR "/%d.tmp", dummy_file_count);
		
		FILE *t_fp = fopen(tmp_path, "wb"); 	
		if(t_fp) {
			u32 size = DATALOG_FILE_SIZE;
		
			char* dummy = new char[size];
			memset((void *)dummy, 0x00, size);
			fwrite((void *)dummy, 1, size, t_fp);

			delete [] dummy;
			
			fclose(t_fp);
			dbg_printf(DBG_DATA_LOG_FUNC, "%s saved %d KB\n", DEF_DATA_LOG_TMP_DIR, size / 1024);
		}else{
			char szCmd[256];

			sprintf(szCmd, "rm %s", tmp_path);
			system(szCmd);

			dbg_printf(DBG_DATA_LOG_FUNC, "%s file make failed\n", tmp_path);
		
		}
		
	}
	return 0;
}
/////////////////////////////////////////////////

int CDatalog::init(time_t time)
{

	u32 sd_size = 0;
	
	if(m_is_init)
		return m_is_init;

	if(sysfs_scanf("/sys/block/mmcblk0/size", "%u", &sd_size)){
		if(sd_size <= 16777216) { //8G
			m_log_file_max_count = DATALOG_FILE_MAX_COUNT / 2;
			
			dbg_printf(DBG_DATA_LOG_FUNC, " 8G or less than 8G");
		}
	}
	struct tm last_log_file_tm_t;
	
	const char * data_dir = DEF_DATA_LOG_DIR;
	const char * tmp_dir = DEF_DATA_LOG_TMP_DIR; //horiba
	char data_path[256];
	char tmp_path[256];
	char oldest_file[512];
	u32	t_oldest_log_file_no = 0xffffffff;
	
	struct tm tm_t;
	int length;

#ifdef Horiba	///////////////////////////////////////////horiba
	if (access(tmp_dir, R_OK) != 0){
		char szCmd[128];
		sprintf(szCmd, "mkdir %s", tmp_dir);
		system(szCmd);
		
		dbg_printf(DBG_DATA_LOG_FUNC, "%s() : %s \n", __func__, szCmd);
		Dummy_File_Make();
	}

#endif		/////////////////////////////////////////////////

	if ( access(data_dir, R_OK ) != 0) {
		char szCmd[128];
		sprintf(szCmd, "mkdir %s", data_dir);
		system(szCmd);

		dbg_printf(DBG_DATA_LOG_FUNC, "%s() : %s \n", __func__, szCmd);
	}
	else {
		int i, file_list_size;
		char file_list[1024 * 10];
		char file_name[512];
		int file_name_length = 0;
		
		m_file_count = 0;
		m_file_serial_no = 0;
		
		file_list_size = SB_Cat("ls -1 " DEF_DATA_LOG_DIR "\n", file_list, sizeof(file_list));
		
		dbg_printf(DBG_DATA_LOG_FUNC, " %d \r\n%s\r\n", file_list_size, file_list);	
		
		for(i=0; i < file_list_size; i++){
			char tmp = file_list[i];
			if (tmp == '\0')
				break;
			else
				file_name[file_name_length++] = tmp;
				
			if( tmp == '\r' || tmp == '\n'){
				file_name[file_name_length] = 0;

				dbg_printf(DBG_DATA_LOG_FUNC, "%d		%s\r\n", m_file_count, file_name);	
				
				if(file_name_length > 1){
					struct tm file_tm_t;
					u32 file_no = 0;
					memset((void *)&file_tm_t, 0, sizeof(file_tm_t));
					
					if(sscanf(file_name, DEF_DATA_LOG_FILE_FMT, &file_tm_t.tm_year, &file_tm_t.tm_mon, &file_tm_t.tm_mday, &file_no)){

						dbg_printf(DBG_DATA_LOG_FUNC, "%s() : %s (%04d-%02d-%02d)\r\n", __func__, file_name, file_tm_t.tm_year,  file_tm_t.tm_mon, file_tm_t.tm_mday);
						
						file_tm_t.tm_year -= 1900;
						file_tm_t.tm_mon -= 1;

						if(m_file_count == 0){
							t_oldest_log_file_no = file_no;
							strcpy(oldest_file, file_name);
						}
						else if( file_no < t_oldest_log_file_no){
							t_oldest_log_file_no = file_no;				
							strcpy(oldest_file, file_name);
						}
						
						if(file_no > m_file_serial_no){
							m_file_serial_no = file_no;
							last_log_file_tm_t = file_tm_t;
						}
						
						m_file_count++;
					}
					
				}
				
				file_name_length = 0;
			}
		}
#ifdef Horiba

#else
		if(m_file_count >= DATALOG_FILE_MAX_COUNT){
			char szCmd[512];

			dbg_printf(DBG_DATA_LOG_FUNC, "%s() : log file count = %d, Oldest log file : %s \r\n", __func__, m_file_count, oldest_file);
			
			sprintf(szCmd, "rm %s/%s", DEF_DATA_LOG_DIR, oldest_file );
			system(szCmd);
			dbg_printf(DBG_DATA_LOG_WRN, "%s() : Deleted oldest log file : %s \r\n", __func__, oldest_file);
			m_file_count--;
		}
#endif
	}

	
	localtime_r(&time, &tm_t);

	dbg_printf(DBG_DATA_LOG_FUNC, "%s() : %d (%04d-%02d-%02d : %04d-%02d-%02d)\r\n", __func__, m_file_serial_no, tm_t.tm_year,  tm_t.tm_mon, tm_t.tm_mday, last_log_file_tm_t.tm_year,  last_log_file_tm_t.tm_mon, last_log_file_tm_t.tm_mday);
	
	if(tm_t.tm_year != last_log_file_tm_t.tm_year || tm_t.tm_mon != last_log_file_tm_t.tm_mon || tm_t.tm_mday != last_log_file_tm_t.tm_mday){
		m_file_serial_no++;
		dbg_printf(DBG_DATA_LOG_FUNC, "%s() : %d \r\n", __func__,  m_file_serial_no);
	}
	
	sprintf(data_path, DEF_DATA_LOG_PATH_FMT, tm_t.tm_year + 1900, tm_t.tm_mon + 1, tm_t.tm_mday, m_file_serial_no);
	
#ifdef Horiba	/////////////////////////////////////test
	if ( 0 != access(data_path, R_OK )) {

		if(m_file_count >= DATALOG_FILE_MAX_COUNT){	/*&& t_oldest_log_file_no != 0xffffffff*/
			char mvCmd[256];
			char *ptr = strchr(oldest_file, '\n');

			if(ptr != NULL)
				ptr[0] = 0;

			sprintf(mvCmd, "mv %s/%s %s", DEF_DATA_LOG_DIR, oldest_file, data_path);
			system(mvCmd);
			dbg_printf(DBG_DATA_LOG_WRN, "%s() : Rename oldest[%s] \r\n", __func__, mvCmd);	
		}
		else
			Dummy_File_Move(data_path);	
		
		if( 0 != access(data_path, R_OK ))
			m_fp = fopen(data_path, "wb");
		else
			m_fp = fopen(data_path, "rb+");
		
		Horiba_Header_Creation(time);
		m_file_count++;
	
	}
#else	/////////////////////////////////////standard
	if ( 0 != access(data_path, R_OK ) ) {
		m_fp = fopen(data_path, "wb");
		m_file_count++;
	}
#endif
	else
		m_fp = fopen(data_path, "rb+");

	if(!m_fp) {
		dbg_printf(DBG_DATA_LOG_ERR, "%s open failed: %d(%s)\n", data_path, errno, strerror(errno));
		return 0;
	}

	int ret;
	u32 size = 64 * 1024;
	char* dummy = new char[  size ];
	
	fseek( m_fp, 0, SEEK_END );
	length = ftell( m_fp );
	fseek( m_fp, 0, SEEK_SET );

	if(length < DATALOG_FILE_SIZE){
#if 0		
		memset((void *)dummy, 0xff, size);
		fseek(m_fp, DATALOG_FILE_SIZE - size , SEEK_SET);
		fwrite((void *)dummy, 1, size, m_fp);
#else
		u32 i;
		memset((void *)dummy, 0x00, size);
		for( i = 0 ; i < (DATALOG_FILE_SIZE /size) ; i++) {
			fwrite((void *)dummy, 1, size, m_fp);
		}
#endif
		fseek( m_fp, 0, SEEK_SET );

		dbg_printf(DBG_DATA_LOG_FUNC, "%s file created( %d : %d KB).\n", data_path, DATALOG_FILE_SIZE / 1024, sysfs_getsize(data_path)/1024);

		/*
		FWVersion=VRHD/0.0.22
		SerialNumber=2020121234
		Date=2020-12-09
		Time,OperatingTime,GpsConnectionState,GpsSignalState,Latitude,Longitude,Speed(Km/h),Distance(Km),RPM,GS_Event,GS_X,GS_Y,GS_Z,Brake,Winker_L,Winker_R,InputTrigger1,InputTrigger2,Volt
		-------------------------------------
		오사카 가스
		H01,FWVersion,"0.0.25"
		H02,SerialNumber,"0"
		H03,Date,"2021-02-03"
		H04,DriverCode,"12345678"
		D01,Time,OperatingTime,GpsConnectionState,GpsSignalState,Latitude,Longitude,Speed(Km/h),Distance(Km),RPM,GS_Event,GS_X,GS_Y,GS_Z,Brake,Winker_L,Winker_R,InputTrigger1,InputTrigger2,Volt,cause

		Cause : Event Cause Category (1: No Event Occurrence, 2: Rapid Acceleration, 3: Sudden Deceleration, 4: Sudden Handle, 5: Others)
		*/

		m_log_position = sprintf(dummy, "H01,FWVersion,\"%s\"\r\n", __FW_VERSION__ );
		m_log_position += sprintf(&dummy[m_log_position], "H02,SerialNumber,\"%u\"\r\n", m_serial_number );
		m_log_position += sprintf(&dummy[m_log_position], "H03,Date,\"%04d-%02d-%02d\"\r\n", tm_t.tm_year + 1900, tm_t.tm_mon + 1, tm_t.tm_mday );
		m_log_position += sprintf(&dummy[m_log_position], "H04,DriverCode,\"%s\"\r\n", m_driver_code );
#ifdef ENABLE_DATA_LOG_UNI
		m_log_position += sprintf(&dummy[m_log_position], "D01,Date,Time,GpsConnectionState,GpsSignalState,Latitude,Longitude,Speed(Km/h),Distance(Km),RPM,GS_Event,GS_X,GS_Y,GS_Z,Brake,Winker_L,Winker_R,InputTrigger1,InputTrigger2,Volt" );
#else
		m_log_position += sprintf(&dummy[m_log_position], "D01,Date,Time,OperatingTime,GpsConnectionState,GpsSignalState,Latitude,Longitude,Speed(Km/h),Distance(Km),RPM,GS_Event,GS_X,GS_Y,GS_Z,Brake,Winker_L,Winker_R,InputTrigger1,InputTrigger2,Volt,Cause,Difference" );
#endif
#ifdef DEF_SAFE_DRIVING_MONITORING
		m_log_position += sprintf(&dummy[m_log_position], ",Highway\r\n");
#else
		m_log_position += sprintf(&dummy[m_log_position], "\r\n");
#endif

		ret = fwrite((void *)dummy, 1, strlen(dummy) + 1, m_fp);

		dbg_printf(DBG_DATA_LOG_FUNC, "%s() :  %d (%d : %s) \n", __func__, m_log_position, ret, dummy);
		
	}
	else {
			m_log_position = 0;
			do {
				int i;
				ret =  fread( (void *)dummy, 1, size, m_fp );

				for(i = 0; i < ret; i++){
					if(dummy[i] == 0 || dummy[i] == 0xff){
						ret = 0;
						break;
					}
					m_log_position++;
				}
				
			}while(ret > 0);

////++{*************************
			size = strlen(LOG_MSG_POWER_OFF);
			if(m_log_position > size){
				
				fseek( m_fp, m_log_position - size, SEEK_SET );
				ret =  fread( (void *)dummy, 1, size + 1, m_fp );

				dbg_printf(DBG_DATA_LOG_FUNC, "%s() :  \"%s\" is the last log message.\n", __func__, dummy);
				if(!strcmp(LOG_MSG_POWER_OFF, dummy)){
					m_log_position -= size;
					dbg_printf(DBG_DATA_LOG_FUNC, "%s() :  Deleted \"%s\" log message.\n", __func__, dummy);
				}
			}			
////++}*************************
			
			dbg_printf(DBG_DATA_LOG_WRN, "%s saved. Use %d Byte out of %d Byte.\n", data_path, m_log_position, DATALOG_FILE_SIZE);
	}

	delete [] dummy;

	
	m_is_init = true;
	m_pre_log_time = time;
	
	dbg_printf(DBG_DATA_LOG_FUNC, "%s() : %d\n", __func__, m_is_init);
	return m_is_init;
}

int CDatalog::deinit(void)
{
	if(m_fp) {
		fclose(m_fp);
		m_fp = NULL;
	}

	m_log_position = 0;
	m_is_init = false;
	return 0;
}

int CDatalog::make_log_string(char *log_string, int size, LPST_DATA_LOG_ITEM pLog, struct tm &tm_t)
{
	int log_length = 0;

	/*
	Time,OperatingTime,GpsConnectionState,GpsSignalState,Latitude,Longitude,Speed(Km/h),Distance(Km),RPM,GS_Event,GS_X,GS_Y,GS_Z,Brake,Winker_L,Winker_R,InputTrigger1,InputTrigger2,Volt,cause
	15:04:15,20,1,1,37.576016,126.676922,100,0.03,1000,0,0.01,-0.02,1.02,0,0,0,0,0,14.4,1
	*/
#ifdef ENABLE_DATA_LOG_UNI
	log_length = sprintf(log_string, "D02,");
	log_length += sprintf(&log_string[log_length], "%04d-%02d-%02d,", tm_t.tm_year + 1900, tm_t.tm_mon + 1, tm_t.tm_mday );
	log_length += sprintf(&log_string[log_length], "%02d:%02d:%02d.%03d", tm_t.tm_hour, tm_t.tm_min, tm_t.tm_sec, pLog->milisec_time); //test
	log_length += sprintf(&log_string[log_length], ",%d,%d,%0.6f,%0.6f", pLog->GpsConnectionState, pLog->GpsSignalState, pLog->Latitude, pLog->Longitude); 
	log_length += sprintf(&log_string[log_length], ",%u,%0.2f,%u",pLog->Speed, pLog->Distance, pLog->RPM);
	log_length += sprintf(&log_string[log_length], ",%u,%0.2f,%0.2f,%0.2f", pLog->GS_Event, pLog->GS_X, pLog->GS_Y, pLog->GS_Z); 
	log_length += sprintf(&log_string[log_length], ",%u,%u,%u,%u,%u", pLog->Brake, pLog->Winker_L, pLog->Winker_R, pLog->InputTrigger1, pLog->InputTrigger2);
	log_length += sprintf(&log_string[log_length], ",%0.1f", pLog->Volt);
	//log_length += sprintf(&log_string[log_length], ",%u", pLog->Cause);
	//log_length += sprintf(&log_string[log_length], ",%u", pLog->Difference);
#ifdef DEF_SAFE_DRIVING_MONITORING	
	log_length += sprintf(&log_string[log_length], ",%u", pLog->Highway);
#endif
#else
	log_length = sprintf(log_string, "D02,");
	log_length += sprintf(&log_string[log_length], "%04d-%02d-%02d,", tm_t.tm_year + 1900, tm_t.tm_mon + 1, tm_t.tm_mday );
	log_length += sprintf(&log_string[log_length], "%02d:%02d:%02d", tm_t.tm_hour, tm_t.tm_min, tm_t.tm_sec); 
	log_length += sprintf(&log_string[log_length], ",%u", pLog->OperatingTime); 
	log_length += sprintf(&log_string[log_length], ",%d,%d,%0.6f,%0.6f", pLog->GpsConnectionState, pLog->GpsSignalState, pLog->Latitude, pLog->Longitude); 
	log_length += sprintf(&log_string[log_length], ",%u,%0.2f,%u",pLog->Speed, pLog->Distance, pLog->RPM);
	log_length += sprintf(&log_string[log_length], ",%u,%0.2f,%0.2f,%0.2f", pLog->GS_Event, pLog->GS_X, pLog->GS_Y, pLog->GS_Z); 
	log_length += sprintf(&log_string[log_length], ",%u,%u,%u,%u,%u", pLog->Brake, pLog->Winker_L, pLog->Winker_R, pLog->InputTrigger1, pLog->InputTrigger2);
	log_length += sprintf(&log_string[log_length], ",%0.1f", pLog->Volt);
	log_length += sprintf(&log_string[log_length], ",%u", pLog->Cause);
	log_length += sprintf(&log_string[log_length], ",%u", pLog->Difference);
#ifdef DEF_SAFE_DRIVING_MONITORING	
	log_length += sprintf(&log_string[log_length], ",%u", pLog->Highway);
#endif
#endif

	log_length += sprintf(&log_string[log_length], "\r\n"); //end

	dbg_printf(DBG_DATA_LOG_FUNC, "%s() :  %d (%s) \n", __func__, log_length, log_string);
	
	return log_length;
}

int CDatalog::write_log(void * log_string, int log_length)
{
	int ret = 0;
	if(m_fp){
		fseek(m_fp, m_log_position, SEEK_SET);

		ret = fwrite(log_string, 1, log_length + 1, m_fp);	
		
		
		if(ret == log_length + 1){
			m_log_position += log_length;
			dbg_printf(DBG_DATA_LOG_FUNC, "%s saved. Use %d KB out of %d KB.\n", __func__, m_log_position/1024, DATALOG_FILE_SIZE/1024);
		}
		else {
			dbg_printf(DBG_DATA_LOG_ERR, "%s write failed: %d(%s)\n", __func__, errno, strerror(errno));
		}
			
	}

	return ret;
}

#endif // end of ENABLE_DATA_LOG
