// pai_r_data.cpp: implementation of the CPai_r_data class.
//
//////////////////////////////////////////////////////////////////////
#include <stdarg.h>
#include "datools.h"
#include "daappconfigs.h"
#include "sysfs.h"

//#include "tinyxml.h"
#include "OasisLog.h"
#include "pai_r_data.h"

#define DBG_PAIR_FUNC 	1 // DBG_MSG
#define DBG_PAIR_ERR  		DBG_ERR
#define DBG_PAIR_WRN		DBG_WRN

#define DEF_SYSTEM_DIR 							"/mnt/extsd/System"
#define DEF_LOCATION_DATA_PATH 		"/mnt/extsd/System/Location_Data.bin"
#define DEF_LOCATION_IN_PATH 			"/mnt/extsd/System/Location_In.bin"
#define DEF_LOCATION_OUT_PATH 		"/mnt/extsd/System/Location_Out.bin"

CPai_r_data::CPai_r_data()
{
	m_auto_id_in = 1;
	m_auto_id_out = 1;
	m_location_queue_init = false;
}

CPai_r_data::~CPai_r_data()
{
	Location_queue_deinit();
}

int CPai_r_data::addLocation(PAI_R_LOCATION loc, bool is_event)
{
	int i;
	PAI_R_DATA data;
	int speed_count = m_pre_speed_list.size();
	
	data.location = loc;
	data.speed_list.clear();

	for(i=0; i < DEF_MAX_CAMERA_COUNT; i++){
		data.thumbnail_data[i] = NULL;
		data.thumbnail_size[i] = 0;
	}
	
	if(is_event && speed_count){
		ITER_SPEED iSpeed = m_pre_speed_list.end();
		//ITER_SPEED iSpeedstart = m_pre_speed_list.begin();

		iSpeed--;
		
		for( i = 0; i < speed_count; i++, iSpeed-- )
		{
			if(iSpeed->create_time < loc.create_time - 30) //pre recording 10 + 20 sec
				break;

			data.speed_list.push_front(*iSpeed);
		}
	}

	if(m_data_list[is_event].size() > PAI_R_DATA_MAX_COUNT){
		ITER_PAI_R_DATA iDataend = m_data_list[is_event].end();
		iDataend--;

		for(int i = 0; i < DEF_MAX_CAMERA_COUNT; i++){
			if(iDataend->thumbnail_size[i] && iDataend->thumbnail_data[i])
				delete [] iDataend->thumbnail_data[i];
		}
		
		m_data_list[is_event].pop_front();
		dbg_printf(DBG_PAIR_WRN, "%s(): data [%d] buffer full !!\r\n", __func__,  (int)is_event);
	}

	m_data_list[is_event].push_back(data);

	dbg_printf(DBG_PAIR_FUNC, "%s() : %d list %d\r\n", __func__, is_event, m_data_list[is_event].size());
	dbg_printf(DBG_PAIR_FUNC, "%s() : speed list count %d\r\n", __func__, data.speed_list.size());
	return 0;
}

int CPai_r_data::addSpeed(PAI_R_SPEED_CSV spd, bool is_event)
{	

	if(m_data_list[0].size()){
		if(m_data_list[0].back().speed_list.size() > 240) // 0.5 * 120
			m_data_list[0].back().speed_list.pop_front();
		
		m_data_list[0].back().speed_list.push_back(spd);
	}

	if(m_data_list[1].size() && is_event){
		if(m_data_list[1].back().speed_list.size() > 240) // 0.5 * 120
			m_data_list[1].back().speed_list.pop_front();
		
		m_data_list[1].back().speed_list.push_back(spd);
	}

	if(m_pre_speed_list.size() > 240) // 0.5 * 120
		m_pre_speed_list.pop_front();

	m_pre_speed_list.push_back(spd);

//	if(m_data_list[0].size())
//		dbg_printf(DBG_PAIR_FUNC, "%s() : %d\r\n", __func__, m_data_list[0].front().speed_list.size());
//

//	dbg_printf(DBG_PAIR_FUNC, "%s() : [e:%d] %d:%d\r\n", __func__, (int)is_event, m_data_list[0].size(), m_pre_speed_list.size());
	
	return 0;
}

