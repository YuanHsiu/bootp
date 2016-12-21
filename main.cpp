// bootpd.cpp : Defines the entry point for the console application.
//
#ifndef	_LINUX_
#include "stdafx.h"
#else
 #include <stdio.h>
#endif
#include "Bootpd.h"

#ifndef	_LINUX_
#pragma comment(lib,"ws2_32.lib")
#endif

#ifndef	_LINUX_
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char *argv[])
#endif
{
	printf("Starting bootp deamon...\n");

	Bootpd bootpd;

	bootpd.m_broadcastIp	= "255.255.255.255";
	bootpd.m_interfaceIp	= "0.0.0.0";
	bootpd.m_targetIp		= "10.0.1.55";
	bootpd.m_targetMac		= "00:11:22:33:44:55";

	bootpd.Start();

	return 0;
}

