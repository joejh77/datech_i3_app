#ifndef _DA_VERSION_H_
#define _DA_VERSION_H_


#define	BUILD_MODEL_VRHD_PAI_R								1		//H:1080p, M:720p, L:720p/360p
#define	BUILD_MODEL_VRHD_SECURITY							2		//H:1080p, M:720p, L:720p/360p
#define	BUILD_MODEL_VRHD_SECURITY_REAR_1CH 					3
#define	BUILD_MODEL_VRHD_V1V2								4		//H:1080p, M:720p, L:720p/360p
#define	BUILD_MODEL_VRHD_220S								5		//AC200 삭제, Pulse, digital IO 삭제
#define BUILD_MODEL_VRHD_MMB_SERVER							6
#define BUILD_MODEL_VRHD_SECURITY_FUTABA					7

//#define	BUILD_MODEL 			BUILD_MODEL_VRHD_PAI_R
#define	BUILD_MODEL 			BUILD_MODEL_VRHD_MMB_SERVER			//MMB Server
//#define	BUILD_MODEL 			BUILD_MODEL_VRHD_SECURITY				//NV2HD, VRHD Firstview Standard
//#define	BUILD_MODEL 		BUILD_MODEL_VRHD_SECURITY_FUTABA				//NV2HD, VRHD Futaba safe monitoring
//#define	BUILD_MODEL 			BUILD_MODEL_VRHD_SECURITY_REAR_1CH		//Osaka Gas_1ch
//#define	BUILD_MODEL 			BUILD_MODEL_VRHD_V1V2
//#define	BUILD_MODEL 			BUILD_MODEL_VRHD_220S					//NV1HD Firstview Standard

#define USE_DA_PULSE  1

#define ENABLE_BOARD_CHECK	0
#define DEF_PULSE_DEBUG		1

#define DEF_BOARD_WITHOUT_DCDC		1 	// new board, without dcdc
#define DEF_SUB_MICOM_USE					1

#define DEF_PAI_R_SPD_INTERVAL	500

#if ENABLE_BOARD_CHECK
#define 	DA_MODEL_NAME		"DA200 S_CHK"
#elif (BUILD_MODEL == BUILD_MODEL_VRHD_SECURITY)
#define 	DA_MODEL_NAME		"DA220" // Model 별 Folder 적용 for update_221130
#elif (BUILD_MODEL == BUILD_MODEL_VRHD_220S)
#define 	DA_MODEL_NAME		"DA220S"
#elif (BUILD_MODEL == BUILD_MODEL_VRHD_SECURITY_REAR_1CH)
#define 	DA_MODEL_NAME		"DA300S"
#else
#define 	DA_MODEL_NAME		"DA200 S"
#endif

#ifdef HTTPD_EMBEDDED
#define DEF_DEBUG_SERIAL_LOG_FILE_SAVE 	1
#else
#define DEF_DEBUG_SERIAL_LOG_FILE_SAVE 	0
#endif



#if (BUILD_MODEL == BUILD_MODEL_VRHD_PAI_R)
#define __FW_VERSION__					"0.0.32"
#define	DA_FIRMWARE_VERSION				"VRHD/" __FW_VERSION__

#define ENABLE_AVI_SCRAMBLING 			0

#define DEF_PAI_R_DATA_SAVE

#define DEF_LOW_FRONT_720P
#define DEF_30FIX_FRAME					0
#define REAR_1080P 						0
#define REAR_NTSC						0
#define ENABLE_DATA_LOG	 				0

#define DEF_SAFE_DRIVING_MONITORING
#define DEF_OSAKAGAS_DATALOG
#define DEF_TEST_SERVER_USE		// LTE 모뎀 통신 test
#define DEF_USB_LTE_MODEM_USE

#define DEF_NO_SOUND_NETWORK_CHECK //항상 AP모드로 동작하는 도델에 사용

