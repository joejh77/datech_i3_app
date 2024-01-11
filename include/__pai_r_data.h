
#if !defined(PAI_R_DATA_H)
#define PAI_R_DATA_H

#include <string>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "datypes.h"
#include "daversion.h"

#define KBYTE(k)		(k*1024)

#define LOCATION_MAX_SIZE		KBYTE(35)	//60 interval �� 24�ð�
#define SPEED_CSV_MAX_SIZE		KBYTE(2000) //0.5 interval �� 24�ð� 

/*
[
    {
        "driverecorder_autoid": 12345,
        "create_datetime": "2020-02-20 20:20:20",
        "movie_filesize": 10994320,
        "g_sensortype_id": 0,
        "latitude": 36.5630188,
        "logitude": 136.6640123,
        "accurate": 10,
        "direction": 0,
        "altitude": 5
    },
    {
        "driverecorder_autoid": 22345,
        "create_datetime": "2020-02-20 20:20:20",
        "movie_filesize": 4702863,
        "g_sensortype_id": 1,
        "latitude": 36.5630188,
        "logitude": 136.6640123,
        "accurate": 10,
        "direction": 0,
        "altitude": 5
    }
]
```
*/
typedef enum{
	eGsensorTypeID_Normal = 0,
	eGsensorTypeID_GEvent,
	eGsensorTypeID_End
}eGsensorTypeID;

typedef struct {
	char 	file_path[64];
	char 	file_type; // I, E ..
	u8		channel_count; // 1 or 2
	u16		create_time_ms;
	time_t 	create_time;
	size_t 	move_filesize; //Byte
	double	latitude;
	double	logitude;
	u16		accurate;
	u16		direction; //0~359
	u16		altitude;
}PAI_R_LOCATION;

#define DEF_MAX_CAMERA_COUNT 2
#define DEF_MAX_THUMBNAIL_DATA_SIZE (20 * 1024 + 50168)
typedef struct {
	u8		data[DEF_MAX_CAMERA_COUNT][DEF_MAX_THUMBNAIL_DATA_SIZE]; // 320 x 180 jpeg
	size_t 	size[DEF_MAX_CAMERA_COUNT]; //byte
}THUMBNAIL;

typedef struct {
	u32		autoid;	//���� ������ �ڵ� ID
	char 	file_path[64]; //" /mnt/extsd/NORMAR/20001130_152107_I2.avi"
	size_t 	move_filesize; //Byte
	THUMBNAIL thumbnail;
	
	double	latitude;
	double	logitude;
	u16		accurate;
	u16		direction; //0~359
	u16		altitude;

	u16		create_time_ms;
}PAI_R_LOCATION_DATA;

/*
CSV�� ���ϸ��� speed(������:���� ������ �ڵ�ID).csv�� �Ѵ�. ��:speed1.csv => zip���� ����
"create_datetime","speed","distance"
"2019-01-01 11:22:33.12","50","0.00694"
"2019-01-01 11:22:33.62","50","0.00694"
"2019-01-01 11:22:34.12","50","0.00694"
*/
typedef struct {
	time_t 	create_time;
	u16		create_time_ms;
	
	u16 		speed;
	double	distance;
}PAI_R_SPEED_CSV;

#define DEF_BACKUP_SPEED_LIST_MAX_COUNT		120
//struct size 102400
typedef struct {
	time_t 	upload_time;
	char 	file_hash[36]; //md5
	
	PAI_R_LOCATION_DATA		location;
	PAI_R_SPEED_CSV			speed_list[DEF_BACKUP_SPEED_LIST_MAX_COUNT];
}PAI_R_BACKUP_DATA;

typedef std::list<PAI_R_LOCATION>								LOCATION_POOL;
typedef LOCATION_POOL::iterator								ITER_LOCATION;

typedef std::list<PAI_R_SPEED_CSV>							SPEED_POOL;
typedef SPEED_POOL::iterator									ITER_SPEED;

