/*****************************************************************************
 * x264: top-level x264cli functions
 *****************************************************************************
 * Copyright (C) 2003-2011 x264 project
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Laurent Aimar <fenrir@via.ecp.fr>
 *          Steven Walters <kemuri9@gmail.com>
 *          Jason Garrett-Glaser <darkshikari@gmail.com>
 *          Kieran Kunhya <kieran@kunhya.com>
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
    
#include <signal.h>
#define _GNU_SOURCE
#include <getopt.h>
#include "common/common.h"
#include "x264cli.h"
#include "input/input.h"
#include "output/output.h"
#include "filters/filters.h"
    


#define FAIL_IF_ERROR( cond, ... ) FAIL_IF_ERR( cond, "x264", __VA_ARGS__ )

#ifdef _WIN32
#include <windows.h>
#else
#define GetConsoleTitle(t,n)
#define SetConsoleTitle(t)
#endif

#if HAVE_LAVF
#undef DECLARE_ALIGNED
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
#include <libavutil/pixdesc.h>
#endif

#if HAVE_SWSCALE
#include <libswscale/swscale.h>
#endif

#if HAVE_FFMS
#include <ffms.h>
#endif

/* Ctrl-C handler */
static volatile int b_ctrl_c = 0;
static int          b_exit_on_ctrl_c = 0;


static void sigint_handler( int a )
{
    if( b_exit_on_ctrl_c )
        exit(0);
    b_ctrl_c = 1;
}

typedef struct {
    int b_progress;
    int i_seek;
    hnd_t hin;
    hnd_t hout;
    FILE *qpfile;
    FILE *tcfile_out;
    double timebase_convert_multiplier;
    int i_pulldown;
} cli_opt_t;

cli_opt_t opt = {0};

/* file i/o operation structs */
cli_input_t input;
static cli_output_t output;

/* video filter operation struct */
static cli_vid_filter_t filter;

static const char * const demuxer_names[] =
{
    "auto",
    "raw",
    "y4m",
#if HAVE_AVS
    "avs",
#endif
#if HAVE_LAVF
    "lavf",
#endif
#if HAVE_FFMS
    "ffms",
#endif
    0
};

static const char * const muxer_names[] =
{
    "auto",
    "raw",
    "mkv",
    "flv",
#if HAVE_GPAC
    "mp4",
#endif
	"rtp",
	"sdp",
    0
};


typedef struct
{
    int mod;
    uint8_t pattern[24];
    float fps_factor;
} cli_pulldown_t;

enum pulldown_type_e
{
    X264_PULLDOWN_22 = 1,
    X264_PULLDOWN_32,
    X264_PULLDOWN_64,
    X264_PULLDOWN_DOUBLE,
    X264_PULLDOWN_TRIPLE,
    X264_PULLDOWN_EURO
};

#define TB  PIC_STRUCT_TOP_BOTTOM
#define BT  PIC_STRUCT_BOTTOM_TOP
#define TBT PIC_STRUCT_TOP_BOTTOM_TOP
#define BTB PIC_STRUCT_BOTTOM_TOP_BOTTOM

static const cli_pulldown_t pulldown_values[] =
{
    [X264_PULLDOWN_22]     = {1,  {TB},                                   1.0},
    [X264_PULLDOWN_32]     = {4,  {TBT, BT, BTB, TB},                     1.25},
    [X264_PULLDOWN_64]     = {2,  {PIC_STRUCT_DOUBLE, PIC_STRUCT_TRIPLE}, 1.0},
    [X264_PULLDOWN_DOUBLE] = {1,  {PIC_STRUCT_DOUBLE},                    2.0},
    [X264_PULLDOWN_TRIPLE] = {1,  {PIC_STRUCT_TRIPLE},                    3.0},
    [X264_PULLDOWN_EURO]   = {24, {TBT, BT, BT, BT, BT, BT, BT, BT, BT, BT, BT, BT,
                                   BTB, TB, TB, TB, TB, TB, TB, TB, TB, TB, TB, TB}, 25.0/24.0}
};

#undef TB
#undef BT
#undef TBT
#undef BTB

// indexed by pic_struct enum
static const float pulldown_frame_duration[10] = { 0.0, 1, 0.5, 0.5, 1, 1, 1.5, 1.5, 2, 3 };

