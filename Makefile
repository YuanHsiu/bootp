
all:	bootpd

bootpd: 	main.cpp Bootpd.cpp
	g++ -lstdc++ -D_LINUX_ -o bootpd main.cpp Bootpd.cpp
