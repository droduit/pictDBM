/**
 * @file dedup.c
 * @brief Dé-duplication des images pour éviter qu'une même image (même contenu)
 * 		  soit présente plusieurs fois dans la base.
 *
 * @author Dominique Roduit, Thierry Treyer
 * @date 29 Avr 2016
 */

#include "pictDB.h"
#include "dedup.h"

#include <stdlib.h> // pour calloc
#include <string.h> // pour strcmp

/********************************************************************/
int shacmp(const unsigned char *sha1, const unsigned char *sha2)
{
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        int diff = sha1[i] - sha2[i];

        if (diff != 0)
            return diff;
    }

    return 0;
}

/********************************************************************/
int do_name_and_content_dedup(struct pictdb_file* db_file, const uint32_t index)
{
    if (index > db_file->header.max_files)
        return ERR_INVALID_ARGUMENT;

    struct pict_metadata *to_check = &db_file->metadata[index];

    // Parcours des images...
    uint32_t i = 0, num_files = 0;
    while (i < db_file->header.max_files && num_files < db_file->header.num_files) {
        struct pict_metadata *metadata = &db_file->metadata[i];

        if (i++ == index || metadata->is_valid == EMPTY)
            continue;

        num_files++;

        if (!strcmp(to_check->pict_id, metadata->pict_id))
            return ERR_DUPLICATE_ID;

        // Gestion des duplicatas
        if (!shacmp(to_check->SHA, metadata->SHA)) {
            for (size_t res = 0; res < NB_RES; res++) {
                to_check->size[res] = metadata->size[res];
                to_check->offset[res] = metadata->offset[res];
            }

            return ERR_NONE;
        }
    }

    to_check->offset[RES_ORIG] = 0;

    return ERR_NONE;
}
