/**
 * @file image_content.c
 * @brief Implementation de la fonction lazily_resize
 *
 * @author Dominique Roduit, Thierry Treyer
 * @date 11 Avr 2016
 */

#include "pictDB.h"
#include "image_content.h"

// ---------------------------------------------------------------------
int lazily_resize(struct pictdb_file* db_file, const size_t index, const uint32_t res)
{
    // Contrôle des arguments
    int err = check_image_exists(db_file, index, RES_ORIG);
    if (err != ERR_NONE)
        return err;

    if (res == RES_ORIG)
        return ERR_NONE;

    if (db_file->metadata[index].offset[res] > 0)
        return ERR_NONE;

    // Init des ressources qui doivent être libérée en cas d'erreur
    void *buf_orig = NULL, *buf_resized = NULL;
    VipsImage *image_orig = NULL, *image_resized = NULL;

    // Récupération des données de l'image originelle
    err = fetch_image(db_file, index, RES_ORIG, &buf_orig);
    if (err != ERR_NONE)
        goto error;

    // Image originelle passée à VIPS
    image_orig = vips_image_new_from_buffer(buf_orig, db_file->metadata[index].size[RES_ORIG], NULL, NULL);
    if (image_orig == NULL) {
        err = ERR_VIPS;
        goto error;
    }

    // Calcul du ratio de redimensionnement
    double ratio = resize_ratio(image_orig, db_file->header.res_resized[2 * res], db_file->header.res_resized[2 * res + 1]);

    // Redimensionnement
    err = vips_resize(image_orig, &image_resized, ratio, NULL);
    if (err != ERR_NONE) {
        err = ERR_VIPS;
        goto error;
    }

    // Export du resultat en JPEG
    size_t len_resized = 0;
    err = vips_jpegsave_buffer(image_resized, &buf_resized, &len_resized, NULL);
    if (err != ERR_NONE) {
        err = ERR_VIPS;
        goto error;
    }

    // Écriture du résultat
    err = store_image(db_file, index, res, buf_resized, (uint32_t)len_resized);
    if (err != ERR_NONE)
        goto error;

    // Libération des ressources
    g_free(buf_resized);
    g_object_unref(image_resized);
    g_object_unref(image_orig);
    free(buf_orig);

    return ERR_NONE;

error:
    if (buf_orig != NULL)
        free(buf_orig);

    if (image_orig != NULL)
        g_object_unref(image_orig);

    if (image_resized != NULL)
        g_object_unref(image_resized);

    if (buf_resized != NULL)
        g_free(buf_resized);

    return err;
}

// ---------------------------------------------------------------------
double resize_ratio(const VipsImage *image, const uint16_t max_width, const uint16_t max_height)
{
    const double w_ratio = (double)max_width  / (double)image->Xsize;
    const double h_ratio = (double)max_height / (double)image->Ysize;

    return w_ratio > h_ratio ? h_ratio : w_ratio;
}

// ---------------------------------------------------------------------
int fetch_image(const struct pictdb_file* db_file, const size_t index, const uint32_t res, void **buf)
{
    int error = check_image_exists(db_file, index, res);
    if (error != ERR_NONE)
        return error;

    const struct pict_metadata *file = &db_file->metadata[index];

    // Lecture de l'image depuis le fichier
    *buf = calloc(1, file->size[res]);
    if (*buf == NULL)
        return ERR_OUT_OF_MEMORY;

    long retval = fseek(db_file->fpdb, (long)file->offset[res], SEEK_SET);
    if (retval != 0) {
        free(*buf);
        return ERR_IO;
    }

    retval = (long)fread(*buf, file->size[res], 1, db_file->fpdb);
    if (retval != 1) {
        free(*buf);
        return ERR_IO;
    }

    return ERR_NONE;
}

// ---------------------------------------------------------------------
int store_image(struct pictdb_file* db_file, const size_t index, const uint32_t res, const void *buf, const uint32_t len)
{
    int error = ERR_NONE;
    if (res != RES_ORIG) {
        error = check_image_exists(db_file, index, RES_ORIG);
        if (error != ERR_NONE)
            return error;
    }

    if (len == 0)
        return ERR_NONE;

    // Récupération du future offset de l'image
    error = fseek(db_file->fpdb, 0, SEEK_END);
    if (error != ERR_NONE)
        return ERR_IO;

    long offset = ftell(db_file->fpdb);
    if (offset == -1)
        return ERR_IO;

    // Écriture de l'image et des metadatas
    error = (int)fwrite(buf, len, 1, db_file->fpdb);
    if (error != 1)
        return ERR_IO;

    db_file->metadata[index].size[res] = len;
    db_file->metadata[index].offset[res] = (uint64_t)offset;

    error = do_write(db_file, NULL);
    if (error != ERR_NONE)
        return error;

    return ERR_NONE;
}

// ---------------------------------------------------------------------
int check_image_exists(const struct pictdb_file* db_file, const size_t index, const uint32_t res)
{
    if (db_file == NULL)
        return ERR_INVALID_ARGUMENT;

    if (db_file->fpdb == NULL)
        return ERR_INVALID_ARGUMENT;

    if (index >= db_file->header.max_files)
        return ERR_INVALID_ARGUMENT;

    if (res >= NB_RES)
        return ERR_RESOLUTIONS;

    struct pict_metadata file = db_file->metadata[index];
    if (file.is_valid == EMPTY)
        return ERR_FILE_NOT_FOUND;

    if (file.offset[res] == 0 || file.size[res] == 0)
        return ERR_FILE_NOT_FOUND;

    return ERR_NONE;
}

// ---------------------------------------------------------------------
int get_resolution(uint32_t* height, uint32_t* width, const char* image_buffer, size_t image_size)
{
    VipsImage* out = vips_image_new();

    int ret = vips_jpegload_buffer((void*)image_buffer, image_size, &out, NULL);
    if (ret != 0 || out == NULL)
        return ERR_VIPS;

    *width = (uint32_t)out->Xsize;
    *height = (uint32_t)out->Ysize;

    g_object_unref(out);

    return ERR_NONE;
}
