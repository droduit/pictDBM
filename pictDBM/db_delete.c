/**
 * @file do_delete.c
 * @brief Implementation de la commande do_delete
 *
 * @author Dominique Roduit, Thierry Treyer
 * @date 04 Apr 2016
 */

#include "pictDB.h"
#include <string.h>

/********************************************************************//**
 * Suppression d'une image
 */
int do_delete(const char* id, struct pictdb_file* db_file)
{
    int haschanged = 0;
    uint32_t num_files = 0, i = 0;
    // Recherche du fichier dans les metadata, puis supression
    while (i < db_file->header.max_files && num_files < db_file->header.num_files) {
        struct pict_metadata *metadata = &db_file->metadata[i];

        if (metadata->is_valid == NON_EMPTY) {
            num_files++;

            if (strcmp(metadata->pict_id, id) == 0) {
                // Reset à zero de cette metadata
                memset(metadata, 0, sizeof(struct pict_metadata));

                haschanged = 1;
                break;
            }
        }

        i++;
    }

    // Aucune image trouvée
    if (!haschanged)
        return ERR_FILE_NOT_FOUND;

    // Mise à jour du header
    db_file->header.num_files--;
    db_file->header.db_version++;

    int retval = do_write(db_file, NULL);

    return retval;
}
