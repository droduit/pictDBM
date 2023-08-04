/*
 * @file db_read.c
 * @brief Lecture d'une image dans la base pictDB
 *
 * @author Dominique Roduit, Thierry Treyer
 * @date 2 Mai 2015
 */
#include <string.h> // for strcmp()

#include "pictDB.h"
#include "image_content.h"

/********************************************************************/
int do_read(const char* pict_id, uint32_t res, char** image_buffer, uint32_t* image_size, struct pictdb_file* db_file)
{

    if (pict_id == NULL)
        return ERR_INVALID_PICID;

    uint32_t image_index = 0;

    // On cherche l'entrée qui nous intéresse
    for (image_index = 0; image_index < db_file->header.max_files; ++image_index) {
        if (db_file->metadata[image_index].is_valid == EMPTY)
            continue;

        if (!strcmp(db_file->metadata[image_index].pict_id, pict_id))
            break;
    }

    // Si l'identifiant demandé n'a pas pu être trouvé
    if (image_index >= db_file->header.max_files)
        return ERR_FILE_NOT_FOUND;

    struct pict_metadata *metadata = &db_file->metadata[image_index];

    // Ne devrait normalement jamais arriver mais pour être sûr...
    if (res == RES_ORIG && metadata->offset[res] == 0)
        return ERR_FILE_NOT_FOUND;

    // Si l'image n'existe pas dans la résolution demandée on la créé
    int ret = ERR_NONE;
    if (metadata->offset[res] == 0) {
        ret = lazily_resize(db_file, image_index, res);
        if (ret != ERR_NONE)
            return ret;
    }

    ret = fetch_image(db_file, image_index, res, (void**)image_buffer);
    if (ret != ERR_NONE)
        return ret;

    *image_size = metadata->size[res];

    return ret;
}
