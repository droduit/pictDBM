/**
 * @file db_create.c
 * @brief pictDB library: do_create implementation.
 *
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <string.h> // for strncpy
#include <stdlib.h> // for calloc

/********************************************************************//**
 * Creates the database called db_filename. Writes the header and the
 * preallocated empty metadata array to database file.
 */
int do_create(const char* filename, struct pictdb_file* db_file)
{
    int ret = ERR_NONE;

    // Initialisation du header
    strncpy(db_file->header.db_name, CAT_TXT,  MAX_DB_NAME);
    db_file->header.db_name[MAX_DB_NAME] = '\0';

    db_file->header.db_version = 0;
    db_file->header.num_files = 0;
    db_file->header.unused_32 = 0;
    db_file->header.unused_64 = 0;

    // Initialisation des métadatas
	db_file->metadata = calloc(db_file->header.max_files, sizeof(struct pict_metadata));
    if(db_file->metadata == NULL)
		return ERR_OUT_OF_MEMORY;
		
    db_file->fpdb = fopen(filename, "w");
    if (db_file->fpdb == NULL)
        return ERR_IO;

    ret = do_write(db_file);

    do_close(db_file);

    if (ret == ERR_NONE)
        printf("%d item(s) written\n", db_file->header.max_files);

    return ret;
}