#elif (BUILD_MODEL == BUILD_MODEL_VRHD_MMB_SERVER)
#define __FW_VERSION_MAJOR__			"01"
#define __FW_VERSION_MINOR__			"03"
#define __FW_VERSION__ __FW_VERSION_MAJOR__ "." __FW_VERSION_MINOR__
#define	DA_FIRMWARE_VERSION				"VRHD/" __FW_VERSION__

#define DEF_VRSE_SECURITY_FILE_EXT		"fvfs"		//firstview file system
#define DEF_VRSE_FVFS_HEADER			"FVFS   CDR MOVIE"
#define ENABLE_AVI_SCRAMBLING 			1
#define AVI_SCRAMBLE_CODES    			"0x000000"

#define DEF_PAI_R_DATA_SAVE

#define DEF_LOW_FRONT_720P
#define DEF_30FIX_FRAME					0
#define REAR_1080P 						0
#define REAR_NTSC						0
#define ENABLE_DATA_LOG	 				1			

#define DEF_SAFE_DRIVING_MONITORING
#define DEF_OSAKAGAS_DATALOG
#define DEF_TEST_SERVER_USE		// LTE 모뎀 통신 test
#define DEF_USB_LTE_MODEM_USE

#define DEF_MMB_VER_2 	// Ryunni
//#define SOS_CHECK		// Joe	SOS BT check

#define DEF_MMB_SERVER
#define DEF_MMB_MAX_EVENTDATA_COUNT		1
#elif (BUILD_MODEL == BUILD_MODEL_VRHD_SECURITY)	//NV2HD, VRHD Standard
#define __FW_VERSION__					"1.3.4"
#define DA_FIRMWARE_VERSION				"VRSE/" __FW_VERSION__

#define DEF_VRSE_SECURITY_FILE_EXT		"fvfs"		//firstview file system
#define DEF_VRSE_FVFS_HEADER			"FVFS   CDR MOVIE"
#define ENABLE_AVI_SCRAMBLING 			1			//0 is avi file
//#define AVI_SCRAMBLE_CODES    			"0x6461746563680635b5695b47d888d1590934138834535c9f63b6907357e724eb"
#define AVI_SCRAMBLE_CODES    			"0x000000"

#define DEF_LOW_FRONT_720P
#define DEF_30FIX_FRAME					0
#define REAR_1080P 						0
#define REAR_NTSC						0  	// 0 is auto detect
#define ENABLE_DATA_LOG	 				1
#define ENABLE_AHD						//Master Updata RearCAM Default

#elif (BUILD_MODEL == BUILD_MODEL_VRHD_SECURITY_FUTABA || BUILD_MODEL == BUILD_MODEL_VRHD_SECURITY_REAR_1CH)	//Futaba safe monitoring
#define __FW_VERSION__					"2.0.0"

 #if BUILD_MODEL == BUILD_MODEL_VRHD_SECURITY_FUTABA
	#define DA_FIRMWARE_VERSION			"VRSE/" __FW_VERSION__
 #else
 	#define DA_FIRMWARE_VERSION			"VRSE/1CH_" __FW_VERSION__
	#define DEF_REAR_CAMERA_ONLY
 #endif

#define DEF_VRSE_SECURITY_FILE_EXT		"fvfs"		//firstview file system
#define DEF_VRSE_FVFS_HEADER			"FVFS   CDR MOVIE"
#define ENABLE_AVI_SCRAMBLING 			1			//0 is avi file
//#define AVI_SCRAMBLE_CODES    			"0x6461746563680635b5695b47d888d1590934138834535c9f63b6907357e724eb"
#define AVI_SCRAMBLE_CODES    			"0x000000"

#define DEF_LOW_FRONT_720P
#define DEF_30FIX_FRAME					0
#define REAR_1080P 						0
#define REAR_NTSC						0  // 0 is auto detect
#define ENABLE_DATA_LOG	 				1
#define ENABLE_AHD						//Master Updata RearCAM Default
/////////////////////////////////////////// Below for Futaba
//#define Horiba							// SD 동시사용
#define DEF_SAFE_DRIVING_MONITORING		//오사카 가스향에서 사용
#define DEF_OSAKAGAS_DATALOG
#define DEF_OSAKAGAS_USERDATA_1SEC		//Osaks Gas 1초에 1번 저장
//#define ENABLE_DATA_LOG_UNI				// 농공대학

