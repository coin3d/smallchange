
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "savings.h"
#include "tariff.h"
#include "config.h"

/**************************************************************************/

riff_avi_t *
avi_file_open(
    const char * const pathname )
{
    riff_avi_t * handle;

    handle = (riff_avi_t *) malloc( sizeof( riff_avi_t ) );
    if ( ! handle ) {
        fprintf( stderr, "avi_file_open(): malloc error\n" );
        return NULL;
    }

    strcpy( handle->pathname, pathname );
    handle->avih = NULL;
    handle->strh = NULL;
    handle->strf = NULL;

    handle->file = riff_file_open( handle->pathname );
    if ( handle->file == NULL ) {
        free( handle );
        return NULL;
    }

    do {
        int hdrl_size;

        if ( riff_file_is_type( handle->file, "AVI " ) == 0 )
            break;

        if ( riff_next_chunk_is_list_type( handle->file, "hdrl" ) == FALSE )
            break;

        hdrl_size = riff_next_list_size( handle->file );
        riff_list_enter( handle->file );

        while ( hdrl_size > 0 ) {
            if ( riff_next_chunk_is_type( handle->file, "avih" ) ) {
                assert( handle->avih == NULL );
                hdrl_size -= riff_next_chunk_size( handle->file ) +
                    RIFF_CHUNK_HEADER_SIZE;
                handle->avih = riff_chunk_read( handle->file );
            } else if ( riff_next_chunk_is_list_type( handle->file, "strl" ) ) {
                int strl_size;
                strl_size = riff_next_list_size( handle->file );
                hdrl_size -= RIFF_LIST_HEADER_SIZE + strl_size;
                riff_list_enter( handle->file );
                while ( strl_size > 0 ) {
                    if ( riff_next_chunk_is_type( handle->file, "strh" ) ) {
                        strl_size -= riff_next_chunk_size( handle->file ) +
                            RIFF_CHUNK_HEADER_SIZE;
                        handle->strh = riff_chunk_read( handle->file );
                    } else if ( riff_next_chunk_is_type(handle->file,"strf") ) {
                        strl_size -= riff_next_chunk_size( handle->file ) +
                            RIFF_CHUNK_HEADER_SIZE;
                        handle->strf = riff_chunk_read( handle->file );
                    } else if ( riff_next_chunk_is_type(handle->file,"strn") ) {
                        /* string description can be skipped */
                        strl_size -= riff_next_chunk_size( handle->file ) +
                            RIFF_CHUNK_HEADER_SIZE;
                        riff_chunk_skip( handle->file );
                    } else {
                        fprintf( stderr, "'strl' not implemented:\n" );
                        riff_peek_info( handle->file );
                        strl_size -= riff_next_chunk_size( handle->file ) +
                            RIFF_CHUNK_HEADER_SIZE;
                        riff_chunk_skip( handle->file );
                    }
                }
            } else {
                fprintf( stderr, "Unknown 'hdrl' list chunk\n" );
                riff_peek_info( handle->file );
                hdrl_size -= riff_next_chunk_size( handle->file ) +
                    RIFF_CHUNK_HEADER_SIZE;
                riff_chunk_skip( handle->file );
            }
        }

        if ( ! handle->avih )
            break;

        handle->avi.usecs_frame =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_USECS );
        handle->avi.max_bps =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_MAX_BPS );
        handle->avi.pad_gran =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_PAD_GRAN );
        handle->avi.flags =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_FLAGS );
        handle->avi.total_frames =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_TOT_FRAMES );
        handle->avi.initial_frames =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_INIT_FRAMES );
        handle->avi.streams =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_STREAMS );
        handle->avi.bufsize =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_BUF_SIZE );
        handle->avi.width =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_WIDTH );
        handle->avi.height =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_HEIGHT );
        handle->avi.scale =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_SCALE );
        handle->avi.rate =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_RATE );
        handle->avi.start =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_START );
        handle->avi.length =
            riff_chunk_longword( handle->avih, RIFF_AVI_avih_LENGTH );

        handle->stream.type =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_FCC_TYPE );
        handle->stream.handler =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_FCC_HANDLER );
        handle->stream.flags =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_FLAGS );
        handle->stream.priority =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_PRIORITY );
        handle->stream.init_frames =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_INIT_FRAMES );
        handle->stream.scale =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_SCALE );
        handle->stream.rate =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_RATE );
        handle->stream.start =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_START );
        handle->stream.length =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_LENGTH );
        handle->stream.bufsize =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_BUF_SIZE );
        handle->stream.quality =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_QUALITY );
        handle->stream.sample_size =
            riff_chunk_longword( handle->strh, RIFF_AVI_strh_SAMPLE_SIZE );

        handle->format.size =
            riff_chunk_longword( handle->strf, RIFF_AVI_vids_SIZE );
        handle->format.width =
            riff_chunk_longword( handle->strf, RIFF_AVI_vids_WIDTH );
        handle->format.height =
            riff_chunk_longword( handle->strf, RIFF_AVI_vids_HEIGHT );
        handle->format.planes =
            riff_chunk_shortword( handle->strf, RIFF_AVI_vids_PLANES_s );
        handle->format.bit_cnt =
            riff_chunk_shortword( handle->strf, RIFF_AVI_vids_BIT_COUNT_s );
        handle->format.compression =
            riff_chunk_longword( handle->strf, RIFF_AVI_vids_COMPRESSION );
        handle->format.image_size =
            riff_chunk_longword( handle->strf, RIFF_AVI_vids_IMAGE_SIZE );
        handle->format.xpels_meter =
            riff_chunk_longword( handle->strf, RIFF_AVI_vids_XPELS_METER );
        handle->format.ypels_meter =
            riff_chunk_longword( handle->strf, RIFF_AVI_vids_YPELS_METER );
        handle->format.num_colors =
            riff_chunk_longword( handle->strf, RIFF_AVI_vids_NUM_COLORS );
        handle->format.imp_colors =
            riff_chunk_longword( handle->strf, RIFF_AVI_vids_IMP_COLORS );

        handle->frame = 0;

        if ( riff_next_chunk_is_type( handle->file, "JUNK" ) == TRUE )
            riff_chunk_skip( handle->file );

        if ( riff_next_chunk_is_list_type( handle->file, "movi" ) == FALSE )
            break;

