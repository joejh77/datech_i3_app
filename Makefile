#soft-fp version
CC=/home/${USER}/workspace/project/oasis/buildroot-oasis/output/firstview_nor/host/bin/arm-linux-gnueabi-gcc
CXX=/home/${USER}/workspace/project/oasis/buildroot-oasis/output/firstview_nor/host/bin/arm-linux-gnueabi-g++
STRIP=/home/${USER}/workspace/V3_I3/buildroot-oasis/output/nor/host/bin/arm-linux-gnueabi-strip
#CXX=/home/${USER}/workspace/project/Oasis/tools/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-g++
#STRIP=/home/${USER}/workspace/Oasis/tools/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-strip
CXXFLAGS=-Wall -std=c++11 -march=armv7-a -mfloat-abi=softfp -mfpu=neon -DNDEBUG -fPIC

#hard-fp version
#CXX=/home/${USER}/workspace/Oasis/tools/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++
#STRIP=CXX=/home/${USER}/workspace/Oasis/tools/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-strip
#CXXFLAGS=-g -Wall -std=c++11 -march=armv7-a -mfloat-abi=hard -mfpu=neon -DNDEBUG -fPIC

CXXFLAGS+=-O0 -fstack-protector-strong
#CXXFLAGS+=-O3

INCLUDES=-I./arm/include -I./include -I./include/iio -I./lib/tinyxml -I./lib/libhttp-1.8/include -I./lib/zip_utils
LDFLAGS=-L./arm/lib -L./lib/tinyxml

LDLIBS=-lpthread -lcdc_base -lcdc_vencoder -lcdc_ve -lcdc_memory -lcdc_vdecoder -ljsoncpp -lfreetype -lpng16 -lz -levdev -ltixml -ldl

#non-gprof
LDLIBS+=-loasis_certs -loasis_codec -loasis_disp -loasis_fs -loasis_media -loasis_ui -loasis_net -loasis_util -loasis
#LDLIBS+=-loasis_certs -loasis_codec -loasis_disp -loasis_fs -loasis_media -loasis_ui -loasis_net -loasis_rtsp -loasis_util -loasis

#gprof
#LDLIBS+=-loasis-static -loffs-direct
#CXXFLAGS+=-pg

#LDLIBS += -lAdas
#LDLIBS += -Wl,--no-as-needed -ltcmalloc_debug

#CXXFLAGS+=-fno-omit-frame-pointer -fsanitize=address
#CXXFLAGS+=-static-libasan  -static-libstdc++
#LDLIBS += -lasan

#CXXFLAGS+=-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
#CXXFLAGS+=-fno-omit-frame-pointer
#LDLIBS += -Wl,--no-as-needed -ltcmalloc

#LDLIBS += libtcmalloc.a

#LDLIBS += -lprofiler

.PHONY: clean
all: clean recorder
#all: recorder
img: make_img

stripped: recorder
	$(STRIP) --strip-unneeded recorder

recorder: recorder.cpp Makefile
	rm -f ../buildroot-oasis/board/datech/oasis_nor/rootfs_overlay/datech/app/httpd_r3

	$(CXX) $(CXXFLAGS) $(INCLUDES) -c hdlc.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c bb_api.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c safe_driving_monitoring.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c data_log.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c recorder.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c recorder_ex.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c bb_micom.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c system_log.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c pai_r_data.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c pai_r_datasaver.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c SB_System.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c led_process.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c serial.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c evdev_input.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c user_data.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c wavplay.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c daSystemSetup.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c ConfigTextFile.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c gsensor.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c mixwav.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c DaUpStreamDelegate.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c ttyraw.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c datools.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c sysfs.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c dagpio.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) *.o $(LDFLAGS) $(LDLIBS) -o $@

#	../../etc/file_extend recorder
#	cp recorder ../buildroot-oasis/board/datech/oasis_nor/userfs/bin/recorder
	cp recorder ../buildroot-oasis/board/datech/oasis_nor/rootfs_overlay/datech/app
	date > ../buildroot-oasis/board/datech/oasis_nor/userfs/reboot.txt

clean:
	$(RM) *.o recorder
make_img:
	../etc/file_checksum ../../oasis/buildroot-oasis/output/firstview_nor/images/sun8iw8p1_linux_oasis_nor_uart0.img recorder

 
