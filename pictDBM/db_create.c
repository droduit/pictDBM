/**
 * @file db_create.c
 * @brief Implémentatoin de la commande  do_create
 *
 * @author Mia Primorac, Dominique Roduit, Thierry Treyer
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <string.h> // for strncpy
#include <stdlib.h> // for calloc

/********************************************************************//**
 * Créé la base de donnée appelée db_filename. Ecrit le header et le
 * tableau de metadata vide préalloué dans le fichier de base de donnée.
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

    db_file->metadata = NULL;
    db_file->fpdb = NULL;

    // Initialisation des métadatas
    db_file->metadata = calloc(db_file->header.max_files, sizeof(struct pict_metadata));
    if(db_file->metadata == NULL) {
        ret = ERR_OUT_OF_MEMORY;
        goto error;
    }

    db_file->fpdb = fopen(filename, "w");
    if (db_file->fpdb == NULL) {
        ret = ERR_IO;
        goto error;
    }

    size_t items_written = 0;
    ret = do_write(db_file, &items_written);

    if (ret == ERR_NONE)
        printf("%ld item(s) written\n", items_written);

    return ret;

error:
    do_close(db_file);

    return ret;
}
