#pragma once

#ifndef	_LINUX_
#include <Winsock2.h>			// If you get compile errors, move this line BEFORE '#include <windows.h>'
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define		SOCKET		int
#define		INVALID_SOCKET  (SOCKET)(~0)
#define		SOCKET_ERROR            (-1)
#endif
#include <string>

class Bootpd
{
protected:
#ifndef	_LINUX_
	WSADATA		wsaData;
#endif
	SOCKET		m_s;			///< The server socket.
	sockaddr_in m_bcIp;			///< Broacast socket.
	sockaddr_in m_ifIp;			///< Interface socket.
	sockaddr_in m_tgIp;			///< Target socket.
public:

	Bootpd(void);
	virtual ~Bootpd(void);
	void	Start();

	std::string m_broadcastIp;	///< Broadcast address "255.255.255.255"
	std::string m_interfaceIp;	///< Interface ip address "0.0.0.0"
	std::string m_targetIp;		///< Target ip addres "10.0.0.50"
	std::string m_targetMac;	///< Mac addres "00:11:ee:ff:66:ef"

};