static int  parse(  x264_param_t *param, cli_opt_t *opt, char* resolution );
static int  encode( x264_param_t *param, cli_opt_t *opt );

/* logging and printing for within the cli system */
static int cli_log_level;
void x264_cli_log( const char *name, int i_level, const char *fmt, ... )
{
    if( i_level > cli_log_level )
        return;
    char *s_level;
    switch( i_level )
    {
        case X264_LOG_ERROR:
            s_level = "error";
            break;
        case X264_LOG_WARNING:
            s_level = "warning";
            break;
        case X264_LOG_INFO:
            s_level = "info";
            break;
        case X264_LOG_DEBUG:
            s_level = "debug";
            break;
        default:
            s_level = "unknown";
            break;
    }
    fprintf( stderr, "%s [%s]: ", name, s_level );
    va_list arg;
    va_start( arg, fmt );
    vfprintf( stderr, fmt, arg );
    va_end( arg );
}


//! \brief init encoder
//! \return -1 by error
//! \return return-value of encode()
int start_x264(char* resolution)
{
    x264_param_t param;
    
    int ret = 0;

    FAIL_IF_ERROR( x264_threading_init(), "unable to initialize threading\n" )

#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    /* Parse command line */
    if( parse( &param, &opt, resolution ) < 0 )
    {
        printf("capture: parser error");
        ret = -1;
    }
    
    /* Control-C handler */
    //signal( SIGINT, sigint_handler );
    printf("encode::x264 ret: %d\n", ret);
 
    if( !ret )
        ret = encode( &param, &opt );

    

    return ret;
}

static int init_vid_filters( char *sequence, hnd_t *handle, video_info_t *info, x264_param_t *param )
{
    x264_register_vid_filters();

    /* intialize baseline filters */
    if( x264_init_vid_filter( "source", handle, &filter, info, param, NULL ) ) /* wrap demuxer into a filter */
        return -1;
    if( x264_init_vid_filter( "resize", handle, &filter, info, param, "normcsp" ) ) /* normalize csps to be of a known/supported format */
        return -1;
    if( x264_init_vid_filter( "fix_vfr_pts", handle, &filter, info, param, NULL ) ) /* fix vfr pts */
        return -1;

    /* parse filter chain */
    for( char *p = sequence; p && *p; )
    {
        int tok_len = strcspn( p, "/" );
        int p_len = strlen( p );
        p[tok_len] = 0;
        int name_len = strcspn( p, ":" );
        p[name_len] = 0;
        name_len += name_len != tok_len;
        printf("encoder.x264 add Filter: %s\n", p);
        if( x264_init_vid_filter( p, handle, &filter, info, param, p + name_len ) )
            return -1;
        p += X264_MIN( tok_len+1, p_len );
    }

    /* force end result resolution */
    if( !param->i_width && !param->i_height )
    {
        param->i_height = info->height;
        param->i_width  = info->width;
    }
    /* if the current csp is supported by libx264, have libx264 use this csp.
     * otherwise change the csp to I420 and have libx264 use this.
     * when more colorspaces are supported, this decision will need to be updated. */
    int csp = info->csp & X264_CSP_MASK;
    if( csp > X264_CSP_NONE && csp < X264_CSP_MAX )
        param->i_csp = info->csp;
    else
        param->i_csp = X264_CSP_I420 | ( info->csp & X264_CSP_HIGH_DEPTH );
    if( x264_init_vid_filter( "resize", handle, &filter, info, param, NULL ) )
        return -1;

    char args[20];
    sprintf( args, "bit_depth=%d", x264_bit_depth );

    if( x264_init_vid_filter( "depth", handle, &filter, info, param, args ) )
        return -1;

    return 0;
}


