#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#ifdef __WIN32
#define _WIN32_WINNT 0x501 // fix for bug in ws2tcpip.h header
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#define BYTE unsigned char
#define MAX_RTP_PACKET_SIZE 1040
#define MAX_RTP_PAYLOAD_SIZE (MAX_RTP_PACKET_SIZE-16)

#ifdef _WIN32

static int WSAInit = 0;
#endif

const char g_slice_types2[] = {'a', 'I', 'i', 'p', 'B', 'b', 'k'};

const char* getErrnoText()
{
#ifdef _WIN32	
	int err = WSAGetLastError();
#else
	int err = errno;
#endif
	switch(err)
	{
#ifndef _WIN32
		case EACCES: return "EACCES";
//		case EAGAIN: return "EAGAIN";
		case EWOULDBLOCK: return "EWOULDBLOCK"; 
		case EBADF: return "EBADF";
		case ECONNRESET: return "ECONNRESET";
		case EDESTADDRREQ: return "EDESTADDRREQ";
		case EFAULT: return "EFAULT, ein uebergebender Pointer zeigt in einen ungueltigen Bereich";
		case EINTR: return "EINTR";
		case EINVAL: return "EINVAL";
		case EISCONN: return "EISCONN";
		case EMSGSIZE: return "EMSGSIZE";
		case ENOBUFS: return "ENOBUFFS";
		case ENOMEM: return "ENOMEM";
		case ENOTCONN: return "ENOTCON";
		case ENOTSOCK: return "ENOTSOCK";
		case EOPNOTSUPP: return "EOPNOTSUPP";
		case EPIPE: return "EPIPE";
		case 111: return "connection refused";
#else
		case WSANOTINITIALISED: return "WSA nicht initalisiert";
		case WSAENETDOWN: return "network subsystem has failed";
		case WSAEACCES: return "the request address is broadcast without the flag set";
		case WSAEINTR: return "WSAEINTR";
		case WSAEINPROGRESS: return "WSAEINPROGRESS";
		case WSAEFAULT: return "WSAEFAULT";
		case WSAENETRESET: return "WSAENETRESET";
		case WSAENOBUFS: return "WSAENOBUFS";
		case WSAENOTCONN: return "socket is not connecte";
		case WSAENOTSOCK: return "the descriptor ist not a socket";
		case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
		case WSAESHUTDOWN: return "WSAESHUTDOWN";
		case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
		case WSAEMSGSIZE: return "WSAEMSGSIZE";
		case WSAEHOSTUNREACH: return "the remote host isn't reachable";
		case WSAEINVAL: return "the socket hast not been bound with bind, or an unknown flag was specified";
		case WSAECONNABORTED: return "WSAECONNABORTED";
		case WSAECONNRESET: return "WSAECONNRESET";
		case WSAETIMEDOUT: return "WSAETIMEDOUT";
	
#endif
	
		default: return "unbehandelte errno";
	};
	return "-- error --";
	
}

void printPicture_t(x264_picture_t* pic)
{
	printf("--- Picture ----\n");
	printf("type: %d, %c\n", pic->i_type, g_slice_types2[pic->i_type]);
	printf("i_qpplus1 :%d\n", pic->i_qpplus1);
	printf("pic struct: %d\n", pic->i_pic_struct);
	printf("is keyframe: %d\n", pic->b_keyframe);
	printf("pts: %6ld\n", (long int)pic->i_pts);
	printf("dts: %6ld\n", (long int)pic->i_dts);
	
	printf("----- Ende -----\n");
}

