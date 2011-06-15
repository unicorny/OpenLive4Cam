/*****************************************************************************
 * raw.c: raw muxer
 *****************************************************************************
 * Copyright (C) 2003-2010 x264 project
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 *          Loren Merritt <lorenm@u.washington.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@x264.com.
 *****************************************************************************/

#include "output.h"
#include "cencode.h"
#include "assert.h"

#define __DEBUG

void print_nal(x264_nal_t* pNal)
{
	printf("--- Nal ---\n");
	printf("Priority: %d\n", pNal->i_ref_idc);
	printf("Unit Type: %d\n", pNal->i_type);
	printf("Startcode: %d\n", pNal->b_long_startcode);
	printf("Index of First MB: %d\n", pNal->i_first_mb);
	printf("Index of Last  MB: %d\n", pNal->i_last_mb);
	printf("Size of payload in Bytes: %d\n", pNal->i_payload);
	printf("--- NAL ENDE ---\n"); 
}

int writeSdp(FILE* f, const char* pps, const char* sps, const uint8_t *profile_level_id )
{
	printf("writeSDP\n");
	int port = 5004;
	int clockrate = 90000;
	float framerate  = 25;
	if(!f) 
	{
		printf("[writeSdp] f is zero\n");
		return -1;
	}
	fprintf(f, "v=0\n");
	fprintf(f, "o=- %d 7 IN IP4 0.0.0.0\n", (int)time(NULL));
	fprintf(f, "s=x264_Video_Stream_HTW_Projekt\n");
	fprintf(f, "i=x264 Video Stream as a HTW Projekt from Students\n");
	fprintf(f, "e=em741@gmx.de\n");
	fprintf(f, "c=IN IP4 0.0.0.0\n");
	// t=<start-time> <stop-time> 
	// wenn set to zero, stream is always active
	fprintf(f, "t=0 0\n");
	fprintf(f, "m=video %d RTP/AVP 96\n", port);
	fprintf(f, "a=framerate:%.1f\n", framerate);
	fprintf(f, "a=rtpmap:96 H264/%d\n", clockrate);
	// Qualit�t 0 (worst) is 10 (best)
	// fprintf(f, "a=quality:9\n");
	
	fprintf(f, 
		"a=fmtp:96 packetization-mode=1; "
		"profile-level-id=%02X%02X%02X; "
		"sprop-parameter-sets=%s,%s\n", 
		profile_level_id[0],
		profile_level_id[1],
		profile_level_id[2],
		sps, 
		pps
	);
	return 0;	
}


static int open_file( char *psz_filename, hnd_t *p_handle )
{
    printf("open SDP File %s\n",psz_filename );
        
/*    char str_port[32];
	strcpy(str_port, get_filename_port(psz_filename));
	//const char* str_port = get_filename_port(server);
	int port = atoi(str_port);
	// delete port from string
	char* point = psz_filename + strlen(psz_filename)-(strlen(str_port)+1);
	memset(point, 0, sizeof(char*)*strlen(str_port));
	*/
	
	// debug info
#ifdef __DEBUG
	//printf("filename: %s\n", psz_filename);	
#endif	
	
	if( !strcmp( psz_filename, "-" ) )
        *p_handle = stdout;
    else if( !(*p_handle = fopen( psz_filename, "wt" )) )
        return -1;
		
    return 0;
}

static int set_param( hnd_t handle, x264_param_t *p_param )
{
    return 0;
}

static int write_headers( hnd_t handle, x264_nal_t *p_nal )
{
	printf("Write header called\n");

    int sps_size = p_nal[0].i_payload - 4;
    int pps_size = p_nal[1].i_payload - 4;
    int sei_size = p_nal[2].i_payload - 4 ;

    uint8_t *sps = p_nal[0].p_payload + 4;	// ignore leading start code 00 00 00 01
    uint8_t *pps = p_nal[1].p_payload + 4;	// ignore leading start code 00 00 00 01
    //uint8_t *sei = p_nal[2].p_payload + 4;	// ignore leading start code 00 00 00 01
    
	//printf( "%02X %02X %02X %02X %02X %02X", sps[0], sps[1], sps[2], sps[3], sps[4], sps[5] );
	// see ITU Rec. H.264 - NAL unit syntax
	assert( sps[0]== 0x67);
	assert( pps[0]== 0x68);
	
	// siehe http://www.itu.int/rec/T-REC-H.264-201003-I
	uint8_t *profile_level_id = sps + 1;	// first 3 bytes after NALU hdr
	
    base64_encodestate encoder;
    // base64 encode sps
    base64_init_encodestate(&encoder);
    char* sps_buffer = (char*)malloc(sps_size*2); 
    if(!sps_buffer)
    {
    	fprintf(stderr, "Fehler, es konnte kein Speicher f�r den SPS-Buffer reserviert werden\n");
    	return -1;
    }
    memset(sps_buffer, 0, sps_size*2*sizeof(char));
 	//encode   
    int sps_encoded_size = base64_encode_block((const char*)sps, sps_size, sps_buffer, &encoder);
    sps_encoded_size += base64_encode_blockend(sps_buffer+sps_encoded_size, &encoder);
   	//remove umbruch	
	sps_buffer[sps_encoded_size-1] = '\0';
    //show
    printf("-- SPS ---\nOriginal Size: %d, size after encoding: %d\nSPS: %s\n---End---\n", sps_size, sps_encoded_size, sps_buffer);
    
    //base64 encode PPS
    base64_init_encodestate(&encoder);
    char* pps_buffer = (char*)malloc(pps_size*2); 
    if(!pps_buffer)
    {
    	fprintf(stderr, "Fehler, es konnte kein Speicher f�r den PPS-Buffer reserviert werden\n");
    	return -1;	
    }
    memset(pps_buffer, 0, pps_size*2*sizeof(char));
    //encode
    int pps_encoded_size = base64_encode_block((const char*)pps, pps_size, pps_buffer, &encoder);
    pps_encoded_size += base64_encode_blockend(pps_buffer+pps_encoded_size, &encoder);
    //show
	printf("-- PPS ---\nOriginal Size: %d, size after encoding: %d\nPPS: %s\n---End---\n", pps_size, pps_encoded_size, pps_buffer);

	//write sdp file
    writeSdp((FILE*)handle, pps_buffer, sps_buffer, profile_level_id );
    fclose((FILE*)handle);
    
    //free data
    free(sps_buffer);
    free(pps_buffer);

  /*  if( fwrite( p_nal[0].p_payload, size, 1, (FILE*)handle ) )
        return size;
    return -1;
    */
    return sps_size + pps_size + sei_size;
}

static int write_frame( hnd_t handle, uint8_t *p_nalu, int i_size, x264_picture_t *p_picture )
{
/*    if( fwrite( p_nalu, i_size, 1, (FILE*)handle ) )
        return i_size;
    return -1;
   */
   return i_size;
}

static int close_file( hnd_t handle, int64_t largest_pts, int64_t second_largest_pts )
{
	//fclose((FILE*)handle);
	return 0;
	/*
    if( !handle || handle == stdout )
        return 0;

    return fclose( (FILE*)handle );
   
    //return 0;*/
}

const cli_output_t sdp_output = { open_file, set_param, write_headers, write_frame, close_file };

