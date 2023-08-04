/**
 * @file do_list.c
 * @brief Implementation de la commande do_list
 *
 * @author Dominique Roduit, Thierry Treyer
 * @date 9 Mar 2016
 */

#include <string.h>
#include <json-c/json.h>

#include "pictDB.h"

/* Fonctions "privées" pour do_list */
const char* do_list_stdout (const struct pictdb_file* db_file);
const char* do_list_json (const struct pictdb_file* db_file);

/********************************************************************//**
 * Affiche complet de la base de donnée
 */
const char* do_list (const struct pictdb_file* db_file, enum do_list_mode mode)
{
    const char *response = NULL;

    switch (mode) {
    case STDOUT:
        response = do_list_stdout(db_file);
        break;
    case JSON:
        response = do_list_json(db_file);
        break;
    default:
        response = "unimplemented do_list mode";
    }

    return response;
}

const char* do_list_stdout (const struct pictdb_file* db_file)
{
    const struct pictdb_header header = db_file->header;

    print_header(&header);

    if (header.num_files == 0) {
        puts("<< empty database >>");
    } else {
        uint32_t i = 0, num_files = 0;
        while (i < header.max_files && num_files < header.num_files) {
            struct pict_metadata metadata = db_file->metadata[i];

            if (metadata.is_valid == NON_EMPTY) {
                print_metadata(&metadata);
                num_files++;
            }

            i++;
        }
    }

    return NULL;
}

const char* do_list_json (const struct pictdb_file* db_file)
{
    const struct pictdb_header header = db_file->header;

    const char*  response = NULL;
    json_object* json_response = json_object_new_object();
    json_object* pictures = json_object_new_array();

    uint32_t i = 0, num_files = 0;
    while (i < header.max_files && num_files < header.num_files) {
        struct pict_metadata metadata = db_file->metadata[i];

        if (metadata.is_valid == NON_EMPTY) {
            json_object* pict_id = json_object_new_string(metadata.pict_id);

            json_object_array_add(pictures, pict_id);

            num_files++;
        }

        i++;
    }

    json_object_object_add(json_response, "Pictures", pictures);

    const char *json_buffer = json_object_to_json_string(json_response);
    if (json_buffer == NULL)
        goto error;

    size_t response_length = strlen(json_buffer);
    response = calloc(response_length + 1, sizeof(char));
    if (response == NULL)
        goto error;

    strcpy((char*)response, json_buffer);

    json_object_put(json_response); // Free

    return response;

error:
    if (json_response != NULL)
        json_object_put(json_response);

    if (response != NULL)
        free((void*)response);

    return NULL;
}