void printInfos(struct addrinfo* p)
{
	int i = 0;
	struct sockaddr_in  *sockaddr_ipv4;
//  struct sockaddr_in6 *sockaddr_ipv6;

	//int iResult;
#ifdef _WIN32
    INT iRetval;
    LPSOCKADDR sockaddr_ip;
    DWORD ipbufferlength = 46;
    char ipstringbuffer[46];
#else
 	
#endif
	
    

	
	printf("getaddrinfo response %d\n", i++);
	printf("\tFlags: 0x%x\n", p->ai_flags);
	printf("\tFamily: ");
	switch (p->ai_family) {
		case AF_UNSPEC:
			printf("Unspecified\n");
			break;
		case AF_INET:
			printf("AF_INET (IPv4)\n");
			sockaddr_ipv4 = (struct sockaddr_in *) p->ai_addr;
			printf("\tIPv4 address %d\n",
				inet_ntoa(sockaddr_ipv4->sin_addr) );
			break;
		case AF_INET6:
			printf("AF_INET6 (IPv6)\n");
			// the InetNtop function is available on Windows Vista and later
			// sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
			// printf("\tIPv6 address %s\n",
			//    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );
			
			// We use WSAAddressToString since it is supported on Windows XP and later
#ifdef _WIN32			
			sockaddr_ip = (LPSOCKADDR) p->ai_addr;

			

			// The buffer length is changed by each call to WSAAddresstoString
			// So we need to set it for each iteration through the loop for safety
			ipbufferlength = 46;
			iRetval = WSAAddressToString(sockaddr_ip, (DWORD) p->ai_addrlen, NULL, 
				ipstringbuffer, &ipbufferlength );
			if (iRetval)
				printf("WSAAddressToString failed with %u\n", WSAGetLastError() );
			else    
				printf("\tIPv6 address %s\n", ipstringbuffer);
			break;

		case AF_NETBIOS:
			printf("AF_NETBIOS (NetBIOS)\n");
			break;
#endif
		default:
			printf("Other %d\n", p->ai_family);
			break;
	}
	printf("\tSocket type: ");
	switch (p->ai_socktype) {
		case 0:
			printf("Unspecified\n");
			break;
		case SOCK_STREAM:
			printf("SOCK_STREAM (stream)\n");
			break;
		case SOCK_DGRAM:
			printf("SOCK_DGRAM (datagram) \n");
			break;
		case SOCK_RAW:
			printf("SOCK_RAW (raw) \n");
			break;
		case SOCK_RDM:
			printf("SOCK_RDM (reliable message datagram)\n");
			break;
		case SOCK_SEQPACKET:
			printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
			break;
		default:
			printf("Other %d\n", p->ai_socktype);
			break;
	}
	printf("\tProtocol: ");
	switch (p->ai_protocol) {
		case 0:
			printf("Unspecified\n");
			break;
		case IPPROTO_TCP:
			printf("IPPROTO_TCP (TCP)\n");
			break;
		case IPPROTO_UDP:
			printf("IPPROTO_UDP (UDP) \n");
			break;
		default:
			printf("Other %d\n", p->ai_protocol);
			break;
	}
	printf("\tLength of this sockaddr: %d\n", p->ai_addrlen);
	printf("\tCanonical name: %s\n", p->ai_canonname);
}

void printByte(BYTE b)
{
	BYTE mask[] = {1,2,4,8,16,32,64,128};
	int i = 0;
	for(i = 0; i < 8; i++)
	{
		printf("%d", (b & mask[i])? 1:0);
	}
	printf("\n");
}

int openSocket(const char* address, const char* port)
{
	printf("openSocket\n");
	int s = 0, status;
#ifdef _WIN32
	WSADATA wsaData;
	//WSAData wsaData; //falls das andere nicht geht
	// MAKEWORD(1,1) for  Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:
	if(!WSAInit)
	{
		if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
		{
			fprintf(stderr, "WSAStartup failed.\n");
			return -1;
		}
		WSAInit = 1;
	}
#endif
	struct addrinfo hints, *servinfo = NULL, *p = NULL, *pSocket = NULL;
	memset(&hints, 0, sizeof(struct addrinfo));

	//prepare
	hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
//	*/
	
	// do the lookup
	// [pretend we already filled out the "hints" struct]
	status = getaddrinfo(address, port, &hints, &servinfo);
	if(status != 0)
	{
		printf("status != 0\n");
		fprintf(stderr, "addresse: %s, port: %s, getaddrinfo error: %s\n", address, port, gai_strerror(status));
		return -1;
	}
	//printf("getaddrinfo fertig\n");

	// Retrieve each address and print out the hex bytes
    for(p=servinfo; p != NULL ;p=p->ai_next) 
	{
#ifdef __DEBUG
		printInfos(p);
#endif
		//Socket erstellen
		if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			fprintf(stderr, "socket erstellung fehlgeschlagen, weiter naechster Versuch!\n");
			continue;
		}
		else
		{
			pSocket= p;
			fprintf(stderr, "socket wurde erstellt!\n");
			break;
		}
    }