/*        riff_peek_info( handle->file ); */
        riff_list_enter( handle->file );
        riff_filepos_mark( handle->file );

/*        riff_peek_info( handle->file ); */

        return handle;
    } while ( FALSE );

    /* break executed - open failed because of bad bits */
    avi_file_close( handle );
    return NULL;
} /* avi_file_open() */

int
avi_file_close(
    riff_avi_t * handle )
{
    if ( handle->avih ) riff_chunk_free( handle->avih );
    if ( handle->strh ) riff_chunk_free( handle->strh );
    if ( handle->strf ) riff_chunk_free( handle->strf );
    riff_file_close( handle->file );
    free( handle );
    return TRUE;
} /* avi_file_close() */

/**************************************************************************/

int
avi_readfirst(
    riff_avi_t * file,
    void * buf,
    int off,
    int * size )
{
    riff_filepos_goto_mark( file->file );
    /* in case either db (DIB) or dc (DIB compressed) is used */
    while ( ! riff_next_chunk_is_type( file->file, "00db" )
         && ! riff_next_chunk_is_type( file->file, "00dc" ) ) {
/*        fprintf( stderr, "skipping chunk: " ); */
/*        riff_peek_info( file->file ); */
        riff_chunk_skip( file->file );
    }
    if ( size )
        *size = riff_chunk_read_data( file->file, (char *) buf, off );
     else
        riff_chunk_read_data( file->file, (char *) buf, off );
    return TRUE;
} /* ops_avi_readfirst() */

/**************************************************************************/