int CPai_r_data::addThumbnail(u8 * data, size_t size, bool is_event)
{	

	if(m_data_list[is_event].size()){
		ITER_PAI_R_DATA iDataend = m_data_list[is_event].end();
		iDataend--;

		for(int i = 0; i < DEF_MAX_CAMERA_COUNT; i++){
			if(iDataend->thumbnail_size[i] ==0)
			{
				iDataend->thumbnail_data[i] = new u8[ size ];
				iDataend->thumbnail_size[i] = size;
				memcpy(iDataend->thumbnail_data[i], data, size);
				
				dbg_printf(DBG_PAIR_FUNC, "%s() : [e:%d] %d new %d\r\n", __func__, (int)is_event, i, size);
				break;
			}
		}
	}

	dbg_printf(DBG_PAIR_FUNC, "%s() : [e:%d] %d\r\n", __func__, (int)is_event, size);
	
	return 0;
}

int CPai_r_data::Location_queue_index_backup(void)
{
	const char * location_queue_in_fmt = "location queue in : %u\r\n";
	const char * location_queue_out_fmt = "location queue out : %u\r\n";
	
	sysfs_printf(DEF_LOCATION_IN_PATH, location_queue_in_fmt, m_auto_id_in);
	sysfs_printf(DEF_LOCATION_OUT_PATH, location_queue_out_fmt, m_auto_id_out);

	return 1;
}

int CPai_r_data::Location_queue_index_load(void)
{
	const char * location_queue_in_fmt = "location queue in : %u\r\n";
	const char * location_queue_out_fmt = "location queue out : %u\r\n";
	u32 queue_in_no = 1;
	u32 queue_out_no = 1;
	
	if(sysfs_scanf(DEF_LOCATION_IN_PATH, location_queue_in_fmt, &queue_in_no)){
		m_auto_id_in = queue_in_no;
	}
	else {
		m_auto_id_in = 1;

		sysfs_printf(DEF_LOCATION_IN_PATH, location_queue_in_fmt, m_auto_id_in);
	}

	if(sysfs_scanf(DEF_LOCATION_OUT_PATH, location_queue_out_fmt, &queue_out_no)){
		m_auto_id_out = queue_out_no;
	}
	else {
		m_auto_id_out = m_auto_id_in;

		sysfs_printf(DEF_LOCATION_OUT_PATH, location_queue_out_fmt, m_auto_id_out);
	}

	return 1;
}

