/**
 * @file pictDBM.c
 * @brief pictDB Manager: command line interpretor for pictDB core commands.
 *
 * Picture Database Management Tool
 *
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <stdlib.h>
#include <string.h>

int do_list_cmd (const char*);
int do_create_cmd (const char*);
int help (void);
int do_delete_cmd (const char*, const char*);

/********************************************************************//**
 * MAIN
 */
int main (int argc, char* argv[])
{
    int ret = 0;

    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        /* **********************************************************************
         * TODO WEEK 08: THIS PART SHALL BE REVISED THEN (WEEK 09) EXTENDED.
         * **********************************************************************
         */
        argc--;
        argv++; // skips command call name
        if (!strcmp("list", argv[0])) {
            if (argc < 2) {
                ret = ERR_NOT_ENOUGH_ARGUMENTS;
            } else {
                ret = do_list_cmd(argv[1]);
            }
        } else if (!strcmp("create", argv[0])) {
            if (argc < 2) {
                ret = ERR_NOT_ENOUGH_ARGUMENTS;
            } else {
                ret = do_create_cmd(argv[1]);
            }
        } else if (!strcmp("delete", argv[0])) {
            if (argc < 3) {
                ret = ERR_NOT_ENOUGH_ARGUMENTS;
            } else {
                ret = do_delete_cmd(argv[1], argv[2]);
            }
        } else if (!strcmp("help", argv[0])) {
            ret = help();
        } else {
            ret = ERR_INVALID_COMMAND;
        }
    }

    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
        (void)help();
    }

    return ret;
}

/********************************************************************//**
 * Opens pictDB file and calls do_list command.
 ********************************************************************** */
int do_list_cmd (const char* filename)
{
    struct pictdb_file myfile;

    int retval = do_open(filename, "rb", &myfile);
    if (retval != 0)
        return retval;

    do_list(&myfile);

    return retval;
}

/********************************************************************//**
 * Prepares and calls do_create command.
********************************************************************** */
int do_create_cmd (const char* filename)
{
    // This will later come from the parsing of command line arguments
    const uint32_t max_files =  10;
    const uint16_t thumb_res =  64;
    const uint16_t small_res = 256;
    struct pictdb_file db_file;
    db_file.header.max_files = max_files;

    db_file.header.res_resized[0] = thumb_res;
    db_file.header.res_resized[1] = thumb_res;
    db_file.header.res_resized[2] = small_res;
    db_file.header.res_resized[3] = small_res;

    int retval = do_create(filename, &db_file);

    if (retval == 0)
        print_header(&db_file.header);

    return retval;
}

/********************************************************************//**
 * Displays some explanations.
 ********************************************************************** */
int help (void)
{
    printf("pictDBM [COMMAND] [ARGUMENTS]\n");
    printf("  help: displays this help.\n");
    printf("  list <dbfilename>: list pictDB content.\n");
    printf("  create <dbfilename>: create a new pictDB.\n");
    printf("  delete <dbfilename> <pictID> : delete picture pictID from pictDB.\n");

    return 0;
}

/********************************************************************//**
 * Deletes a picture from the database.
 */
int do_delete_cmd (const char* filename, const char* pictID)
{
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
