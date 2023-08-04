/*
 * @file db_gbcollect.c
 * @brief Collect l'espace libre d'une pictDB.
 *
 * @author Dominique Roduit, Thierry Treyer
 * @date 2 Mai 2015
 */

#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <stdio.h>

#include "pictDB.h"
#include "image_content.h"

int do_gbcollect(struct pictdb_file* src, const char* src_name, const char* tmp_name)
{
    int retval = ERR_NONE;

    struct pictdb_file tmp;

    // Copie des valeurs de l'original dans la temporaire
    tmp.header.max_files = src->header.max_files;
    for (int i = 0; i < 2 * (NB_RES - 1); i++)
        tmp.header.res_resized[i] = src->header.res_resized[i];

    // Création d'une nouvelle pictdb temporaire
    retval = do_create(tmp_name, &tmp);
    if (retval != ERR_NONE)
        goto error;

    // Parcours de tous les fichiers valides de la pictdb source
    uint32_t srci = 0, tmpi = 0, num_files = 0;
    while (srci < src->header.max_files && num_files < src->header.num_files) {
        struct pict_metadata* srcmeta = &src->metadata[srci];
        struct pict_metadata* tmpmeta = &tmp.metadata[tmpi];

        // Pour chaque image valide (non supprimée)
        if (srcmeta->is_valid == NON_EMPTY) {
            void *image = NULL;
            uint32_t image_size = srcmeta->size[RES_ORIG];

            // Copie de l'original
            retval = fetch_image(src, srci, RES_ORIG, &image);
            if (retval != ERR_NONE)
                goto error;

            // Insertion dans la structure temporaire
            retval = do_insert(image, image_size, srcmeta->pict_id, &tmp);

            free(image);
            image = NULL;
            image_size = 0;

            if (retval != ERR_NONE)
                goto error;

            // Copie des petites images
            for (uint32_t res = RES_THUMB; res < RES_ORIG; res++) {
                // Pas de petites images dans src
                if (srcmeta->size[res] == 0 || srcmeta->offset[res] == 0)
                    continue;

                // Déjà des petites images dans tmp
                if (tmpmeta->size[res] != 0 && tmpmeta->offset[res] != 0)
                    continue;

                image_size = srcmeta->size[res];
                retval = fetch_image(src, srci, res, &image);
                if (retval != ERR_NONE)
                    goto error;

                retval = store_image(&tmp, tmpi, res, image, image_size);

                free(image);
                image = NULL;
                image_size = 0;

                if (retval != ERR_NONE)
                    goto error;
            }

            num_files++;
            tmpi++;
        }

        srci++;
    }

    tmp.header.db_version = src->header.db_version + 1;


    // Remplacement du fichier d'origine par le nouveau fichier nettoyé
    retval = remove(src_name);
    if(retval != 0)
        return ERR_IO;

    retval = rename(tmp_name, src_name);
    if(retval != 0)
        return ERR_IO;

    return ERR_NONE;

error:
    do_close(&tmp);

    return retval;
}
