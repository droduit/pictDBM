/*
 * @file db_insert.c
 * @brief Insertion d'une image dans la base pictDB
 *
 * @author Dominique Roduit, Thierry Treyer
 * @date 2 Mai 2015
 */

#include <stdlib.h> // pour calloc
#include <string.h> // pour strlen()
#include <openssl/sha.h> // pour SHA256_DIGEST_LENGTH and SHA256()

#include "pictDB.h"
#include "image_content.h"
#include "dedup.h"

/********************************************************************/
int do_insert(const char* img, size_t size, const char* pict_id, struct pictdb_file* db_file)
{
    if (db_file->header.num_files >= db_file->header.max_files)
        return ERR_FULL_DATABASE;

    uint32_t new_image_index = 0;

    // Recherche d'une position libre dans l'index
    for (new_image_index = 0; new_image_index < db_file->header.max_files; ++new_image_index) {
        if (db_file->metadata[new_image_index].is_valid == EMPTY)
            break;
    }

    // Pas nécessaire, mais sécurité supplémentaire
    if (new_image_index >= db_file->header.max_files)
        return ERR_FULL_DATABASE;

    struct pict_metadata *metadata = &db_file->metadata[new_image_index];

    // Initialisation des metadatas
    SHA256((const unsigned char*)img, size, (unsigned char*)&metadata->SHA);

    strncpy(metadata->pict_id, pict_id, MAX_PIC_ID);
    metadata->pict_id[MAX_PIC_ID] = '\0';

    metadata->size[RES_ORIG] = (uint32_t)size;
    metadata->is_valid = NON_EMPTY;

    // De-duplication de l'image
    int retval = do_name_and_content_dedup(db_file, new_image_index);
    if (retval != ERR_NONE)
        goto error;

    // Finalisation des metadatas et du header
    retval = get_resolution(&metadata->res_orig[1], &metadata->res_orig[0], img, size);
    if (retval != ERR_NONE)
        goto error;

    db_file->header.num_files++;

    // Ecriture de l'image sur le disque
    if (metadata->offset[RES_ORIG] == 0)
        retval = store_image(db_file, new_image_index, RES_ORIG, img, (uint32_t)size);
    else
        retval = do_write(db_file, NULL);

    return retval;

error:
    // Nettoyage des metadatas
    memset(metadata, 0, sizeof(struct pict_metadata));

    return retval;
}
