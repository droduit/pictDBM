/**
 * @file pictDBM.c
 * @brief pictDB Manager: Interpréteur de ligne de commande pour les commandes de pictDB.
 *
 * Outils de gestion d'une base de données d'images
 *
 * @author Mia Primorac, Dominique Roduit, Thierry Treyer
 * @date 2 Nov 2015
 */

#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>

#include "pictDB.h"
#include "pictDBM_tools.h"

#define LAST_COMMAND_MAPPING(cmd) \
    (cmd.name == NULL || cmd.function == NULL)

int do_list_cmd (int argc, char* argv[]);
int do_create_cmd (int argc, char* argv[]);
int help (int argc, char* argv[]);
int do_delete_cmd (int argc, char* argv[]);
int do_insert_cmd (int argc, char* argv[]);
int do_read_cmd (int argc, char* argv[]);
int do_gc_cmd (int argc, char *argv[]);

typedef int (*command)(int argc, char* argv[]);

/**
 * @brief Structure représentant une commande (nom + pointeur sur sa fonction)
 */
typedef struct command_mapping {
    const char *name;
    command function;
} command_mapping;

// Tableau des commandes disponibles
const command_mapping commands[] = {
    { "list", do_list_cmd },
    { "create", do_create_cmd },
    { "help", help },
    { "delete", do_delete_cmd },
    { "insert", do_insert_cmd },
    { "read", do_read_cmd },
    { "gc", do_gc_cmd },
    { NULL, NULL }
};

/********************************************************************//**
 * MAIN
 */
int main (int argc, char* argv[])
{
    int ret = ERR_NONE;

    if (VIPS_INIT(argv[0]))
        vips_error_exit("unable to start VIPS");

    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        argc--;
        argv++; // saute la première commande (le nom du programme)

        int cmd_defined = 0, i = 0;
        while (!LAST_COMMAND_MAPPING(commands[i])) {
            command_mapping cmd = commands[i];

            if (!strcmp(cmd.name, argv[0])) {
                cmd_defined = 1;
                ret = cmd.function(argc, argv);
                break;
            }

            i++;
        }

        if (!cmd_defined)
            ret = ERR_INVALID_COMMAND;
    }

    if (ret != ERR_NONE) {
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
        (void)help(argc, argv);
    }

    vips_shutdown();

    return ret;
}

/********************************************************************//**
 * Ouvre le fichier pictDB et appel la commande do_list.
 ********************************************************************** */
int do_list_cmd (int argc, char* argv[])
{
    if (argc < 2)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    const char* filename = argv[1];
    struct pictdb_file myfile;

    int retval = do_open(filename, "rb", &myfile);
    if (retval != ERR_NONE)
        return retval;

    do_list(&myfile, STDOUT);

    do_close(&myfile);

    return retval;
}

/********************************************************************//**
 * Prépare et appel la commande do_create.
********************************************************************** */
int do_create_cmd (int argc, char* argv[])
{
    if (argc < 2)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    const char* filename = argv[1];

    // Valeurs par défaut
    uint32_t max_files = 10;
    uint16_t thumb_res[2] = { 64, 64 };
    uint16_t small_res[2] = { 256, 256 };

    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-max_files")) {
            int mfi = i + 1; // Max Files Index

            if (mfi >= argc)
                return ERR_NOT_ENOUGH_ARGUMENTS;

            max_files = atouint32(argv[mfi]);

            if (max_files <= 0 || max_files > MAX_MAX_FILES)
                return ERR_MAX_FILES;

            i = mfi;
        } else if (!strcmp(argv[i], "-small_res")) {
            int sxri = i + 1, syri = i + 2; // Small X/Y Res Index

            if (sxri >= argc || syri >= argc)
                return ERR_NOT_ENOUGH_ARGUMENTS;

            small_res[0] = atouint16(argv[sxri]);
            small_res[1] = atouint16(argv[syri]);

            if (small_res[0] <= 0 || small_res[0] > MAX_SMALL_RES)
                return ERR_RESOLUTIONS;

            if (small_res[1] <= 0 || small_res[1] > MAX_SMALL_RES)
                return ERR_RESOLUTIONS;

            i = syri;
        } else if (!strcmp(argv[i], "-thumb_res")) {
            int txri = i + 1, tyri = i + 2; // Thumb X/Y Res Index

            if (txri >= argc || tyri >= argc)
                return ERR_NOT_ENOUGH_ARGUMENTS;

            thumb_res[0] = atouint16(argv[txri]);
            thumb_res[1] = atouint16(argv[tyri]);

            if (thumb_res[0] <= 0 || thumb_res[0] > MAX_THUMB_RES)
                return ERR_RESOLUTIONS;

            if (thumb_res[1] <= 0 || thumb_res[1] > MAX_THUMB_RES)
                return ERR_RESOLUTIONS;

            i = tyri;
        } else {
            return ERR_INVALID_ARGUMENT;
        }
    }

    struct pictdb_file db_file;
    db_file.header.max_files = max_files;

    db_file.header.res_resized[0] = thumb_res[0];
    db_file.header.res_resized[1] = thumb_res[1];
    db_file.header.res_resized[2] = small_res[0];
    db_file.header.res_resized[3] = small_res[1];

    int retval = do_create(filename, &db_file);

    if (retval == ERR_NONE) {
        print_header(&db_file.header);
        do_close(&db_file);
    }

    return retval;
}


/********************************************************************//**
 * Affiche l'aide a propos des commandes disponibles
 ********************************************************************** */