int CPai_r_data::Location_queue_init(void)
{
	//Folder check
	int retry = 0;
	
	const char * system_path = DEF_SYSTEM_DIR;
	const char * data_path = DEF_LOCATION_DATA_PATH;
	const char * location_queue_in_fmt = "location queue in : %u\r\n";
	const char * location_queue_out_fmt = "location queue out : %u\r\n";
	
	char szCmd[256];
	u32 file_size;

	if(m_location_queue_init)
		return 1;
	
SYSTEM_INIT:

	dbg_printf(DBG_PAIR_FUNC, "%s() : %d \n", __func__, retry);
	
	if ( access(system_path, R_OK ) != 0) {
		sprintf(szCmd, "mkdir %s", DEF_SYSTEM_DIR);
		system(szCmd);

		dbg_printf(DBG_PAIR_FUNC, "%s() : %s \n", __func__, szCmd);
	}

	if(retry++ >= 3){
		dbg_printf(DBG_PAIR_FUNC, "%s() : error !\n", __func__);
		return 0;
	}
	
	if ( access(system_path, R_OK ) != 0) {
		dbg_printf(DBG_PAIR_ERR, "%s() : error! (mkdir %s)\r\n", __func__, DEF_SYSTEM_DIR);
		return 0;
	}

	if ( access(data_path, R_OK ) != 0) {
#if 0
			u32 size = LOACATION_QUEUE_MAX_COUNT * sizeof(PAI_R_BACKUP_DATA);
 
			//int fd = creat(data_path, 0644);
		    //ftruncate(fd, size);
		    //close(fd);

			sprintf(szCmd, "fallocate -l %d %s", size, data_path);
			system(szCmd);
			dbg_printf(DBG_PAIR_FUNC, "%s saved %d bytes.\n", data_path, size);
#else	
		u32 i;
		FILE *fp = fopen(data_path, "wb");
		dbg_printf(DBG_PAIR_FUNC, "%s fopen %d\n", data_path, fp);
		
		if(fp) {
			u32 size = 20*1024; 
			//u32 size = sizeof(PAI_R_BACKUP_DATA);
			//char* dumy = new char[  size ];
			char dumy[20 * 1024];
			memset((void *)&dumy, 0xff, size);
			
			for(i=0; i < /*LOACATION_QUEUE_MAX_COUNT*/1 * sizeof(PAI_R_BACKUP_DATA); i+=size){
				fwrite((void *)&dumy, 1, size, fp);
			}

			//delete [] dumy;

			fclose(fp);
			dbg_printf(DBG_PAIR_FUNC, "%s saved %d bytes.\n", data_path, i);
		} else {
			dbg_printf(DBG_PAIR_ERR, "%s creation failed: %d(%s)\n", data_path, errno, strerror(errno));
		}
	#endif
	
		m_auto_id_out = m_auto_id_in = 1;
		Location_queue_index_backup();

		goto SYSTEM_INIT;
	}
	else {
		file_size = sysfs_getsize(data_path);

		if(file_size != LOACATION_QUEUE_MAX_COUNT * sizeof(PAI_R_BACKUP_DATA) &&  file_size < (m_auto_id_in & (LOACATION_QUEUE_MAX_COUNT-1)) * sizeof(PAI_R_BACKUP_DATA)){
			//file error			
			dbg_printf(DBG_PAIR_FUNC, "%s file size %u bytes. error! %d \n", data_path, file_size, retry);

			//sprintf(szCmd, "rm %s", data_path);
			//system(szCmd);

			sprintf(szCmd, "rm -rf %s", system_path);
			system(szCmd);

			goto SYSTEM_INIT;
		}

		dbg_printf(DBG_PAIR_FUNC, "%s file size %u bytes.\n", data_path, file_size);
		
		Location_queue_index_load();
	}
	m_location_queue_init = true;

	dbg_printf(DBG_PAIR_FUNC, "%s() : queue in : %u, out : %u, count : %u \r\n", __func__, m_auto_id_in, m_auto_id_out, Location_queue_count());
	return 1;
}


int CPai_r_data::Location_queue_deinit(void)
{
	if(m_location_queue_init == false)
		return 0;
	
//	const char * system_path = DEF_SYSTEM_DIR;

	if(!m_location_queue_init){
		return 0;
	}

	u32 count = Location_queue_count();

	Location_queue_index_backup();
	
	m_location_queue_init = false;

	dbg_printf(DBG_PAIR_FUNC, "%s() : queue in : %u, out : %u, count : %u \r\n", __func__, m_auto_id_in, m_auto_id_out, count);
	return 1;
}

int CPai_r_data::Location_queue_count(void){ 
	if(!m_location_queue_init){
		if(Location_queue_init() == 0)
			return 0;
	}
	
	return m_auto_id_in - m_auto_id_out; 
}

int CPai_r_data::Location_push_back(PAI_R_BACKUP_DATA *pLoc)
{
	if(!m_location_queue_init){
		if(Location_queue_init() == 0)
			return 0;
	}
	u32 in_id = (m_auto_id_in & (LOACATION_QUEUE_MAX_COUNT-1));
	const char * data_path = DEF_LOCATION_DATA_PATH;
	FILE *fp = fopen(data_path, "rb+");

	if(fp){			
		m_auto_id_in++;
		if(Location_queue_count() >= LOACATION_QUEUE_MAX_COUNT){
			m_auto_id_out = m_auto_id_in - (LOACATION_QUEUE_MAX_COUNT - 1);
			dbg_printf(DBG_PAIR_FUNC, "%s() : queue full! : deleted data. \r\n", __func__);
		}
		
		fseek(fp, in_id * sizeof(PAI_R_BACKUP_DATA),SEEK_SET);
		fwrite((void *)pLoc, 1, sizeof(PAI_R_BACKUP_DATA), fp);				

		fclose(fp);

		//dprintf(DBG_PAIR_FUNC, "%s() : size : %d ( 0x%08x ...)\n", __func__, pLoc->location.thumbnail.size[0], *((int *)&pLoc->location.thumbnail.data[0][0]));
		
		dbg_printf(DBG_PAIR_FUNC, "%s push %d (size : %d) \n", data_path, in_id, sizeof(PAI_R_BACKUP_DATA));
	} else {
		DLOG(DLOG_ERROR, "%s creation failed: %d(%s)\n", data_path, errno, strerror(errno));
	}
	return in_id;
}

