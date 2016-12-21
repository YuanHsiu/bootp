#ifndef	_LINUX_
#include "stdafx.h"
#endif
/**
 * This class is a bare bones bootp deamon.
 * The bootp protocol is described in RFC-951.
 * 
 * Author Dennis Kuppens
 * 9 June 2007
 *
 * Platform Win32
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * The source code is based on the work of 
 * Ivan Tikhonov [www.brokestream.com/wordpress/category/netboot/] 
 */

#include "Bootpd.h"

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>	// For transform.


Bootpd::Bootpd(void)
{
#ifndef	_LINUX_
	// Initialize WSA (do this only once in your application).
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()!\n");
#endif
}

Bootpd::~Bootpd(void)
{
#ifndef	_LINUX_
	// Cleanup WSA (do this only once in your application).
	WSACleanup();
#endif
}

/**
 * Starts the bootp deamon.
 * This function will never end.  
 */
void Bootpd::Start()
{
	bool error = false;

	if(m_broadcastIp == "")
	{
		printf("Error: m_broadcastIp IP address is empty!\n");
		error = true;
	}
	else
	{
		printf("Info: m_broadcastIp:\t%s\n", m_broadcastIp.c_str());
	}

	if(m_interfaceIp == "")
	{
		printf("Error: m_interfaceIp IP address is empty!\n");
		error = true;
	}
	else
	{
		printf("Info: m_interfaceIp:\t%s\n", m_interfaceIp.c_str());
	}

	if(m_targetIp == "")
	{
		printf("Error: m_targetIp IP address is empty!\n");
		error = true;
	}
	else
	{
		printf("Info: m_targetIp:\t%s\n", m_targetIp.c_str());
	}

	if(m_targetMac == "")
	{
		printf("Error: m_targetMac Mac address is empty!\n");
		error = true;
	}
	else
	{
		printf("Info: m_targetMac:\t%s\n", m_targetMac.c_str());
	}

	if(!error)
	{
		m_bcIp.sin_addr.s_addr = inet_addr(m_broadcastIp.c_str());
		m_ifIp.sin_addr.s_addr = inet_addr(m_interfaceIp.c_str());
		m_tgIp.sin_addr.s_addr = inet_addr(m_targetIp.c_str());

		// Convert the mac address string to lower case.
		std::transform(m_targetMac.begin(),m_targetMac.end(),m_targetMac.begin(),tolower); 
	}

	if(!error)
	{
		// Create an UDP socket. 
		m_s = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

		if( m_s == SOCKET_ERROR )
		{
			printf("Error: Socket error\n");
			error = true;
		}
	}

	// Bind to the listen port 67 (Defined by RFC)
	if(!error)
	{
		sockaddr_in b;
		b.sin_family		= AF_INET;
		b.sin_port			= htons(67);
		b.sin_addr.s_addr	= inet_addr(m_interfaceIp.c_str());
#ifndef	_LINUX_
		if( bind(m_s, (SOCKADDR*)&b, sizeof(b)) == SOCKET_ERROR ) 
#else
		if( bind(m_s, (struct sockaddr *)&b, sizeof(b)) == SOCKET_ERROR ) 		
#endif
		{
			printf("Error: Can't bind to interface address:\t%s\n", m_interfaceIp.c_str());
			printf("Error: Can't bind to interface port:\t%d\n", 67);
			error = true;
		}
		else 
		{
			printf("Info: Binded to address:\t%s\n", m_interfaceIp.c_str());
			printf("Info: Binded to Port:\t%d\n", 67);
		}
	}

	// Change the socket option to 'broadcast' so we can send a bootp response broadcast
	// message.
	if(!error)
	{ 
		int value = 1;
		if( setsockopt(m_s, SOL_SOCKET, SO_BROADCAST, (char*)&value, sizeof(value)) == SOCKET_ERROR )
		{
			printf("Error: setsockopt ERROR!\n");
			error = true;
		}
		else
		{
			printf("Info: Socket option changed to SO_BROADCAST\n");
		}
	}

	// Start the server and listen for bootp requests.
	if(!error)
	{
		for(;;)
		{
			fd_set rset;
			SOCKET n;
			FD_ZERO(&rset);
			FD_SET(m_s,&rset);
			n = select((int)m_s+1,&rset,0,0,0);

			if(FD_ISSET(m_s,&rset)) 
			{
				unsigned char p[1500] = {0xff};
				recv( m_s, (char*)p, 1500, 0 );
				int type = p[0];
				if(type != 1) continue;	// Not a request? continue.
				
				char info[20];
				sprintf(info, "%02x:%02x:%02x:%02x:%02x:%02x",p[28],p[29],p[30],p[31],p[32],p[33]);
				printf("Info: Bootp request from:\t%s\n", info);
				
				std::string reqMac;
				reqMac = info;

				// Check if the request mac address matches our defined mac address. 
				if(m_targetMac != reqMac) continue;

				// We have match. Now assign the provided ip address to the client. 
				printf("Info: Assigning ip address:\t%s to %s\n", m_targetIp.c_str(), reqMac.c_str());

				p[0] = 2;	// Change bootp message type to 'response'
				p[12] = 0; p[13] = 0; p[14] = 0; p[15] = 0;
				
				// The ip address the client should use.
				p[16] = char(m_tgIp.sin_addr.s_addr&0xff); 
				p[17] = char((m_tgIp.sin_addr.s_addr>>8)&0xff); 
				p[18] = char((m_tgIp.sin_addr.s_addr>>16)&0xff); 
				p[19] = char((m_tgIp.sin_addr.s_addr>>24)&0xff);
				
				// Our own ip address. 
				p[20] = char(m_ifIp.sin_addr.s_addr&0xff); 
				p[21] = char((m_ifIp.sin_addr.s_addr>>8)&0xff); 
				p[22] = char((m_ifIp.sin_addr.s_addr>>16)&0xff); 
				p[23] = char((m_ifIp.sin_addr.s_addr>>24)&0xff);

				// Send the response message.
				{
					sockaddr_in a;
					a.sin_family		= AF_INET;
					a.sin_port			= htons(68);
					a.sin_addr.s_addr	= inet_addr(m_broadcastIp.c_str());

					// A bootp response packet is 300 bytes big. 
					if( SOCKET_ERROR == sendto( m_s, (char*)p, 300, 0, (sockaddr *)&a, sizeof(a)) )
					{
						printf("Error: Sendto error, could not send bootp response.\n");
#ifndef	_LINUX_
						printf("Error: WSAGetLastError():\t%d\n", WSAGetLastError());
#endif
					}
				}
			}
		}
	}
}

