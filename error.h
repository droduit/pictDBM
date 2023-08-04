/**
 * @file error.h
 * @brief Codes d'erreurs et messages pour pictDB.
 *
 * @author Mia Primorac, Thierry Treyer, Dominique Roduit
 * @date 2 Nov 2015
 */

#ifndef PICTDBPRJ_ERROR_H
#define PICTDBPRJ_ERROR_H

/**
 * @brief Codes d'erreurs pour la librairie interne pictDB.
 *
 */
enum error_codes {
    ERR_NONE = 0,
    ERR_IO,
    ERR_OUT_OF_MEMORY,
    ERR_NOT_ENOUGH_ARGUMENTS,
    ERR_INVALID_FILENAME,
    ERR_INVALID_COMMAND,
    ERR_INVALID_ARGUMENT,
    ERR_MAX_FILES,
    ERR_RESOLUTIONS,
    ERR_INVALID_PICID,
    ERR_FULL_DATABASE,
    ERR_FILE_NOT_FOUND,
    NOT_IMPLEMENTED,
    ERR_DUPLICATE_ID,
    ERR_VIPS,
    ERR_BIND,
    ERR_INTERNAL,
    ERR_INVALID_PARAM,
    ERR_DEBUG
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Message d'erreurs pour la librairie interne pictDB.
 *
 */
extern
const char * const ERROR_MESSAGES[];

#ifdef __cplusplus
}
#endif
#endif