int CPai_r_data::Location_pop_front(PAI_R_BACKUP_DATA *pLoc)
{
	return Location_pop(pLoc, 0);
}

int CPai_r_data::Location_pop(PAI_R_BACKUP_DATA *pLoc, u32 auto_id)
{
	if(!m_location_queue_init){
		if(Location_queue_init() == 0)
			return 0;
	}

	int data_count = Location_queue_count();
	u32 out_id = 0;

	if(auto_id && auto_id < m_auto_id_in){
		out_id = (auto_id & (LOACATION_QUEUE_MAX_COUNT-1));
	}
	else if(data_count){
		out_id = (m_auto_id_out & (LOACATION_QUEUE_MAX_COUNT-1));
		m_auto_id_out++;
	}
	else {
		return 0;
	}
	
	const char * data_path = DEF_LOCATION_DATA_PATH;
	FILE *fp = fopen(data_path, "rb");

	if(fp){
		fseek(fp, out_id * sizeof(PAI_R_BACKUP_DATA),SEEK_SET);	
		int ret =  fread( (void *)pLoc, 1, sizeof(PAI_R_BACKUP_DATA), fp );
		if (ret != sizeof(PAI_R_BACKUP_DATA) ) {
			DLOG(DLOG_ERROR, "%s read failed: %d(%s) , ret = %d : %d\n", data_path, errno, strerror(errno), ret, sizeof(PAI_R_BACKUP_DATA));
		}

		fclose(fp);
		dbg_printf(DBG_PAIR_FUNC, "%s pop %d \n", data_path, out_id);
	} else {
		DLOG(DLOG_ERROR, "%s open failed: %d(%s)\n", data_path, errno, strerror(errno));
		return 0;
	}
	
	return out_id;
}

int CPai_r_data::BackupLocation(bool is_event)
{
	int data_count = m_data_list[is_event].size();
	
	if(data_count)
	{
		PAI_R_BACKUP_DATA data;
		
		ITER_PAI_R_DATA iData = m_data_list[is_event].begin();

		data.upload_time = 0;
		memset(data.file_hash, 0, sizeof(data.file_hash));
		
		data.location.autoid = m_auto_id_in;	//로컬 동영상 자동 ID
		strcpy(data.location.file_path, iData->location.file_path); //" /mnt/extsd/NORMAR/20001130_152107_I2.avi"
		data.location.move_filesize= iData->location.move_filesize; //KByte

		for( int j=0 ; j < DEF_MAX_CAMERA_COUNT; j++){
			if(iData->thumbnail_size[j] && iData->thumbnail_data[j]) {
				if(iData->thumbnail_size[j] < DEF_MAX_THUMBNAIL_DATA_SIZE)
					memcpy((void *)data.location.thumbnail.data[j], (void *)iData->thumbnail_data[j], iData->thumbnail_size[j]); // 360 x 180 jpeg
				else
					dbg_printf(DBG_PAIR_ERR, "%s() : Error!! thumbnail buffer size.\r\n", __func__);
				
				dbg_printf(DBG_PAIR_FUNC, "%s() : [e:%d] delete %d ( 0x%08x ...)\r\n", __func__, (int)is_event, iData->thumbnail_size[j], *((int *)&iData->thumbnail_data[j][0]));
				delete [] iData->thumbnail_data[j];
			}
				
			data.location.thumbnail.size[j]= iData->thumbnail_size[j];
		}
	
		data.location.latitude = iData->location.latitude;
		data.location.logitude = iData->location.logitude;
		data.location.accurate = iData->location.accurate;
		data.location.direction = iData->location.direction; //0~359
		data.location.altitude = iData->location.altitude;

		data.location.create_time_ms = iData->location.create_time_ms;


		memset((void *)data.speed_list, 0, sizeof(data.speed_list));
		int speed_count = iData->speed_list.size();
			
		if(speed_count){
			ITER_SPEED iSpeed = iData->speed_list.begin();
			//ITER_SPEED iSpeedend = iData.speed_list.end();

			for( int j=0 ; j < speed_count; j++, iSpeed++ )
			{
				if(j < DEF_BACKUP_SPEED_LIST_MAX_COUNT)	{
					data.speed_list[j].create_time = iSpeed->create_time;
					data.speed_list[j].create_time_ms = iSpeed->create_time_ms;
				
					data.speed_list[j].speed = iSpeed->speed;
					data.speed_list[j].distance = iSpeed->distance;
				}				
			}

			iData->speed_list.clear();
		}

		
		//m_data_list[is_event].pop_back();
		//m_data_list[is_event].pop_front();
		m_data_list[is_event].erase(iData);

		dbg_printf(DBG_PAIR_FUNC , "%s() : %d \r\n", __func__, m_auto_id_in);
		
		Location_push_back(&data);

		return 1;
	}
	
	return 0;
}

