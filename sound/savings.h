#ifndef _SAVINGS_H
#define _SAVINGS_H

/**************************************************************************
 *
 *  SAVINGS: Simple AVI (No Good) Streamer
 *
 *  This AVI module is designed for streaming AVI files with _only_one_
 *  graphics stream.  The 'rec ' LIST is not implemented (which is used
 *  for interleaved graphics and audio).  Using this module with AVI files
 *  it is not designed for may cause computer crashes and other mayhem,
 *  so use this module carefully...
 *
 *  Original code was written for AVI Motion JPEG decoding on SGI IRIX 6.5
 *  on O2/mvp/vice hardware by Lars J. Aas <larsa@sim.no>, September 1999.
 *
 *  Copyright (C) 1999 by Systems in Motion <URL:http://www.sim.no/>.
 * 
 **************************************************************************/

#include "tariff.h"

/**************************************************************************/

typedef struct riff_avi_s {
    char pathname[128];

    struct _aviheader { /* AVI HEADER */
        int usecs_frame;
        int max_bps;
        int pad_gran;
        int flags;
        int total_frames;
        int initial_frames;
        int streams;
        int bufsize;
        int width;
        int height;
        int scale;
        int rate;
        int start;
        int length;
    } avi;

    struct _streamheader { /* GRAPHICS STREAM HEADER */
        int type;
        int handler;
        int flags;
        int priority;
        int init_frames;
        int scale;
        int rate;
        int start;
        int length;
        int bufsize;
        int quality;
        int sample_size;
    } stream;

    struct _formatheader { /* GRAPHICS STREAM FORMAT */
        int size;
        int width;
        int height;
        int planes;
        int bit_cnt;
        int compression;
        int image_size;
        int xpels_meter;
        int ypels_meter;
        int num_colors;
        int imp_colors;
    } format;

/* private: */
    int frame;

    riff_t * file;

    void * avih;
    void * strh;
    void * strf;
} riff_avi_t;

/**************************************************************************/

riff_avi_t * avi_file_open( const char * const pathname );
int avi_file_close( riff_avi_t * avifile );

int avi_readfirst( riff_avi_t * avifile, void * buf, int off, int * len );
int avi_readnext( riff_avi_t * avifile, void * buf, int off, int * len );

void * find_jpeg_chunk( void * chunk, int num, int size );

void avi_dumpheaders( riff_avi_t * avifile );
void avi_shortinfo( riff_avi_t * avifile );

/**************************************************************************/

/* The following info is lifted from xanim's xa_avi.h */

/* RIFF:AVI 'avih' chunk words: */
#define RIFF_AVI_avih_USECS        0
#define RIFF_AVI_avih_MAX_BPS      1
#define RIFF_AVI_avih_PAD_GRAN     2
#define RIFF_AVI_avih_FLAGS        3
#define RIFF_AVI_avih_TOT_FRAMES   4
#define RIFF_AVI_avih_INIT_FRAMES  5
#define RIFF_AVI_avih_STREAMS      6
#define RIFF_AVI_avih_BUF_SIZE     7
#define RIFF_AVI_avih_WIDTH        8
#define RIFF_AVI_avih_HEIGHT       9
#define RIFF_AVI_avih_SCALE       10
#define RIFF_AVI_avih_RATE        11
#define RIFF_AVI_avih_START       12
#define RIFF_AVI_avih_LENGTH      13

/* flag bits for avih->flags: */
#define AVI_avih_HAS_INDEX          0x00000010
#define AVI_avih_MUST_USE_INDEX     0x00000020
#define AVI_avih_IS_INTERLEAVED     0x00000100
#define AVI_avih_WAS_CAPTUREFILE    0x00010000
#define AVI_avih_COPYRIGHTED        0x00020000

/* RIFF:AVI 'strh' chunk words: */
#define RIFF_AVI_strh_FCC_TYPE     0
#define RIFF_AVI_strh_FCC_HANDLER  1
#define RIFF_AVI_strh_FLAGS        2
#define RIFF_AVI_strh_PRIORITY     3
#define RIFF_AVI_strh_INIT_FRAMES  4
#define RIFF_AVI_strh_SCALE        5
#define RIFF_AVI_strh_RATE         6
#define RIFF_AVI_strh_START        7
#define RIFF_AVI_strh_LENGTH       8
#define RIFF_AVI_strh_BUF_SIZE     9
#define RIFF_AVI_strh_QUALITY     10
#define RIFF_AVI_strh_SAMPLE_SIZE 11

/* flag bits for strh->flags */
#define AVI_strh_DISABLED           0x00000001
#define AVI_strh_VIDEO_PALCHANGES   0x00010000

/* RIFF:AVI 'strf' chunk words for 'vids' FCC_TYPE: */
#define RIFF_AVI_vids_SIZE         0
#define RIFF_AVI_vids_WIDTH        1
#define RIFF_AVI_vids_HEIGHT       2
#define RIFF_AVI_vids_PLANES_s     6    /* OBS: short word */
#define RIFF_AVI_vids_BIT_COUNT_s  7    /* OBS: short word */
#define RIFF_AVI_vids_COMPRESSION  4
#define RIFF_AVI_vids_IMAGE_SIZE   5
#define RIFF_AVI_vids_XPELS_METER  6
#define RIFF_AVI_vids_YPELS_METER  7
#define RIFF_AVI_vids_NUM_COLORS   8
#define RIFF_AVI_vids_IMP_COLORS   9

/**************************************************************************/

#endif /* ! _SAVINGS_H */