static int parse( x264_param_t *param, cli_opt_t *opt, char* resolution )
{
    char *input_filename = NULL;
    //const char *demuxer = demuxer_names[0];
    char *output_filename = NULL;
    const char *muxer = muxer_names[0];
    x264_param_t defaults;
    char *profile = NULL;
    char *vid_filters = NULL;
    cli_input_opt_t input_opt;
    cli_output_opt_t output_opt;

    x264_param_default( &defaults );
    cli_log_level = defaults.i_log_level;

    memset( &input_opt, 0, sizeof(cli_input_opt_t) );
    memset( &output_opt, 0, sizeof(cli_output_opt_t) );
    input_opt.bit_depth = 8;
    opt->b_progress = 1;
   
    
    if( x264_param_default_preset( param, NULL, NULL ) < 0 )
        return -1;
    
    profile = "baseline";
    input_opt.resolution = resolution;    
    output_filename = "rtp://192.168.1.51:5004";
    //output_filename = "/media/Videos/jumper.sdp";
    //input_filename = "/media/Videos/jumper.yuv";

    /* If first pass mode is used, apply faster settings. */
    x264_param_apply_fastfirstpass( param );

    /* Apply profile restrictions. */
    if( x264_param_apply_profile( param, profile ) < 0 )
        return -1;

    /*if( select_output( muxer, output_filename, param ) )
        return -1;*/
    output = rtp_output;
    //param->b_repeat_headers = 0;
    FAIL_IF_ERROR( output.open_file( output_filename, &opt->hout, &output_opt ), "could not open output file `%s'\n", output_filename )

    video_info_t info = {0};
    char demuxername[5];

    /* set info flags to param flags to be overwritten by demuxer as necessary. */
    info.csp        = param->i_csp;
    info.fps_num    = param->i_fps_num;
    info.fps_den    = param->i_fps_den;
    info.interlaced = param->b_interlaced;
    info.sar_width  = param->vui.i_sar_width;
    info.sar_height = param->vui.i_sar_height;
    info.tff        = param->b_tff;
    info.vfr        = param->b_vfr_input;

    input_opt.progress = opt->b_progress;

    //if( select_input( demuxer, demuxername, input_filename, &opt->hin, &info, &input_opt ) )
    input = raw_input;
        //return -1;
        

    FAIL_IF_ERROR( !opt->hin && input.open_file( input_filename, &opt->hin, &info, &input_opt ),
                   "could not open input file `%s'\n", input_filename )
    

    x264_reduce_fraction( &info.sar_width, &info.sar_height );
    x264_reduce_fraction( &info.fps_num, &info.fps_den );
    x264_cli_log( demuxername, X264_LOG_INFO, "%dx%d%c %d:%d @ %d/%d fps (%cfr)\n", info.width,
                  info.height, info.interlaced ? 'i' : 'p', info.sar_width, info.sar_height,
                  info.fps_num, info.fps_den, info.vfr ? 'v' : 'c' ); 
     
    /* override detected values by those specified by the user */
    info.timebase_num = info.fps_den;
    info.timebase_den = info.fps_num;
       
    
    if( init_vid_filters( vid_filters, &opt->hin, &info, param ) )
        return -1;
    
    

    /* set param flags from the post-filtered video */
    param->b_vfr_input = info.vfr;
    param->i_fps_num = info.fps_num;
    param->i_fps_den = info.fps_den;
    param->i_timebase_num = info.timebase_num;
    param->i_timebase_den = info.timebase_den;
    param->vui.i_sar_width  = info.sar_width;
    param->vui.i_sar_height = info.sar_height;

    info.num_frames = X264_MAX( info.num_frames - opt->i_seek, 0 );
    if( (!info.num_frames || param->i_frame_total < info.num_frames)
        && param->i_frame_total > 0 )
        info.num_frames = param->i_frame_total;
    param->i_frame_total = info.num_frames;

    /* Automatically reduce reference frame count to match the user's target level
     * if the user didn't explicitly set a reference frame count. */
    
    int mbs = (((param->i_width)+15)>>4) * (((param->i_height)+15)>>4);
    for( int i = 0; x264_levels[i].level_idc != 0; i++ )
        if( param->i_level_idc == x264_levels[i].level_idc )
        {
            while( mbs * 384 * param->i_frame_reference > x264_levels[i].dpb &&
                   param->i_frame_reference > 1 )
            {
                param->i_frame_reference--;
            }
            break;
        }

    return 0;
}


static int encode_frame( x264_t *h, hnd_t hout, x264_picture_t *pic, int64_t *last_dts )
{
    x264_picture_t pic_out;
    x264_nal_t *nal;
    int i_nal;
    int i_frame_size = 0;
    
    i_frame_size = x264_encoder_encode( h, &nal, &i_nal, pic, &pic_out );
    

    FAIL_IF_ERROR( i_frame_size < 0, "x264_encoder_encode failed\n" );

    if( i_frame_size )
    {
        i_frame_size = output.write_frame( hout, nal[0].p_payload, i_frame_size, &pic_out );
        *last_dts = pic_out.i_dts;
    }    

    return i_frame_size;
}