int help (int argc, char* argv[])
{
    printf("pictDBM [COMMAND] [ARGUMENTS]\n");
    printf("  help: displays this help.\n");
    printf("  list <dbfilename>: list pictDB content.\n");
    printf("  create <dbfilename> [options] : create a new pictDB.\n");
    printf("      options are:\n");
    printf("          -max_files <MAX_FILES>: maximum number of files.\n");
    printf("                                  default value is 10\n");
    printf("                                  maximum value is 100000\n");
    printf("          -thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n");
    printf("                                  default value is 64x64\n");
    printf("                                  maximum value is 128x128\n");
    printf("          -small_res <X_RES> <Y_RES>: resolution for small images.\n");
    printf("                                  default value is 256x256\n");
    printf("                                  maximum value is 512x512\n");
    printf("  read    <dbfilename> <pictID> [original|orig|thumbnail|thumb|small]:\n");
    printf("      read an image from the pictDB and save it to a file.\n");
    printf("      default resolution is \"original\".\n");
    printf("  insert <dbfilename> <pictID> <filename>: insert a new image in the pictDB.\n");
    printf("  delete <dbfilename> <pictID> : delete picture pictID from pictDB.\n");
    printf("  gc <dbfilename> <tmp dbfilename>: performs garbage collecting on pictDB. Requires a temporary filename for copying the pictDB.\n");
    return ERR_NONE;
}

/********************************************************************//**
 * Supprime une image de la base de donnée
 */
int do_delete_cmd (int argc, char* argv[])
{
    if (argc < 3)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    const char* filename = argv[1];
    const char* pictID = argv[2];

    if (pictID == NULL || strlen(pictID) > MAX_PIC_ID)
        return ERR_INVALID_PICID;

    int retval = ERR_NONE;
    struct pictdb_file db_file;

    retval = do_open(filename, "r+b", &db_file);
    if (retval != ERR_NONE)
        return retval;

    retval = do_delete(pictID, &db_file);

    do_close(&db_file);

    return retval;
}

/********************************************************************//**
 * Insert une image dans la base de donnée.
 */
int do_insert_cmd (int argc, char* argv[])
{
    if (argc < 3)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    const char* dbfilename = argv[1];
    const char* pictID     = argv[2];
    const char* filename   = argv[3];

    if (pictID == NULL || strlen(pictID) > MAX_PIC_ID)
        return ERR_INVALID_PICID;

    // Variables utilisées ou libérées en cas d'erreur
    int retval = ERR_NONE;
    void *image = NULL;
    struct pictdb_file db_file;

    retval = do_open(dbfilename, "r+b", &db_file);
    if (retval != ERR_NONE)
        goto error;

    if (db_file.header.num_files >= db_file.header.max_files) {
        retval = ERR_FULL_DATABASE;
        goto error;
    }

    size_t image_size = 0;
    retval = read_disk_image(filename, &image, &image_size);
    if (retval != ERR_NONE)
        goto error;

    retval = do_insert(image, image_size, pictID, &db_file);
    if (retval != ERR_NONE)
        goto error;

    free(image);
    do_close(&db_file);

    return ERR_NONE;

error:
    do_close(&db_file);

    if (image != NULL)
        free(image);

    return retval;
}

/********************************************************************//**
 * Lis une image de la base de donnée.
 */
int do_read_cmd (int argc, char* argv[])
{
    if (argc < 3)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    // Variables utilisées ou libérées en cas d'erreur
    int retval = ERR_NONE;
    const char *name = NULL;
    char *image_buffer = NULL;
    struct pictdb_file db_file;

    // Récupération des arguments
    const char* dbfilename = argv[1];
    const char* pict_id = argv[2];

    if (pict_id == NULL || strlen(pict_id) > MAX_PIC_ID) {
        retval = ERR_INVALID_PICID;
        goto error;
    }

    int res = resolution_atoi( (argc > 3) ? argv[3] : "original" );
    if (res == -1) {
        retval = ERR_RESOLUTIONS;
        goto error;
    }

    // Lecture de l'image depuis la DB
    retval = do_open(dbfilename, "r+b", &db_file);
    if (retval != ERR_NONE)
        goto error;

    uint32_t image_size = 0;

    retval = do_read(pict_id, (uint32_t)res, &image_buffer, &image_size, &db_file);
    if (retval != ERR_NONE)
        goto error;

    // Écriture de l'image trouvée
    name = create_name(pict_id, (uint32_t)res);
    if (name == NULL) {
        retval = ERR_OUT_OF_MEMORY;
        goto error;
    }

    retval = write_disk_image(name, image_buffer, image_size);
    if (retval != ERR_NONE)
        goto error;

    // Nettoyage
    free((char*)name);
    free(image_buffer);

    do_close(&db_file);

    return ERR_NONE;

error:
    do_close(&db_file);

    if (name != NULL)
        free((void*)name);

    if (image_buffer != NULL)
        free(image_buffer);

    return retval;
}

/********************************************************************//**
 * Nettoye la base d'images (garbage collecting)
 ********************************************************************** */
int do_gc_cmd (int argc, char *argv[])
{
    if (argc < 3)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    // Récupération des arguments
    const char* dbfilename = argv[1];
    const char* tmpFilename = argv[2];

    struct pictdb_file db_file;

    do_open(dbfilename, "r+b", &db_file);
    do_gbcollect(&db_file, dbfilename, tmpFilename);
    do_close(&db_file);

    return ERR_NONE;
}
