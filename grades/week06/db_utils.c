/* ** NOTE: undocumented in Doxygen
 * @file db_utils.c
 * @brief implementation of several tool functions for pictDB
 *
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <stdint.h> // for uint8_t
#include <stdio.h> // for sprintf
#include <inttypes.h> // for PRI...
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH

/********************************************************************//**
 * Human-readable SHA
 */
static void
sha_to_string (const unsigned char* SHA,
               char* sha_string)
{
    if (SHA == NULL) {
        return;
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(&sha_string[i*2], "%02x", SHA[i]);
    }

    sha_string[2*SHA256_DIGEST_LENGTH] = '\0';
}

/********************************************************************//**
 * pictDB header display.
 */
void print_header (const struct pictdb_header* header)
{
    printf("*****************************************\n");
    printf("**********DATABASE HEADER START**********\n");
    printf("DB NAME: %31s\n", header->db_name);
    printf("VERSION: %" PRIu32 "\n", header->db_version);
    printf("IMAGE COUNT: %" PRIu32 "\t\tMAX IMAGES: %" PRIu32 "\n", header->num_files, header->max_files);
    printf("THUMBNAIL: %" PRIu16 " x %" PRIu16 "\tSMALL: %" PRIu16 " x %" PRIu16 "\n",
           header->res_resized[0], header->res_resized[1], header->res_resized[2], header->res_resized[3]);
    printf("***********DATABASE HEADER END***********\n");
    printf("*****************************************\n");
}

/********************************************************************//**
 * Metadata display.
 */
void
print_metadata (const struct pict_metadata* metadata)
{
    char sha_printable[2*SHA256_DIGEST_LENGTH+1];
    sha_to_string(metadata->SHA, sha_printable);

    printf("PICTURE ID: %s\n", metadata->pict_id);
    printf("SHA: %s\n", sha_printable);
    printf("VALID: %" PRIu16 "\n", metadata->is_valid);
    printf("UNUSED: %" PRIu16 "\n", metadata->unused_16);
    printf("OFFSET ORIG. : %" PRIu64 "\t\tSIZE ORIG. : %" PRIu32 "\n", metadata->offset[RES_ORIG], metadata->size[RES_ORIG]);
    printf("OFFSET THUMB.: %" PRIu64 "\t\tSIZE THUMB.: %" PRIu32 "\n", metadata->offset[RES_THUMB], metadata->size[RES_THUMB]);
    printf("OFFSET SMALL : %" PRIu64 "\t\tSIZE SMALL : %" PRIu32 "\n", metadata->offset[RES_SMALL], metadata->size[RES_SMALL]);
    printf("ORIGINAL: %" PRIu32 " x %" PRIu32 "\n", metadata->res_orig[0], metadata->res_orig[1]);
    printf("*****************************************\n");
}

/********************************************************************/
int do_open(const char* db_filename, const char* mode, struct pictdb_file* db_file)
{
    size_t retval = 0;

    db_file->fpdb = fopen(db_filename, mode);

    if (db_file->fpdb == NULL)
        goto error;

    // Lecture du header
    retval = fread(&db_file->header, sizeof(struct pictdb_header), 1, db_file->fpdb);
    if (retval != 1)
        goto error;

    // Lecture des métadonnées
    retval = fread(db_file->metadata, sizeof(struct pict_metadata), db_file->header.max_files, db_file->fpdb);
    if (retval != db_file->header.max_files)
        goto error;

    return 0;

error:
    do_close(db_file);

    return ERR_IO;
}

/********************************************************************/
void do_close(struct pictdb_file* db_file)
{
    // Si le fichier de la base de données existe, on le ferme
    if (db_file->fpdb != NULL)
        fclose(db_file->fpdb);

    db_file->fpdb = NULL; // +0.5: Good catch!
}

int do_write(const struct pictdb_file* db_file)
{
    if (db_file->fpdb == NULL)
        return ERR_IO;

    size_t retval = 0;

    retval = (size_t)fseek(db_file->fpdb, 0, SEEK_SET);
    if (retval != 0)
        return ERR_IO;

    // Ecriture du header
    retval = fwrite(&db_file->header , sizeof(struct pictdb_header), 1, db_file->fpdb);
    if (retval != 1)
        return ERR_IO;

    // Ecriture des metadatas
    retval = fwrite(db_file->metadata, sizeof(struct pict_metadata), db_file->header.max_files, db_file->fpdb);
    if (retval != db_file->header.max_files)
        return ERR_IO;

    return 0;
}