static int64_t print_status( int64_t i_start, int64_t i_previous, int i_frame, int i_frame_total, int64_t i_file, x264_param_t *param, int64_t last_ts )
{
    char buf[200];
    int64_t i_time = x264_mdate();
    if( i_previous && i_time - i_previous < UPDATE_INTERVAL )
        return i_previous;
    int64_t i_elapsed = i_time - i_start;
    double fps = i_elapsed > 0 ? i_frame * 1000000. / i_elapsed : 0;
    double bitrate;
    if( last_ts )
        bitrate = (double) i_file * 8 / ( (double) last_ts * 1000 * param->i_timebase_num / param->i_timebase_den );
    else
        bitrate = (double) i_file * 8 / ( (double) 1000 * param->i_fps_den / param->i_fps_num );
    if( i_frame_total )
    {
        int eta = i_elapsed * (i_frame_total - i_frame) / ((int64_t)i_frame * 1000000);
        sprintf( buf, "x264 [%.1f%%] %d/%d frames, %.2f fps, %.2f kb/s, eta %d:%02d:%02d",
                 100. * i_frame / i_frame_total, i_frame, i_frame_total, fps, bitrate,
                 eta/3600, (eta/60)%60, eta%60 );
    }
    else
    {
        sprintf( buf, "x264 %d frames: %.2f fps, %.2f kb/s", i_frame, fps, bitrate );
    }
    fprintf( stderr, "%s  \r", buf+5 );
    SetConsoleTitle( buf );
    fflush( stderr ); // needed in windows
    return i_time;
}

static void convert_cli_to_lib_pic( x264_picture_t *lib, cli_pic_t *cli )
{
    memcpy( lib->img.i_stride, cli->img.stride, sizeof(cli->img.stride) );
    memcpy( lib->img.plane, cli->img.plane, sizeof(cli->img.plane) );
    lib->img.i_plane = cli->img.planes;
    lib->img.i_csp = cli->img.csp;
    lib->i_pts = cli->pts;
}

#define FAIL_IF_ERROR2( cond, ... )\
if( cond )\
{\
    x264_cli_log( "x264", X264_LOG_ERROR, __VA_ARGS__ );\
    retval = -1;\
    goto fail;\
}

