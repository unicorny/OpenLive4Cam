#define __DEBUG
#include "output.h"

typedef struct 
{
	FILE* log;
	//SOCKET socket;
	int  socket;
	
	unsigned long timestamp;
	short sequenznummer;
	int ssrc;

        x264_nal_t* pps;
        x264_nal_t* sps;

	
} rtp_out_handle;


//! \param address url oder IP-Adresse
//! \param port port oder protokoll (z.B. http, ftp, etc)
//! \return socket file descriptor oder -1 bei Fehler
int openSocket(const char* address, const char* port);

//! \brief schlie�t den Socket wieder
void closeSocket(int socket);

int sendFrame(rtp_out_handle* socket, uint8_t *p_nalu, int i_size, x264_picture_t *p_picture);