#elif (BUILD_MODEL == BUILD_MODEL_VRHD_V1V2)
#define __FW_VERSION__					"0.0.27"
#define	DA_FIRMWARE_VERSION				"VRVX/" __FW_VERSION__

#define ENABLE_AVI_SCRAMBLING 			0

#define DEF_LOW_FRONT_720P
 #define DEF_30FIX_FRAME				0
#define REAR_1080P 						0
#define REAR_NTSC						0
#define ENABLE_DATA_LOG	 				1


#elif (BUILD_MODEL == BUILD_MODEL_VRHD_220S)
#define __FW_VERSION__					"1.3.0"

#define DA_FIRMWARE_VERSION				"VRSS/" __FW_VERSION__


#define DEF_VRSE_SECURITY_FILE_EXT		"fvfs"		//firstview file system
#define DEF_VRSE_FVFS_HEADER			"FVFS   CDR MOVIE"
#define ENABLE_AVI_SCRAMBLING 			1
//#define AVI_SCRAMBLE_CODES    			"0x6461746563680635b5695b47d888d1590934138834535c9f63b6907357e724eb"
#define AVI_SCRAMBLE_CODES    			"0x000000"

#define DEF_LOW_FRONT_720P
#define DEF_30FIX_FRAME					0
#define REAR_1080P 						0
#define REAR_NTSC						0  // 0 is auto detect
#define ENABLE_DATA_LOG	 				1


#define DEF_DO_NOT_USE_DIGITAL_IO

#undef USE_DA_PULSE
#define USE_DA_PULSE  					0
#endif