int encode_frames()
{
    x264_picture_t pic;
    cli_pic_t cli_pic;
    int     retval = 0;
    int     b_ctrl_c = 0;
    encoder_datas* datas = &en_data;
        
    if( filter.get_frame( opt.hin, &cli_pic, datas->i_frame + opt.i_seek ) )
         return -1;
    
 
    if( filter.release_frame( opt.hin, &cli_pic, datas->i_frame + opt.i_seek ) )
         return -1;
    datas->i_frame++;
    return 0;   
     
        x264_picture_init( &pic );
        convert_cli_to_lib_pic( &pic, &cli_pic );
        
     

     //   if( !param->b_vfr_input )
            pic.i_pts = datas->i_frame;
            

       /* if( opt->i_pulldown && !param->b_vfr_input )
        {
            pic.i_pic_struct = pulldown->pattern[ i_frame % pulldown->mod ];
            pic.i_pts = (int64_t)( pulldown_pts + 0.5 );
            pulldown_pts += pulldown_frame_duration[pic.i_pic_struct];
        }
        else if( opt->timebase_convert_multiplier )
            pic.i_pts = (int64_t)( pic.i_pts * opt->timebase_convert_multiplier + 0.5 );
*/
        if( pic.i_pts <= datas->largest_pts )
        {
            if( cli_log_level >= X264_LOG_DEBUG || datas->pts_warning_cnt < MAX_PTS_WARNING )
                x264_cli_log( "x264", X264_LOG_WARNING, "non-strictly-monotonic pts at frame %d (%"PRId64" <= %"PRId64")\n",
                             datas->i_frame, pic.i_pts, datas->largest_pts );
            else if( datas->pts_warning_cnt == MAX_PTS_WARNING )
                x264_cli_log( "x264", X264_LOG_WARNING, "too many nonmonotonic pts warnings, suppressing further ones\n" );
            datas->pts_warning_cnt++;
            pic.i_pts = datas->largest_pts + datas->ticks_per_frame;
        }

        datas->second_largest_pts = datas->largest_pts;
        datas->largest_pts = pic.i_pts;

        
        datas->prev_dts = datas->last_dts;
        
        datas->i_frame_size = encode_frame( datas->h, opt.hout, &pic, &datas->last_dts );
     
        if( datas->i_frame_size < 0 )
        {
            //b_ctrl_c = 1; /* lie to exit the loop */
            retval = -1;
        }
        else if( datas->i_frame_size )
        {
            datas->i_file += datas->i_frame_size;
            datas->i_frame_output++;
            if( datas->i_frame_output == 1 )
                datas->first_dts = datas->prev_dts = datas->last_dts;
        }

        if( filter.release_frame( opt.hin, &cli_pic, datas->i_frame + opt.i_seek ) )
            return -1;
     

        /* update status line (up to 1000 times per input file) */
     //   if( opt->b_progress )
       //     i_previous = print_status( i_start, i_previous, i_frame_output, param->i_frame_total, i_file, param, 2 * last_dts - prev_dts - first_dts );
        datas->i_frame++;
        
                
        while(!b_ctrl_c && x264_encoder_delayed_frames( datas->h ))
        {
            printf("encoder.x264 %d delayed frames found! \n", x264_encoder_delayed_frames( datas->h ));
            break;
            //flush delayed frames ! beendet den encoder!
            datas->prev_dts = datas->last_dts;
            datas->i_frame_size = encode_frame( datas->h, opt.hout, NULL, &datas->last_dts );
            if( datas->i_frame_size < 0 )
            {
               b_ctrl_c = 1; /* lie to exit the loop */
               printf("encoder.x264 Fehler: i_frame_size is less than zero\n");
                retval = -1;
            }
            else if( datas->i_frame_size )
            {
                printf("encode frame size: %d\n", datas->i_frame_size);
                datas->i_file += datas->i_frame_size;
                datas->i_frame_output++;
                if( datas->i_frame_output == 1 )
                    datas->first_dts = datas->prev_dts = datas->last_dts;
                
                datas->i_frame++;
            }
           // if( opt->b_progress && i_frame_output )
             //   i_previous = print_status( i_start, i_previous, i_frame_output, param->i_frame_total, i_file, param, 2 * last_dts - prev_dts - first_dts );
        
        }
fail:
    if(  en_data.pts_warning_cnt >= MAX_PTS_WARNING && cli_log_level < X264_LOG_DEBUG )
        x264_cli_log( "x264", X264_LOG_WARNING, "%d suppressed nonmonotonic pts warnings\n", en_data.pts_warning_cnt-MAX_PTS_WARNING );

     
        return retval;
}
//! \brief return oldest frame from stack (FILO)
//!
//! get the oldest frame from stack, return pointer to data,<br>
//! data will delete by next call of getFrame
//! \param size pointer to var for size of frame, if NULL, only last frame will be deleted
//! \return pointer to framedata with size write into size, framedate changed after next call
//! \return 0 by error or if param size is 0
unsigned char* getFrame(int *size)
{
  //  encode_frames(&opt);
   // return NULL;
    static SFrame* current = NULL;
    if(current) delete_frame(current);
    current = NULL;

    if(size)
    {
        
        if(!g_FrameBuffer || g_FrameBuffer->count < 2) return 0;
        stack_pop(g_FrameBuffer, &current);
        //return 0;
        //printf("encoder.x264::getFrame stack-count: %d, current. %d\n", g_FrameBuffer->count, (int)current);
        if(!current) return 0;
 
        
        //return NULL;
        *size = current->size;
        return current->data;
    }
    return 0;
}
/*
int getFrame()
{
    return encode_frames(&opt);
}
*/
//! \brief close filter and file-handles
//! \return 0
int encoder_stop_frames()
{
    ///* clean up handles 
   if( filter.free && opt.hin){
        filter.free( opt.hin );
        opt.hin = NULL;
   }
   else if( opt.hin )
   {
       input.close_file( opt.hin );
       opt.hin = NULL;
   }
   
    if( opt.hout )
    {
        output.close_file( opt.hout, en_data.largest_pts, en_data.second_largest_pts );
        opt.hout = NULL;
    }
    if( opt.tcfile_out )
    {
        fclose( opt.tcfile_out );
        opt.tcfile_out = NULL;
    }
    if( opt.qpfile )
    {
        fclose( opt.qpfile );
        opt.qpfile = NULL;
    }
    //*/
    return 0;
}