int CPai_r_data::BackupLocationGetJson(LOCATION_INFO &loc_info)
{
	int data_count = Location_queue_count();
	
	if(data_count)
	{
		int i = 0;
		
		loc_info.speed = std::string("\"create_datetime\",\"loc_info.speed\",\"distance\"\r\n");
		loc_info.loc = std::string("[");

		for(i=0 ; i < data_count; i++)
		{			
			static PAI_R_BACKUP_DATA loc;
			if(!Location_pop_front(&loc))
				break;

			const char * file = strrchr(loc.location.file_path, '/');
	  		file++;

			time_t create_time = recording_time_string_to_time(file);
			int is_event = 0;
			if(strchr(file, REC_FILE_TYPE_EVENT))
				is_event = 1;				
			
			if(i == 0) {
				loc_info.create_time = create_time;
				loc_info.file_name = std::string(file);
				loc_info.file_auto_id = loc.location.autoid;
				loc_info.is_event = is_event ? true : false;
			}
			else{
				if(is_event)
					break;
				
				loc_info.loc.append(",");
			}
			
			loc_info.loc.append("{");
			loc_info.loc.append(format_string("\"driverecorder_autoid\":%u,", loc.location.autoid/*m_auto_id_in.c_str()*/));
        	loc_info.loc.append(format_string("\"create_datetime\":\"%s.%03u\",", make_time_string(create_time).c_str(),  (u32)loc.location.create_time_ms));
        	loc_info.loc.append(format_string("\"movie_filesize\":%u,", sysfs_getsize(loc.location.file_path)  / 1024 /*m_backup_data.location.move_filesize*/));
        	loc_info.loc.append(format_string("\"g_sensortype_id\":%d,", is_event));
			loc_info.loc.append(format_string("\"latitude\":%0.6f,", loc.location.latitude));
			loc_info.loc.append(format_string("\"longitude\":%0.6f,", loc.location.logitude));
			loc_info.loc.append(format_string("\"accurate\":%u,", loc.location.accurate));
			loc_info.loc.append(format_string("\"direction\":%u,", loc.location.direction));
			loc_info.loc.append(format_string("\"altitude\":%u", loc.location.altitude));
			loc_info.loc.append("}");

			for( int j=0 ; j < DEF_BACKUP_SPEED_LIST_MAX_COUNT; j++){
				if(loc.speed_list[j].create_time == 0)
					break;

				loc_info.speed.append(format_string("\"%s.%02d\",\"%d\",\"%0.3f\"\r\n", \
					make_time_string(loc.speed_list[j].create_time).c_str(), loc.speed_list[j].create_time_ms/10, \
					loc.speed_list[j].speed, \
					loc.speed_list[j].distance));		
			}
		}

		loc_info.loc.append("]");
		//dbg_printf(DBG_PAIR_FUNC , "%s \r\n", loc_info.loc.c_str());
		//loc_info.loc = std::string("[{\"driverecorder_autoid\":\"58\",\"create_datetime\":\"2019-09-08 00:08:00.598\",\"movie_filesize\":6907955,\"g_sensortype_id\":0,\"latitude\":34.668555,\"longitude\":135.518744,\"accurate\":10,\"direction\":90,\"altitude\":6}]");
		dbg_printf(DBG_PAIR_FUNC , "%s \r\n", loc_info.file_name.c_str());
		dbg_printf(DBG_PAIR_FUNC , "%s \r\n", loc_info.loc.c_str());
		dbg_printf(DBG_PAIR_FUNC , "%s \r\n", loc_info.speed.c_str());
		
		return 1;
	}

	return 0;
}