#ifdef __DEBUG
	printf("socket %d\n", s);	
#endif
	
	if(s != -1)
	{	
		//printf("s: %d, ai_addr: %d" );//, ai_addr: %d, ai_addrlen: %d", s, p->ai_addr, p->ai_addrlen);
		// socket connecten, damit der send aufruf leichter ist
		if(connect(s, pSocket->ai_addr, pSocket->ai_addrlen) != 0)
		{
#ifdef _WIN32
			//int err = WSAGetLastError();
			fprintf(stderr, "errorcode: %s ", getErrnoText());
#endif
			fprintf(stderr, "Fehler beim connecten: %d, %s\n", errno, getErrnoText());
		}	
		else
		{
			printf("connection wurde erstellet\n");
		}
	}
	
	freeaddrinfo(servinfo);
	
	return s;					
}

char* getHead(char* head, BYTE marker, BYTE payloadTyp, short seqNr, int timestamp, int ssrc)
{
	//BYTE* head = malloc(sizeof(BYTE)*12);
	short *shead = (short*)head;
	int* ihead = (int*)head;
	head[0] = 2<<6;
	head[1] = ((marker & 1)<<7)|(payloadTyp);
	shead[1] = htons(seqNr);	
	ihead[1] = htonl(timestamp);
	ihead[2] = ssrc;
	//shead[6] = htons(0x5C01);
	return head;
}

int sendRTPPacket(rtp_out_handle* p, uint8_t *p_nalu, int i_size, int first, int last, int nalu_typ)
{
    char buffer[MAX_RTP_PACKET_SIZE];
    BYTE marker = 0;
    int ret = 0;
    if(last) marker = 1;
    BYTE payloadTyp = 96;
    
    memset(buffer, 0, MAX_RTP_PACKET_SIZE);
    getHead(buffer, marker, payloadTyp, p->sequenznummer++, p->timestamp, p->ssrc);

    if( first & last )
    {
        // a FU_A is not allowed here by RFC 3984
        buffer[12] = nalu_typ;
        memcpy( buffer+13, p_nalu, i_size );
        i_size += 13;
    }
    else
    {
        // send FU-A (fragmentation unit type A)
        // F|NRI| Type
        buffer[12] = 28 | (nalu_typ & 0x60); // FU indicator. NRI taken from original NAL unit
        // S|E|R| Type
        buffer[13] = (first ? 0x80 : 0) | (last ? 0x40 : 0) | (0x1F & nalu_typ); // FU header: S:1 E:1 R:1 Typ:5
        memcpy( buffer+14, p_nalu, i_size );
        i_size += 14;
    }
  //  printf("timestamp %10d: sending %4d bytes\n", p->timestamp, i_size );
    ret = send(p->socket, buffer, i_size, 0);
    if(ret < 0)
    {
#ifdef _WIN32
        int err = WSAGetLastError();
        fprintf(stderr, "Fehler bei send:  %s\n\n", getErrnoText());
        if(err == WSAEINVAL) return 0;
#else
        fprintf(stderr, "Fehler bei send: %d, %s\n\n", errno, getErrnoText());
        if(errno == 111) return 0;
#endif
        return -1;
    }
    
    return ret;

}

int sendFrame(rtp_out_handle* p, uint8_t *p_nalu, int i_size, x264_picture_t *p_picture)
{
    int offset = 1;
    int packet_size = 0;
    int first = 1;
    int last = 0;
    p->timestamp = (unsigned long)((90000ULL * clock()) / CLOCKS_PER_SEC);

    while(!last)
    {
      if(offset > i_size - MAX_RTP_PAYLOAD_SIZE)
      {
          packet_size = i_size - offset;
          last = 1;
      }
      else
      {
          packet_size = MAX_RTP_PAYLOAD_SIZE;
      }

      sendRTPPacket(p, p_nalu+offset, packet_size, first, last, p_nalu[0]);
      first = 0;

      offset += packet_size;
    }

#ifdef __WIN32
        Sleep(30);
#else
	usleep(50000);
#endif	
    return 0;
}

void closeSocket(int s)
{
#ifdef _WIN32
	closesocket(s);
	WSACleanup();
	WSAInit = 0;
#else
	close(s);
#endif
}