//! \return 0
static int encode( x264_param_t *param, cli_opt_t *opt )
{
        
    //const cli_pulldown_t *pulldown = NULL; // shut up gcc

    //double  pulldown_pts = 0;
    int     retval = 0;

    opt->b_progress &= param->i_log_level < X264_LOG_DEBUG;
    
    /* set up pulldown */
    
    en_data.h = x264_encoder_open( param );
    FAIL_IF_ERROR2( !en_data.h, "x264_encoder_open failed\n" );

    x264_encoder_parameters( en_data.h, param );

    FAIL_IF_ERROR2( output.set_param( opt->hout, param ), "can't set outfile param\n" );

    en_data.i_start = x264_mdate();

    /* ticks/frame = ticks/second / frames/second */
    en_data.ticks_per_frame = (int64_t)param->i_timebase_den * param->i_fps_den / param->i_timebase_num / param->i_fps_num;
    FAIL_IF_ERROR2(  en_data.ticks_per_frame < 1 && !param->b_vfr_input, "ticks_per_frame invalid: %"PRId64"\n", en_data.ticks_per_frame )
    en_data.ticks_per_frame = X264_MAX( en_data.ticks_per_frame, 1 );
    if( !param->b_repeat_headers )
    {
        // Write SPS/PPS/SEI
        x264_nal_t *headers;
        int i_nal;

        FAIL_IF_ERROR2( x264_encoder_headers( en_data.h, &headers, &i_nal ) < 0, "x264_encoder_headers failed\n" )
        FAIL_IF_ERROR2( (en_data.i_file = output.write_headers( opt->hout, headers )) < 0, "error writing headers to output file\n" );
    }
    
    
    /* Encode frames */
   // for( ; !b_ctrl_c && (en_data.i_frame < param->i_frame_total || !param->i_frame_total); en_data.i_frame++ )
    //{
      
    //}
    /* Flush delayed frames */
/*    while( !b_ctrl_c && x264_encoder_delayed_frames( h ) )
    {
        prev_dts = last_dts;
        i_frame_size = encode_frame( h, opt->hout, NULL, &last_dts );
        if( i_frame_size < 0 )
        {
            b_ctrl_c = 1; // lie to exit the loop 
            retval = -1;
        }
        else if( i_frame_size )
        {
            i_file += i_frame_size;
            i_frame_output++;
            if( i_frame_output == 1 )
                first_dts = prev_dts = last_dts;
        }
        if( opt->b_progress && i_frame_output )
            i_previous = print_status( i_start, i_previous, i_frame_output, param->i_frame_total, i_file, param, 2 * last_dts - prev_dts - first_dts );
    }
   // */
fail:
    if(  en_data.pts_warning_cnt >= MAX_PTS_WARNING && cli_log_level < X264_LOG_DEBUG )
        x264_cli_log( "x264", X264_LOG_WARNING, "%d suppressed nonmonotonic pts warnings\n", en_data.pts_warning_cnt-MAX_PTS_WARNING );

   /* // duration algorithm fails when only 1 frame is output 
    if( i_frame_output == 1 )
        duration = (double)param->i_fps_den / param->i_fps_num;
    else if( b_ctrl_c )
        duration = (double)(2 * last_dts - prev_dts - first_dts) * param->i_timebase_num / param->i_timebase_den;
    else
        duration = (double)(2 * largest_pts - second_largest_pts) * param->i_timebase_num / param->i_timebase_den;

    i_end = x264_mdate();
    */
    // Erase progress indicator before printing encoding stats. 
    //if( opt->b_progress )
      //  fprintf( stderr, "                                                                               \r" );
   
    //fprintf( stderr, "\n" );

    //if( b_ctrl_c )
      //  fprintf( stderr, "aborted at input frame %d, output frame %d\n", opt->i_seek + i_frame, i_frame_output );

    
    //opt->hout = NULL;
/*
    if( i_frame_output > 0 )
    {
        double fps = (double)i_frame_output * (double)1000000 /
                     (double)( i_end - i_start );

        fprintf( stderr, "encoded %d frames, %.2f fps, %.2f kb/s\n", i_frame_output, fps,
                 (double) i_file * 8 / ( 1000 * duration ) );
    }
 //*/

    return retval;
}
