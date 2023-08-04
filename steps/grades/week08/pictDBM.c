/**
 * @file pictDBM.c
 * @brief pictDB Manager: command line interpretor for pictDB core commands.
 *
 * Picture Database Management Tool
 *
 * @date 2 Nov 2015
 */

#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>

#include "pictDB.h"
#include "pictDBM_tools.h"

// -0: Please avoid cryptic code-shortenning macros, it may look cool, but it doesn't worth the pain.
// The idea of having a delimiter at the end is nice.
#define LAST_COMMAND_MAPPING(cmd) \
    (cmd.name == NULL || cmd.function == NULL)

int do_list_cmd (int argc, char* argv[]);
int do_create_cmd (int argc, char* argv[]);
int help (int argc, char* argv[]);
int do_delete_cmd (int argc, char* argv[]);

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
    { NULL, NULL }
};

/********************************************************************//**
 * MAIN
 */
int main (int argc, char* argv[])
{
    int ret = 0;

    if (VIPS_INIT(argv[0]))
        vips_error_exit("unable to start VIPS");

    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        argc--;
        argv++; // skips command call name

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

    if (ret != 0) {
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
        (void)help(argc, argv);
    }

    vips_shutdown();

    return ret;
}

/********************************************************************//**
 * Opens pictDB file and calls do_list command.
 ********************************************************************** */
int do_list_cmd (int argc, char* argv[])
{
	if (argc < 2)
		return ERR_NOT_ENOUGH_ARGUMENTS;
		
	const char* filename = argv[1];
    struct pictdb_file myfile;

    int retval = do_open(filename, "rb", &myfile);

    if (retval != 0)
        return retval;
	
    do_list(&myfile);

    do_close(&myfile);

    return retval;
}

/********************************************************************//**
 * Prepares and calls do_create command.
********************************************************************** */
int do_create_cmd (int argc, char* argv[])
{
	if (argc < 2)
		return ERR_NOT_ENOUGH_ARGUMENTS;
	
	const char* filename = argv[1];
	
    // Valeurs par défaut
    // -0: You shouldn't use hardcoded values.
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

    if (retval == 0)
        print_header(&db_file.header);

    return retval;
}

/********************************************************************//**
 * Displays some explanations.
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
    printf("  delete <dbfilename> <pictID> : delete picture pictID from pictDB.\n");

    return 0;
}

/********************************************************************//**
 * Deletes a picture from the database.
 */
int do_delete_cmd (int argc, char* argv[])
{
	if (argc < 3)
		return ERR_NOT_ENOUGH_ARGUMENTS;
		
	const char* filename = argv[1];
	const char* pictID = argv[2];
	
    int retval = 0;
    if (pictID == NULL || strlen(pictID) > MAX_PIC_ID)
        return ERR_INVALID_PICID;

    struct pictdb_file db_file;

    retval = do_open(filename, "r+b", &db_file);
    if (retval != 0)
        return retval;

    retval = do_delete(pictID, &db_file);

    do_close(&db_file);

    return retval;
}