int CPai_r_data::getLastLocation_json(LOCATION_INFO &loc_info, bool is_event)
{
	int data_count = m_data_list[is_event].size();
	
	if(data_count)
	{
		int i = 0;
		
		loc_info.speed = std::string("\"create_datetime\",\"loc_info.speed\",\"distance\"\r\n");
		loc_info.loc = std::string("[");
		
		ITER_PAI_R_DATA iData = m_data_list[is_event].begin();
		//ITER_PAI_R_DATA iDataend = m_data_list[is_event].end();
		//iData--;
		
		//for(i=0 ; i < data_count; i++, iData++)
		{			
			if(i == 0) {
				loc_info.create_time = iData->location.create_time;
				loc_info.file_name = std::string(iData->location.file_path);
				loc_info.is_event = is_event;
			}
			else
				loc_info.loc.append(",");
			
			loc_info.loc.append("{");
			loc_info.loc.append(format_string("\"driverecorder_autoid\":%d,", m_auto_id_in /*m_auto_id_in.c_str()*/));
        	loc_info.loc.append(format_string("\"create_datetime\":\"%s.%u\",", make_time_string(iData->location.create_time).c_str(),  (u32)iData->location.create_time_ms));
        	loc_info.loc.append(format_string("\"movie_filesize\":%u,", sysfs_getsize(iData->location.file_path)  / 1024 /*iData->location.move_filesize*/));
        	loc_info.loc.append(format_string("\"g_sensortype_id\":%d,", (int)is_event));
			loc_info.loc.append(format_string("\"latitude\":%0.6f,", iData->location.latitude));
			loc_info.loc.append(format_string("\"longitude\":%0.6f,", iData->location.logitude));
			loc_info.loc.append(format_string("\"accurate\":%u,", iData->location.accurate));
			loc_info.loc.append(format_string("\"direction\":%u,", iData->location.direction));
			loc_info.loc.append(format_string("\"altitude\":%u", iData->location.altitude));
			loc_info.loc.append("}");
			
			int speed_count = iData->speed_list.size();
			
			if(speed_count){
				ITER_SPEED iSpeed = iData->speed_list.begin();
				//ITER_SPEED iSpeedend = iData.speed_list.end();

				for( int j=0 ; j < speed_count; j++, iSpeed++ )
				{
					//"create_datetime","loc_info.speed","distance"
					//"2019-01-01 11:22:33.12","50","0.00694"
					loc_info.speed.append(format_string("\"%s.%02d\",\"%d\",\"%0.3f\"\r\n", \
					make_time_string(iSpeed->create_time).c_str(), iSpeed->create_time_ms/10, \
					iSpeed->speed, \
					iSpeed->distance));					
				}

				iData->speed_list.clear();
			}

			for( int j=0 ; j < DEF_MAX_CAMERA_COUNT; j++){
				if(iData->thumbnail_size[j] && iData->thumbnail_data[j]) {
					dbg_printf(DBG_PAIR_FUNC, "%s() : [e:%d] delete %d\r\n", __func__, (int)is_event, iData->thumbnail_size[j]);
					delete [] iData->thumbnail_data[j];
				}
			}
			
			//m_data_list[is_event].pop_back();
			//m_data_list[is_event].pop_front();
			m_data_list[is_event].erase(iData);
		}

		loc_info.loc.append("]");
		//dbg_printf(DBG_PAIR_FUNC , "%s \r\n", loc_info.loc.c_str());
		//loc_info.loc = std::string("[{\"driverecorder_autoid\":\"58\",\"create_datetime\":\"2019-09-08 00:08:00.598\",\"movie_filesize\":6907955,\"g_sensortype_id\":0,\"latitude\":34.668555,\"longitude\":135.518744,\"accurate\":10,\"direction\":90,\"altitude\":6}]");
		dbg_printf(DBG_PAIR_FUNC , "%s \r\n", loc_info.file_name.c_str());
		dbg_printf(DBG_PAIR_FUNC , "%s \r\n", loc_info.loc.c_str());
		dbg_printf(DBG_PAIR_FUNC , "%s \r\n", loc_info.speed.c_str());
		
		return 1;
	}

	return 0;
}


/////////////////////////////////////////
