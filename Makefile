CC = g++
ifeq ($(shell sw_vers 2>/dev/null | grep Mac | awk '{ print $$2}'),Mac)
CFLAGS = -g -DOSX
INCFLAGS = -I./glm-0.9.2.7 -I/usr/X11/include -I./include/
LDFLAGS = -lm -lstdc++ -lfreeimage
else
CFLAGS = -g 
INCFLAGS = -I./glm-0.9.2.7 -I./include/ -I/usr/X11R6/include -I/sw/include \
		-I/usr/sww/include -I/usr/sww/pkg/Mesa/include
LDFLAGS = -lX11 -lfreeimage
endif

RM = /bin/rm -f 
all: raytrace
raytrace: raytracer.o Transform.o accel_grid.o readfile.o variables.h readfile.h Transform.h accel_grid.h
	$(CC) $(CFLAGS) -o raytracer raytracer.o Transform.o accel_grid.o readfile.o $(INCFLAGS) $(LDFLAGS) 
raytracer.o: raytracer.cpp Transform.h variables.h accel_grid.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c raytracer.cpp
readfile.o: readfile.cpp readfile.h variables.h 
	$(CC) $(CFLAGS) $(INCFLAGS) -c readfile.cpp
Transform.o: Transform.cpp Transform.h 
	$(CC) $(CFLAGS) $(INCFLAGS) -c Transform.cpp
accel_grid.o: accel_grid.cpp accel_grid.h Transform.h variables.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c accel_grid.cpp
clean: 
	$(RM) *.o raytracer *.png


 
