#ifndef _TARIFF_H
#define _TARIFF_H

/**************************************************************************
 *
 *  TARIFF: Trivial Access to RIFF
 *
 *  This module is designed for low-level access to RIFF files.
 *
 *  Original code was written for AVI file streaming on SGI IRIX 6.5 by
 *  Lars J. Aas <larsa@sim.no>, September 1999.
 *
 *  Copyright (C) 1999 by Systems in Motion <URL:http://www.sim.no/>.
 *
 **************************************************************************/

#define RIFF_CHUNK_HEADER_SIZE     8
#define RIFF_LIST_HEADER_SIZE     12

#define RIFF_PEEKLEN              12

/**************************************************************************/

typedef struct riff_s {
    char magic[12];
    char peek[RIFF_PEEKLEN];
    int pos;
    int mark;
    int size;
    int fd;
} riff_t;

/**************************************************************************/

riff_t * riff_file_open( const char * const pathname );
void     riff_file_close( riff_t * file );
int      riff_file_is_type( riff_t * file, const char * const type );
int      riff_file_size( riff_t * file );

/* file "traverse" */
void *   riff_chunk_read( riff_t * file );
int      riff_chunk_read_data( riff_t * file, char * buf, int off );
void     riff_chunk_skip( riff_t * file );
int      riff_chunk_enter( riff_t * file );
int      riff_list_enter( riff_t * file );
int      riff_chunk_longword( void * chunk, int word );
int      riff_chunk_shortword( void * chunk, int word );

void     riff_filepos_mark( riff_t * file );
void     riff_filepos_goto_mark( riff_t * file );

/* peek ahead to next chunk */
int      riff_chunk_is_type( void * chunk, const char * const type );
int      riff_chunk_is_list_type( void * list, const char * const type );
int      riff_chunk_size( void * chunk );
int      riff_list_size( void * chunk );

int      riff_next_chunk_is_type( riff_t * file, const char * const type );
int      riff_next_chunk_is_list_type( riff_t * file, const char * const type );
int      riff_next_chunk_size( riff_t * file );
int      riff_next_list_size( riff_t * file );

void     riff_chunk_free( void * chunk );
void     riff_list_free( void * chunk );

/* debug info methods */
void     riff_chunk_info( void * chunk, int words );
void     riff_peek_info( riff_t * file );

/**************************************************************************/

#endif /* ! _TARIFF_H */
