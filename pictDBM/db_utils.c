/* **
 * @file db_utils.c
 * @brief Implémentation de plusieurs fonctions utilitaires pour pictDB
 *
 * @author Mia Primorac, Dominique Roduit, Thierry Treyer
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <stdint.h> // pour uint8_t
#include <stdio.h> // pour sprintf
#include <stdlib.h> // pour calloc
#include <string.h> // pour strcmp
#include <inttypes.h> // pour PRI...
#include <openssl/sha.h> // pour SHA256_DIGEST_LENGTH

/********************************************************************//**
 * SHA lisible par un humain
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
 * affichage du header de la base de donnée
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
 * Affichage d'une métadata
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
    enum error_codes err = ERR_NONE;

    db_file->fpdb = NULL;
    db_file->metadata = NULL;

    db_file->fpdb = fopen(db_filename, mode);
    if (db_file->fpdb == NULL) {
        err = ERR_IO;
        goto error;
    }

    // Lecture du header
    retval = fread(&db_file->header, sizeof(struct pictdb_header), 1, db_file->fpdb);
    if (retval != 1) {
        err = ERR_IO;
        goto error;
    }

    if(db_file->header.max_files >= MAX_MAX_FILES)
        goto error;

    // Allocation et lecture des métadonnées
    db_file->metadata = calloc(db_file->header.max_files, sizeof(struct pict_metadata));
    if(db_file->metadata == NULL) {
        err = ERR_OUT_OF_MEMORY;
        goto error;
    }

    retval = fread(db_file->metadata, sizeof(struct pict_metadata), db_file->header.max_files, db_file->fpdb);
    if (retval != db_file->header.max_files) {
        err = ERR_IO;
        goto error;
    }

    return ERR_NONE;

error:
    do_close(db_file);

    return err;
}

/********************************************************************/
void do_close(struct pictdb_file* db_file)
{
    // Si le fichier de la base de données existe, on le ferme
    if (db_file->fpdb != NULL)
        fclose(db_file->fpdb);

    db_file->fpdb = NULL;

    if (db_file->metadata != NULL)
        free(db_file->metadata);

    db_file->metadata = NULL;
}

/********************************************************************/
int do_write(const struct pictdb_file* db_file, size_t *items_written)
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

    if (items_written != NULL)
        *items_written += retval;

    // Ecriture des metadatas
    retval = fwrite(db_file->metadata, sizeof(struct pict_metadata), db_file->header.max_files, db_file->fpdb);
    if (retval != db_file->header.max_files)
        return ERR_IO;

    if (items_written != NULL)
        *items_written += retval;

    return ERR_NONE;
}

/********************************************************************/
int resolution_atoi(const char* res)
{
    if (res == NULL)
        return -1;

    if(!strcmp(res, "thumb") || !strcmp(res, "thumbnail"))
        return RES_THUMB;
    else if(!strcmp(res, "small"))
        return RES_SMALL;
    else if(!strcmp(res, "orig") || !strcmp(res, "original"))
        return RES_ORIG;
    else
        return -1;
}

/********************************************************************/
const char* create_name(const char* pict_id, uint32_t res)
{
    if (pict_id == NULL || res >= NB_RES)
        return NULL;

    const char* suffixes[] = { "thumb", "small", "orig" };
    const char* suffix = suffixes[res];

    size_t id_length = strlen(pict_id);
    size_t suffix_length = strlen(suffix);
    size_t total_length = id_length + suffix_length + NAME_SUFFIX_LENGTH;

    // Allocation et formattage du nom
    char *name = calloc(total_length, sizeof(char));
    if (name == NULL)
        return NULL;

    size_t ret = (size_t)snprintf(name, total_length, "%s_%s.jpg", pict_id, suffix);
    if (ret != total_length - 1) {
        free(name);
        return NULL;
    }

    name[total_length - 1] = '\0';

    return name;
}

/********************************************************************/
int read_disk_image(const char *filename, void **image, size_t *image_size)
{
    int ret = ERR_NONE;

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        ret = ERR_IO;
        goto error;
    }

    // Récupération de la taille de l'image
    ret = fseek(file, 0, SEEK_END);
    if (ret != ERR_NONE) {
        ret = ERR_IO;
        goto error;
    }

    long tmp_size = ftell(file);
    if (tmp_size == -1) {
        ret = ERR_IO;
        goto error;
    }

    *image_size = (size_t)tmp_size;

    // Allocation et lecture de l'image
    *image = calloc(1, *image_size);
    if (*image == NULL) {
        ret = ERR_OUT_OF_MEMORY;
        goto error;
    }

    ret = fseek(file, 0, SEEK_SET);
    if (ret != ERR_NONE) {
        ret = ERR_IO;
        goto error;
    }

    ret = (int)fread(*image, *image_size, 1, file);
    if (ret != 1) {
        ret = ERR_IO;
        goto error;
    }

    fclose(file);

    return ERR_NONE;

error:
    if (file != NULL)
        fclose(file);

    if (*image != NULL)
        free(*image);

    return ret;
}

/********************************************************************/
int write_disk_image(const char *filename, const void *image, const size_t image_size)
{
    if (filename == NULL || image == NULL || image_size == 0)
        return ERR_INVALID_ARGUMENT;

    int ret = ERR_NONE;

    FILE *file = fopen(filename, "wb");
    if (file == NULL)
        return ERR_IO;

    ret = (int)fwrite(image, image_size, 1, file);
    ret = (ret == 1) ? ERR_NONE : ERR_IO; // ret redevient une enum error

    fclose(file);

    return ret;
}
