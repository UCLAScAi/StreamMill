SUFFIXES =  .c .o

lib_LIBRARIES = libimdb.a

libimdb_a_SOURCES = im_rel.c im_cursor.c im_debug.c im_db_env_method.c im_db_err.c \
	hash-arbitrary.c mylib.c buffer.c

include_HEADERS = im_db.h im_db_buffer.h

datadir = $(prefix)/@DATADIRNAME@
CFLAGS = -g
INCLUDES = -I../BerkeleyDB/include



