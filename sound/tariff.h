#ifndef SMALLCHANGE_TARIFF_H
#define SMALLCHANGE_TARIFF_H

/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2002 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.  See the
 *  file LICENSE.GPL at the root directory of this source distribution
 *  for more details.
 *
 *  If you desire to use SmallChange with software that is incompatible
 *  licensewise with the GPL, and / or you would like to take
 *  advantage of the additional benefits with regard to our support
 *  services, please contact Systems in Motion about acquiring a Coin
 *  Professional Edition License.  See <URL:http://www.coin3d.org> for
 *  more information.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  <URL:http://www.sim.no>, <mailto:support@sim.no>
 *
\**************************************************************************/

/**************************************************************************
 *
 *  TARIFF: Trivial Access to RIFF
 *
 *  This module is designed for low-level access to RIFF files.
 *
 *  Original code was written for AVI file streaming on SGI IRIX 6.5 by
 *  Lars J. Aas <larsa@sim.no>, September 1999.
 *  The code has been modified by Thomas Hammer <thammer@sim.no>
 *  - some debugging and some convenience functions.
 *
 **************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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
    int at_eof;
} riff_t;

/**************************************************************************/

riff_t * riff_file_open( const char * const pathname );
void     riff_file_close( riff_t * file );
int      riff_file_is_type( riff_t * file, const char * const type );
int      riff_file_size( riff_t * file );

/* file "traverse" */
void *   riff_chunk_read( riff_t * file );
int      riff_chunk_read_data( riff_t * file, char * buf, int off );
int      riff_chunk_read_data_ex( riff_t * file, char * buf, int readsize);
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

#ifdef __cplusplus
}
#endif

#endif /* !SMALCHANGE_TARIFF_H */