typedef struct {
	u8		*thumbnail_data[DEF_MAX_CAMERA_COUNT]; // 320 x 180 jpeg
	size_t 	thumbnail_size[DEF_MAX_CAMERA_COUNT]; //KByte
	
	PAI_R_LOCATION	location;
	SPEED_POOL		speed_list;
}PAI_R_DATA;

typedef std::list<PAI_R_DATA>									PAI_R_DATA_POOL;
typedef PAI_R_DATA_POOL::iterator								ITER_PAI_R_DATA;

#define PAI_R_DATA_MAX_COUNT		((LOCATION_MAX_SIZE + SPEED_CSV_MAX_SIZE) / (sizeof(PAI_R_LOCATION) + sizeof(PAI_R_SPEED_CSV)))

typedef struct {
	bool 	is_event;
	time_t 	create_time;
	u32 		file_auto_id;
	
	std::string loc;
	std::string speed;
	std::string file_name;
}LOCATION_INFO;

class CPai_r_data 
{
public:
	CPai_r_data();
	virtual ~CPai_r_data();
	
	PAI_R_DATA_POOL m_data_list[2]; // normal, event
	SPEED_POOL		m_pre_speed_list;
	
	u32 		m_auto_id_in; // ���� ������ �ڵ� ID, queue in no
	u32 		m_auto_id_out; // ���� ������ �ڵ� ID, queue out no
	bool		m_location_queue_init;
	
public:
	

	int addLocation(PAI_R_LOCATION loc, bool is_event = false);
	int addSpeed(PAI_R_SPEED_CSV spd, bool is_event = false);
	int addThumbnail(u8 * data, size_t size, bool is_event);

	int Location_queue_index_backup();
	int Location_queue_index_load();
	int Location_queue_init(void);
	int Location_queue_deinit(void);
	int Location_queue_count(void);
	int Location_push_back(PAI_R_BACKUP_DATA *pLoc);
	int Location_pop_front(PAI_R_BACKUP_DATA *pLoc);
	int Location_pop(PAI_R_BACKUP_DATA *pLoc, u32 auto_id = 0);
	int BackupLocation(bool is_event = false);
	int BackupLocationGetJson(LOCATION_INFO &loc_info);
	int getLastLocation_json(LOCATION_INFO &loc_info, bool is_event = false);
protected:

};

/*
	iterator(�ݺ���)
	begin() : beginning iterator�� ��ȯ
	end() : end iterator�� ��ȯ
	�߰� �� ����
	push_front(element) : ����Ʈ ���� �տ� ���� �߰�
	pop_front() : ����Ʈ ���� �տ� ���� ����
	push_back(element) : ����Ʈ ���� �ڿ� ���� �߰�
	pop_back() : ����Ʈ ���� �ڿ� ���� ����
	insert(iterator, element) : iterator�� ����Ű�� �κ� ���ա��� ���Ҹ� �߰�
	erase(iterator) : iterator�� ����Ű�� �κп� ���Ҹ� ����
	��ȸ
	*iterator : iterator�� ����Ű�� ���ҿ� ����
	front() : ù��° ���Ҹ� ��ȯ
	back() : ������ ���Ҹ� ��ȯ
	��Ÿ
	empty() : ����Ʈ�� ��������� true �ƴϸ� false�� ��ȯ
	size() : ����Ʈ ���ҵ��� ���� ��ȯ

	m_GSensorList.push_back(data);
	
	CTextData::ITER_GSENSOR iGSensor;
	iGSensor = m_GSensorList.begin();

	CTextData::ITER_GSENSOR iGSensor = this->m_pSensorData->m_GSensorList.begin();
	CTextData::ITER_GSENSOR iGSensorend = this->m_pSensorData->m_GSensorList.end();

	m_GSensorList.size()
	
	for( ; iGSensor != iGSensorend; iGSensor++ )
	{
		if( dwCurrentTime <= iGSensor->m_cts)
			break;
	}
	
	m_GSensorList.clear();
*/
#endif // !defined(PAI_R_DATA_H)

