/*
 * @file error.c
 * @brief Messages d'erreurs pour pictDB.
 *
 * @author Mia Primorac, Thierry Treyer, Dominique Roduit
 * @date 2 Nov 2015
 */

#include "error.h"

const char * const ERROR_MESSAGES[] = {
    "(no error)", // Pas d'erreur
    "I/O Error",
    "(re|m|c)alloc failled",
    "Not enough arguments",
    "Invalid filename",
    "Invalid command",
    "Invalid argument",
    "Invalid max_files number",
    "Invalid resolution(s)",
    "Invalid picture ID",
    "Full database",
    "File not found",
    "Not implemented",
    "Existing picture ID",
    "Vips error",
    "Unable to start listener",
    "Internal error",
    "Invalid or missing parameter",
    "Debug"
};