int
avi_readnext(
    riff_avi_t * file,
    void * buf,
    int off,
    int * size )
{
    while ( ! riff_next_chunk_is_type( file->file, "00db" )
         && ! riff_next_chunk_is_type( file->file, "00dc" ) ) {
        fprintf( stderr, "skipping chunk: " );
/*        riff_peek_info( file->file ); */
        riff_chunk_skip( file->file );
    }
    if ( size )
        *size = riff_chunk_read_data( file->file, (char *) buf, off );
    else
        riff_chunk_read_data( file->file, (char *) buf, off );
    return TRUE;
} /* ops_avi_readnext() */

/**************************************************************************/

void *
find_jpeg_chunk(
    void * chunk,
    int num,
    int size )
{
    unsigned char * ret;
    unsigned char * ptr = (unsigned char *) chunk;

    while ( size &&
       (ret = (unsigned char *) memchr( (void *) ptr, 0xFF, size )) != NULL ) {
        size -= ((ret - ptr) + 1);
        ptr = ret + 1;
        if ( ret[1] == 0xDB ) {
            num--;
            if ( ! num ) {
                return (void *) ret;
            }
        }
    }
    return NULL;
} /* find_chunk() */

/**************************************************************************/

void
avi_dumpheaders(
    riff_avi_t * file )
{
    printf( "AVI headers for '%s':\n", file->pathname );

    printf( "CHUNK 'avih':\n" );
    riff_chunk_info( file->avih, -1 );
    printf( "\n"
        "    us_frames   = %d (1/%6.4f),\n"
        "    max_bps     = %d,\n"
        "    pad_gran    = %d,\n"
        "    flags       = 0x%x,\n"
        "    tot_frames  = %d,\n"
        "    init_frames = %d,\n"
        "    streams     = %d,\n"
        "    bufsize     = %d,\n"
        "    width       = %d,\n"
        "    height      = %d,\n"
        "    scale       = %d,\n"
        "    rate        = %d,\n"
        "    start       = %d,\n"
        "    length      = %d\n\n",
        file->avi.usecs_frame,
        (1.0/((float)file->avi.usecs_frame)*1000.0), file->avi.max_bps,
        file->avi.pad_gran, file->avi.flags, file->avi.total_frames,
        file->avi.initial_frames, file->avi.streams, file->avi.bufsize,
        file->avi.width, file->avi.height, file->avi.scale, file->avi.rate,
        file->avi.start, file->avi.length );

    printf( "CHUNK 'strh':\n" );
    riff_chunk_info( file->strh, -1 );
    printf( "\n"
        "    fcc_type     = 0x%08x\n"
        "    fcc_handler  = 0x%08x\n"
        "    flags        = 0x%x\n"
        "    priority     = %d\n"
        "    init_frames  = %d\n"
        "    scale        = %d\n"
        "    rate         = %d\n"
        "    start        = %d\n"
        "    length       = %d\n"
        "    bufsize      = %d\n"
        "    quality      = %d\n"
        "    sample_size  = %d\n\n",
        file->stream.type, file->stream.handler, file->stream.flags,
        file->stream.priority, file->stream.init_frames, file->stream.scale,
        file->stream.rate, file->stream.start, file->stream.length,
        file->stream.bufsize, file->stream.quality, file->stream.sample_size );

    printf( "CHUNK 'strf':\n" );
    riff_chunk_info( file->strf, -1 );
    printf( "\n"
        "    size         = %d\n"
        "    width        = %d\n"
        "    height       = %d\n"
        "    planes       = %d\n"
        "    bit_cnt      = %d\n"
        "    compression  = 0x%08x\n"
        "    image_size   = %d\n"
        "    xpels_meter  = %d\n"
        "    ypels_meter  = %d\n"
        "    num_colors   = %d\n"
        "    imp_colors   = %d\n\n",
        file->format.size, file->format.width, file->format.height,
        file->format.planes, file->format.bit_cnt, file->format.compression,
        file->format.image_size, file->format.xpels_meter,
        file->format.ypels_meter, file->format.num_colors,
        file->format.imp_colors );

} /* avi_dumpheaders() */

/**************************************************************************/

void
avi_shortinfo(
    riff_avi_t * file )
{
    printf( "File: %s (%d frames)\n", file->pathname, file->stream.length );
} /* avi_shortinfo() */

/**************************************************************************/