//////////////////////////////////////////////////////
//Revision Note
/*
VRHD 01.03_240611
 1) 급핸들 default = 50 으로 수정

VRSE 2.0.0_240611
 1) 급핸들 default = 50 으로 수정
 2) Futaba safe monotoring 버전 수정

VRSE 1.3.3_240206
 1) Horiba SD카드 동시 사용 
 2) TEMP Folder & tmp file 30개 생성
 3) Futaba 기능 적용

VRHD 01.02_240118
 1) MMB 표준사양(SOS 기능 삭제)
 2) setup.xml default 생성(CloudServerName, CloudServerPort, CloudServerPollingPort)

VRHD 02.02_231207
 1) SOS BT 동작 기존과 동일
 2) setup.xml default 항목추가(CloudServerName, CloudServerPort, CloudServerPollingPort)

VRHD 02.01
 1) SOS CHECK BOX_Input2 High 유지시 이벤트 발생 
 2) 나머지 이벤트는 서버 전송 안함
 3) Input1,2 Trigger Event 삭제

VRHD 01.01
 1) SKYEYEDMS 서버 통신기능 적용

VRSE1.3.1_1CH
 1) Futata 최신 일보 기능 적용

VRSE1.3.1
 1) 일반속도, 고속속도 로그 2초 딜레이 문제 수정
  => 이벤트 로그 기록 안되는 문제로 1초 딜레이로 수정

VRSE1.3.0/VRSS1.3.0
 1) 양산 버전
 2) SD_offs3.14.3 버전 적용

VRSE1.2.9/1CH_1.2.9
 1) 1CH_User Data 1초에 1번 저장
 2) SD_offs3.14.3 버전 적용

VRSE 1.2.8
 1) Log 15ea/1s => 농공대학

VRSE 1.2.7
 1) /tmp/offs.info file에서 result 값 확인 코드 추가
 
VRSE1.2.6
 1) Over Speed, Fast Speed log flag clear 안되는 문제 수정
 
VRSE1.2.5
 1) Over Speed, Fast Speed log flag 유지하도록 적용
 
VRSE1.2.4
 1) 일반속도, 고속속도 기능 추가

VRSE1.2.3 (NV2HD 양산)
 1) imx307 MCLK 주파수 시프트 74M -> 70M
 2) Day Cahnge : D09
 3) Drivercode 추가

VRSS1.2.1 (NV1HD 양산)
 1) imx307 MCLK 주파수 시프트 74M -> 70M
 

1.1.1 : 20220822
 1) OFFS 3.13.19 적용
 2) Pulse L, R, Brk ON에도 chattering 시간(200mS) 적용(기존에는 OFF시에만 적용됨)
 3) Pulse Input 1, Input 2 and Brk Signal Power Off시 변경되지 않도록 수정
 
0.0.29 : 20220514
 1) CloudServerSSL Config Add
 
1.0.8 : 20220411
 1) serial text code added(gps serial use)
 
1.0.7 : 20220328
 1) setup.xml file : do not used 8k fix size, 
  - 후타바에서 Microsoft edge에서 Open 가능 하게 해달라는 요청으로, 사용하지 않음
 2) Input 1, 2 Signal : 1초가 안 되는 Signal도 Log에 1로 한 번 기록하도록 수정
 
1.0.6 : 20220223
 1) GPS가 수신이 되지 않는 터널 등에서 GPS와 PULSE 속도 차(10.0 Km/h)가 발생하면 펄스 셋팅을 초기화 하는 문제 수정(GPS가 수신 되는 경우에만 실행)

1.0.5 : 20220222
 1) Speed 값으로 거리를 환산하는 방식 버그 수정(pulse 연결된 경우 Speed값으로 추가 누적)
 
1.0.4 : 20220217
 1) Speed 값으로 거리를 환산하는 방식은 100km 이상 누적 시 오차가 심하여 좌표 값으로 거리를 계산하도록 수정
 
1.0.3 : 20211104
 1) 오사카 가스향 로그기록 Cause value 필드 추가
 2) Speed limit test mode 적용
   (설정에서 Test mode 1이면 pulse reset 값을 자동으로 0으로 인식하도록 수정,  pulse reset 은 1로 두고 test mode만 1로하여 테스트)
 
1.0.2 : 20210810
 1) 오사카 가스향 로그기록 "D09" 재부팅 후에 삭제하는 코드 추가
 
1.0.1
 1) 오사카 가스향 로그기록 적용
 	- H01~04, D01~02 적용 및 Cause 학목 추가
 	- DriverCode 항목 추가 (sd root에 driver_code.txt file을 읽어 적용함)

 	- D09 코드 추가 20210719
 	
1.0.0
 1) 1ch 녹화 시 음성멘트 2회 출력되는 문제 수정
 2) default value 스피커 : 4, G센서 : 2 적용
 3) PulseReset 0(init completed), 1(need init), 2(manual mode) 적용 
 4) setup.xml에 ProductSerialNo 추가 
 	ex : <item value="2101000002" id="ProductSerialNo" />
 5) serial console root password 적용 "77797779"
 6) exfat로 포맷된 sd 삽입 시 리부팅 되는 문제 수정
 7) exp_delay_frame = 4, gain_delay_frame = 4  => 2로 수정
 8) VRSE (2채널용 펌웨어) 1.0.0에서 후방카메라 없이 1CH로 녹화 시 I 프레임이 많이 생기는 문제 수정

 9) sd카드 없이 부팅 시 간헐적으로 음성멘트 반복하는 문제 수정
 
0.0.26
 1) WiFi Module이 연결되지 않은 경우 주행로그에 거리기록이 안 되는 문제 수정
 2) 펄스 세팅 시 “딩동” 음성 출력
 3) 녹화 중에 후방 카메라 타입을 변경 시 화면오류 수정(재부팅 됨)
 4) 1CH 전용  모델 추가 (버전 표시 "VRSE/1CH_x.x.xx")
 	- 모든 VideoQuality에서 enable-dynamic-qp-range ON 함 (1CH이라 비트레이트를 높게 설정할 수 있고 또한 후방카메라는 샤프니스가 낮아 블록 노이즈가 덜 생기는 것으로 보임)
 	- File Size : VideoQuality High 48M, Middle 32M, Low 16M
 	* 전방카메라 없이 동작시킬 경우 Video 출력 화면에 문제가 있음(녹화 영상은 정상)

 5) WiFi SSID 관련 Unicode Type 문자 인식하도록 수정
 6) CloudServerName : "mirec.info" 적용
 	 CloudServerApiUrl : "/api_driverecorder/firstview/1/insert_driverecorder.php" 적용

 
0.0.25
 1) No85 updateTethering과 insertOperation에 Tel Number 추가
 2) No87 WiFi AP가 많은 환경에서 접속을 못하는 문제 수정 (최대 50개까지 동작하도록 수정)
 	기존엔 5개 미만에서만 정상 동작을 했습니다. 
 3) test mode에 Serial Number 표시 추가
 4) VideoQuality LOW : min-qp 34 적용
 5) gps 시간 설정 : 오차범위를 기존 +- 15 => +-2초로 수정
 6) VGA 카메라 시 MIDDLE과 HIGH에서 720p로 녹화되는 문제 수정
 7) 데이터 로그 : operating time 과 volt 값이 가끔 한 번씩 빠지는 문제와 volt값이 0v로 기록되는 문제 수정
 8) 펌웨어 업데이트 후 가끔 sd 초가화 안 되는 문제 수정
 
0.0.24
 1)oasis 4.2.10 적용
 	- I 프레임 연속으로 발생하는 문제 수정
 2) dynamic-qp-range (min-qp : H 27, M 27, L F27/R29(720p)/R23(NTSC), FPS : H 30, M 30, L 10/10)
 3) 화질 LOW 전방 720p / 후방 720카메라는 1280x720, NTSC 카메라는 640x360 적용
 4) data log add (하루에 1개 파일 생성, 최대 30개까지 생성 후 덮어쓰기 됨)

 5) 720p 후방카메라 화면 위치 보정(컬러는 기존 설정유지)
 6) NTSC 자동 감지( 카메라 타입 교체 시 전원 OFF / ON 해야 적용 됨, 부팅 시 자동 리셋 1번 진행)
 7) VideoQuality 
 	High 		: Front 1920x1080, 30FPS		Rear 1280x720, 30FPS
 	Middele	: Front 1280x720, 30FPS		Rear 1280x720, 30FPS
 	Low 			: Front	1280x720, 10FPS		Rear 1280x720, 10FPS
 	* Rear NTSC : High: 640x360 30F, Middle: 640x360 30F, Low 640x360 10FPS
 8) 전원 종료 멘트 녹음 되도록 수정
 
0.0.23
1) G-Sensor 감도 둔감하게(3단계 쉬프트) 변경, Z축은 더 둔감하게 수정
//2) disable dynamic-qp-range (min-qp : H 33, M 37, L 41)
2) disable dynamic-qp-range (min-qp : H 33, M 33, L 32, FPS : H 30, M 15, L 6/3) 
3) save-stream-data-in-header을 1로 설정( 0 설정은 뷰어에서 40초 이상 재생이 안 되는 문제가 발생)

0.0.22
 1) INPUT1, 2 Port로 50ms ~ 1000ms 동안만 High 신호가 입력되는 경우 EVENT 녹화(INPUT1 = "A", INPUT2 = "B")
 2) RPM 추가
 3) Speed Limit 추가 (20km/h 이상 시 High)
 4) RPM, Speed Limit 관련 setup.xm 설정 추가
 	<item value="4" id="EngineCylinders" />
    <item value="20" id="SpeedLimitValue" />
    <item value="TRUE" id="OSDSpeed" />
    <item value="TRUE" id="OSDRpm" />
 5) G-Sensor Event 감지기간 500 = > 200ms (속도 변화에 약간 둔감해짐)

 
 6) No64 서버에서 요청된 동영상 파일 다 업로드 하지 못한 경우 재기동 시 이어 업로드 하도록 수정
 7) No58 동일한 URL인 경우 재기동 시 driverecorder POST skip 하도록 수정
 8) No75 (다른 제품에 사용하던 SD카드를 연결한 경우)서버에서 요청한 동영상의 autoid를 POST하지 않는 문제 수정
 9) updateTethering 정보 10개까지 설정 및 동작하도록 수정

 // No74 뷰어 최소 후 파일 클릭으로 실행 후 최대화 시 화면 에러 수정 <== 뷰어에서 수정
 // No 78 뷰어 후방 화면 확대 후 최대화 시 화면 겹치는 문제 수정 <== 뷰어에서 수정
 // No79 Video Osd Speed와 Viewer Speed 표시에 차이가 발생하는 문제 수정 <== 뷰어에서 수정

 //20201027
 ) 파일확장자 ".fvfs" 적용 및 avi header 정보 변경 (SECURITY 모드 버전 정보는 "VRSE 0.0.22"로 적용 됨)
 ) VideoQuality : HIGH (80M, 1080p/720p), MIDDLE(40M, 720p/720p), LOW(24M, 720p/720p) 적용
 
0.0.21
 1) No.58 [A안]에서 url 시작을 "/driverecorder/..." 로 하지 않으면 오류가 발생하는 문제 수정
 	url 주소 유효성 검사에서 "/driverecorder" 문자를 확인하는 부분을 제거 하였습니다.

0.0.20
 1) No.46 문제 수정 ( 1Km 이상 주행, 10분 이상 연속 동작 등의 조건에서 발생 할 수 있는 버그가 발견되어 수정하였습니다.)
 2) N0.54 SD Card가 교체되어도 로컬동영상자동ID는 계속 누적 적용
 	- SD Card 교체 시 과거 녹화된 파일은 autoid가 맞지 않아 status 값은 90을 전송 (90:오류:블랙 박스에 해당 데이터는 더 이상 존재하지 않는다)

 3) N0.57 thumbnail 빠지는 문제 개선 (증산 재현이 안 되어 관련 부분 보완 프로그램을 추가함)
 4) No.58 문제 수정 (FW갱신으로 서버 변경 시 SD의 api_list 삭제), A안 적용
   	sd card setup.xml(값이 없으면 FW 적용 값 사용)
 	<item value="mar-i.com" id="CloudServerName" />
    <item value="/driverecorder/api_driverecorder/firstview/1/insert_driverecorder.php" id="CloudServerApiUrl" />
    <item value="80" id="CloudServerPort" />
 5) No.59 pai-r movie filesize Kbyte => byte 수정
 6) Event Recording 10 + 20 => 10 + 10으로 수정
 7) 전방 카메라 osd "[1]" 부분 삭제, 후방 카메라 osd "[2] 2020/10/15 19:07:31" 전체 삭제
 8) OSDSpeed config 추가 및 "TRUE" 시 전방 카메라 osd "000km/h" 표시
 
0.0.19
 1)PC viewer에서 G-Sensor와 GPS 정보표시가 안 되는 문제 수정

0.0.18
 1) GalaxyS10 사용 시 서버로부터 들어오는 Gzip type file 처리 코드 추가
 2) SD Card의 setup.xml의 SSID 반영하지 않도록 수정 (44이슈)
 3) insert_operation 응답 시 now_datetime VRHD에 적용(서버 시간으로 VRHD 시간을 맞춤)
 4) 20200815VRHD音?リスト.xlsx 문서의 2, 3번 내용 적용
 	2번. 음성 멘트 변경 "Wi-Fi 설정에 실패했습니다. Wi-Fi 설정 모드입니다" ==> "통신 오류, 네트워크 설정을 확인하십시오."
 	3번. API에서 드라 레코의 볼륨 변경 시 음계 조절 방식 적용(파일 : WAVE1 ~ WAVE5.wav)
 5) 장시간 에이징 시 메모리 누수 현상 수정
 6) G-Sensor 감도 수정 0.1 ~ 1.1g (default value 2 => 5 변경)
 
0.0.17
 1) pulse 입력 시 user data 표시가 40초정도만 되는 문제 수정 (avi-strd-size-max size 12000 => 48000으로 수정)
 2) Pai-r 경로 저장 시 UUID backup
 3) WiFi Module 연결 후 Aging 시 Segmentation fault 후 rebooting 문제 수정
 
0.0.16
 1) GPS 정보 저장 시 G$ Header 없이 저장되어 뷰어에서 GPS정보를 표시 못하는 문제 수정
 2) Pai-r 관련 데이터 저장용 파일 최대 사이즈로 파일 생성(기존 조금씩 저장하던 방식보다 안정적임)
 3) Pai-r 4-05 getUpdateLog POST API에 data_type 변수 추가
 4) Pai-r API의 insert_list_and_reques의 latitude, longitude value degree unit으로 수정 (ddmm.mmmm => dd.dddddd)
 5) 후방 카메라 AHD 1080p 방식 인식하도록 수정
 
0.0.15
 1) 카드 미삽입 경고음(3회 반복_5s 주기) 울리고 모든 LED Off하도록 수정
 2) power off 시 모든 LED off하도록 수정
 
0.0.14
 1) sub mcu code add
 2) API 4-8 getSystemLog str_day_ago, how_many_days_ago 사양 문서에 맞게 수정
 3) API 4-4 getMovieList의 반환 값 중 hash 값을 hash path로 변경
 
0.0.13
 1) getSystemLog API Json format 수정
 2) updateConfig의 Speaker Level 설정 시 "댕동" 사운드 출력
 3) 1080p 녹화 모드 썸네일 캡쳐이미지 480x270으로 수정
 4) API 501(insert driverecorder) 통신은 테더링 설정 후 최초 1회만 하도록 수정
 5) L,R 깜박이 On후 2초 체터링 적용
 6) VRHD 뒤집어 설치하는 경우 전방 카메라 180도 반전


0.0.12
 1) API 4-04 getMovieList 느린 응답 개선 및 일부 파일 md5 hash error 수정
  LOCATION_QUEUE_MAX_COUNT 2014 => 2048
 2) 주행 중엔 테더링 설정을 위해 AP모드로 전환하지 않도록 수정
 3) API 4-08 getSystemLog type, day_ago 적용
 4) front image sensor IMX307 하이페리온 튜닝 값 적용
 5) max jpg image capture size 640x360 => 1920x1080으로 수정
 6) API 4-10 updateConfig resolution_cam0, resolution_cam1 추가
 
0.0.11
 1) PAI-R HTTP Protocol : getSystemLog, getConfig, updateConfig added
 
0.0.10
 1) PAI-R HTTP Protocol : getSystem added
 
0.0.9
 1) PAI-R HTTP Protocol : getUpdateLog added
 
0.0.8
 1) PAI-R HTTP Protocol : Movie POST added
 
0.0.7
 1) PAI-R HTTP Protocol Add
 
0.0.6
 1) 리어카메라 제거후 다시 연결 시 2Ch 녹화로 전환 안 되는 문제 수정

0.0.5
 1) 1Ch Aging 시 2Ch 자동전환 문제 수정
 2) GPS User Data 추가 
 3) User Data space 부족하여 키움
 4) GPS Timezone 문제 수정
 
0.0.4
 1) sdk oasis-v2.2.29-offs-3.11.22 적용
 2) gps invalid signal 시 led 동작 bug 수정
 
0.0.3
 1) sdk oasis-v2.2.19-offs-3.11.14 적용

0.0.2
 1) led bug fixed
  	전원 인가 및 10초 이후 부팅 시와 이벤트 시 교차 점멸, ( 업데이트 시에는 동시 점멸 : 추후 적용 예정)
 2) Input1 : High 신호 입력될 동안 계속 ‘G’ 파일 생성 / Low 신호일 때 ‘I’ 파일 생성
    Input1, Input2 : 동시 High 신호 입력될 동안 계속 ‘H’ 파일 생성 
 3) SD 볼륨 Firstview -> SD 로 변경
 4) video output과 uart debug message에 pulse data 확인 가능하도록 text추가

 0.0.1
 1) led control add
 
0.0.0 Initial Version
*/

#endif // _DA_VERSION_H_
