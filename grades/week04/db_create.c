/**
 * @file db_create.c
 * @brief pictDB library: do_create implementation.
 *
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <string.h> // for strncpy

/********************************************************************//**
 * Creates the database called db_filename. Writes the header and the
 * preallocated empty metadata array to database file.
 */
/* **********************************************************************
 * TODO: ADD THE PROTOTYPE OF do_create HERE.
 * **********************************************************************
 */
int do_create(const char* filename, struct pictdb_file db_file) {
    // Initialisation du header
    strncpy(db_file.header.db_name, CAT_TXT,  MAX_DB_NAME);
    db_file.header.db_name[MAX_DB_NAME] = '\0';

    db_file.header.db_version = 0;
    db_file.header.num_files = 0;
    db_file.header.unused_32 = 0;
    db_file.header.unused_64 = 0;

    // Initialisation des métadatas
    memset(&db_file.metadata, 0, sizeof(struct pict_metadata) * db_file.header.max_files);

    db_file.fpdb = fopen(filename, "w");
    if (db_file.fpdb == NULL)
        return ERR_IO;

    // Ecriture du header
    fwrite(&db_file.header , sizeof(struct pictdb_header), 1, db_file.fpdb);
    // Ecriture des metadatas
    fwrite(db_file.metadata, sizeof(struct pict_metadata), db_file.header.max_files, db_file.fpdb);

    fclose(db_file.fpdb);

    printf("%d item(s) written\n", 1 + db_file.header.max_files);
    return 0;
}
