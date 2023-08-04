/**
 * @file pictDB_server.c
 * @brief Implémentation du serveur Web pictDB
 * @author Dominique Roduit, Thierry Treyer
 * @date 16 Mai 2015
 */

#include <signal.h>
#include <vips/vips.h>

#include "mongoose.h"
#include "pictDB.h"

#define LISTEN_ADDR "localhost"
#define LISTEN_PORT "8000"
#define MAX_QUERY_PARAM 5
#define MAX_QUERY_LENGTH ((MAX_PIC_ID + 1) * MAX_QUERY_PARAM - 1)

#define LAST_HANDLE_MAPPING(cmd) \
    (cmd.uri == NULL || cmd.function == NULL)

/**
 * @brief Affiche l'aide d'utilisation du serveur
 */
int help (struct mg_connection *nc, struct http_message *hm);

/**
 * @brief Retourne un code d'erreur HTTP
 * @param nc La connexion qui recevra l'erreur
 * @param error Le code de l'erreur
 */
void mg_error(struct mg_connection* nc, int error);

/**
 * @brief Envoie la liste des images
 * @param nc La connexion demandant la liste des images
 * @param hm Le contenu de la requête demandant la liste des images
 * @return ERR_NONE si tout s'est bien passé, sinon le code d'erreur approprié
 */
int handle_list_call (struct mg_connection *nc, struct http_message *hm);

/**
 * @brief Envoie une image demandée
 * @param nc La connexion demandant une image
 * @param hm Le contenu de la requête demandant une image
 * @return ERR_NONE si tout s'est bien passé, sinon le code d'erreur approprié
 */
int handle_read_call (struct mg_connection *nc, struct http_message *hm);

/**
 * @brief Insert l'image donnée
 * @param nc La connexion insérant une image
 * @param hm Le contenu de la requête insérant une image
 * @return ERR_NONE si tout s'est bien passé, sinon le code d'erreur approprié
 */
int handle_insert_call (struct mg_connection *nc, struct http_message *hm);

/**
 * @brief Supprime une image demandée
 * @param nc La connexion supprimant une image
 * @param hm Le contenu de la requête supprimant une image
 * @return ERR_NONE si tout s'est bien passé, sinon le code d'erreur approprié
 */
int handle_delete_call (struct mg_connection *nc, struct http_message *hm);

/**
 * @brief Sépare les paramètres de la query_string
 * @param result Nombre maximum de paramètre que nous accepterons
 * @param tmp chaine de taille (MAX_PIC_ID + 1) * MAX_QUERY_PARAM qui contient l'ensemble des parties de la query string
 * @param src query string a découper
 * @param delim contient la liste de délimiteurs de token (&= signie que les car. & et = séparent les paramètres)
 * @param len Longeur de la query string à découper
 */
void split (char* result[], char* tmp, const char* src, const char* delim, size_t len);

typedef int (*handle)(struct mg_connection *nc, struct http_message *hm);

typedef struct handle_mapping {
    const char *uri;
    handle function;
} handle_mapping;

// Tableau des handles disponibles
static const handle_mapping handles[] = {
    { "/pictDB/list", handle_list_call },
    { "/pictDB/read", handle_read_call },
    { "/pictDB/insert", handle_insert_call },
    { "/pictDB/delete", handle_delete_call },
    { NULL, NULL }
};

static int signal_received = 0;
static struct mg_serve_http_opts http_server_opts;

static void signal_handler (int signum)
{
    signal(signum, signal_handler);
    signal_received = signum;
}

static void pictdb_handler (struct mg_connection* nc, int ev, void *p)
{
    // On ne gère qu'un cas, pas besoin de switch/case
    if (ev == MG_EV_HTTP_REQUEST) {
        int retval = ERR_NONE;
        int handle_defined = 0, i = 0;
        struct http_message *hm = (struct http_message*)p;

        while (!LAST_HANDLE_MAPPING(handles[i])) {
            handle_mapping handle = handles[i];

            if (!mg_vcmp(&hm->uri, handle.uri)) {
                handle_defined = 1;
                retval = handle.function(nc, hm);
                break;
            }

            i++;
        }

        if (retval != ERR_NONE)
            mg_error(nc, retval);
        else if (!handle_defined)
            mg_serve_http(nc, hm, http_server_opts);

        nc->flags |= MG_F_SEND_AND_CLOSE;
    }

}

int main (int argc, char *argv[])
{
    int retval = ERR_NONE;
    struct pictdb_file db_file = {
        .fpdb = NULL, .header = { }, .metadata = NULL
    };

    struct mg_mgr mgr;
    struct mg_connection *nc = NULL;

    // On initialise VIPS avant toutes allocations ou initialisations.
    if (VIPS_INIT(argv[0]))
        vips_error_exit("unable to start VIPS");

    /* Comme mg_mgr_init() ne peut pas échouer,
     * l'init est fait ici pour ne pas segfault en cas d'erreur.
     */
    mg_mgr_init(&mgr, &db_file);

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    if (argc < 2) {
        retval = ERR_NOT_ENOUGH_ARGUMENTS;
        goto error;
    }

    // Init pictDB
    retval = do_open(argv[1], "r+", &db_file);
    if (retval != ERR_NONE)
        goto error;

    print_header(&db_file.header);

    // Start listening
    nc = mg_bind(&mgr, LISTEN_PORT, pictdb_handler);
    if (nc == NULL) {
        retval = ERR_BIND;
        goto error;
    }

    mg_set_protocol_http_websocket(nc);

    http_server_opts.document_root = ".";

    // Polling
    while (!signal_received) {
        mg_mgr_poll(&mgr, 1000);
    }

    // Exciting
    printf("Exciting on signal %d\n", signal_received);

    mg_mgr_free(&mgr);
    do_close(&db_file);
    vips_shutdown();

    return ERR_NONE;

error:
    mg_mgr_free(&mgr);
    do_close(&db_file);
    vips_shutdown();

    fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[retval]);
    help(NULL, NULL);

    return retval;
}

int help (struct mg_connection *nc, struct http_message *hm)
{
    printf("pictDB_server <dbfilename>\n");

    return ERR_NONE;
}

void mg_error(struct mg_connection* nc, int error)
{
    /*
     * Il serait préférable d'envoyer une erreur 500 avec le message d'erreur.
     * Cependant, nous avons choisi ici d'afficher directement le message d'erreur
     * dans la page pour une meilleur expérience utilisateur.
     * On évite ainsi de changer de page pour pas grand chose.
     * Sans cette modification, le code demandé est :
     * 	mg_printf(nc,
    		   "HTTP/1.1 500 %s\r\n"
               "Content-Length: 0\r\n\r\n",
               ERROR_MESSAGES[error]);
     */
    mg_printf(nc,
              "HTTP/1.1 302 Found\r\n"
              "Location: /?error=%d\r\n\r\n", error);
}

int handle_list_call (struct mg_connection *nc, struct http_message *hm)
{
    // Récupération de la liste
    const char *response = do_list((struct pictdb_file*)nc->mgr->user_data, JSON);
    if (response == NULL)
        return ERR_INTERNAL;

    // Envoi de la liste
    size_t response_length = strlen(response);

    mg_send_head(nc, 200, (signed long)response_length, "Content-Type: application/json");
    mg_send(nc, response, (int)response_length);

    free((char*)response);

    return ERR_NONE;
}

int handle_read_call (struct mg_connection *nc, struct http_message *hm)
{
    int retval = ERR_NONE;

    // Gestion des paramètres
    char *result[MAX_QUERY_PARAM] = { NULL };
    char tmp[MAX_QUERY_LENGTH + 1] = { '\0' };

    split(result, tmp, hm->query_string.p, "&=", hm->query_string.len);

    int resolution = -1;
    char *pict_id = NULL;
    for (int i = 0; i < MAX_QUERY_PARAM; i += 2) {
        if (result[i] == NULL)
            break;

        if (!strcmp(result[i], "res"))
            resolution = resolution_atoi(result[i + 1]);
        else if (!strcmp(result[i], "pict_id"))
            pict_id = result[i + 1];
    }

    if (resolution == -1 || pict_id == NULL)
        return ERR_INVALID_PARAM;

    // Lecture de l'image
    char *image = NULL;
    uint32_t image_size = 0;
    retval = do_read(pict_id, (uint32_t)resolution, &image, &image_size, (struct pictdb_file*)nc->mgr->user_data);
    if (retval != ERR_NONE)
        return retval;

    // Envoi de l'image
    mg_send_head(nc, 200, (signed long)image_size, "Content-Type: image/jpeg");
    mg_send(nc, image, (int)image_size);

    free(image);

    return ERR_NONE;
}

int handle_insert_call (struct mg_connection *nc, struct http_message *hm)
{
    int retval = ERR_NONE;

    // Récupération de l'image dans le corps de la requête
    char varname[MAX_PIC_ID + 1] = { '\0' };
    size_t varname_length = MAX_PIC_ID;

    char pict_id[MAX_PIC_ID + 1] = { '\0' };
    size_t pict_id_length = MAX_PIC_ID;

    const char *image = NULL;
    size_t image_size = 0;

    mg_parse_multipart(hm->body.p, hm->body.len, varname, varname_length, pict_id, pict_id_length, &image, &image_size);

    if (image == NULL || image_size == 0)
        return ERR_INVALID_PARAM;

    // Insertion de la nouvelle image
    retval = do_insert(image, image_size, pict_id, (struct pictdb_file*)nc->mgr->user_data);
    if (retval != ERR_NONE)
        return retval;

    // Redirection vers l'accueil
    mg_printf(nc,
              "HTTP/1.1 302 Found\r\n"
              "Location: http://%s:%s/index.html\r\n\r\n", LISTEN_ADDR, LISTEN_PORT
             );

    return ERR_NONE;
}

int handle_delete_call (struct mg_connection *nc, struct http_message *hm)
{
    int retval = ERR_NONE;

    // Gestion des paramètres
    char *result[MAX_QUERY_PARAM] = { NULL };
    char tmp[MAX_QUERY_LENGTH + 1] = { '\0' };

    split(result, tmp, hm->query_string.p, "&=", hm->query_string.len);

    char *pict_id = NULL;
    for (int i = 0; i < MAX_QUERY_PARAM; i += 2) {
        if (result[i] == NULL)
            break;

        if (!strcmp(result[i], "pict_id"))
            pict_id = result[i + 1];
    }

    if (pict_id == NULL)
        return ERR_INVALID_PARAM;

    // Suppression de l'image
    retval = do_delete(pict_id, (struct pictdb_file*)nc->mgr->user_data);
    if (retval != ERR_NONE)
        return retval;

    // Redirection vers l'accueil
    mg_printf(nc,
              "HTTP/1.1 302 Found\r\n"
              "Location: http://%s:%s/index.html\r\n\r\n", LISTEN_ADDR, LISTEN_PORT
             );

    return ERR_NONE;
}

void split (char* result[], char* tmp, const char* src, const char* delim, size_t len)
{
    if (src == NULL || tmp == NULL || delim == NULL || len == 0)
        return; // On peut pas travailler sur ça !

    size_t input_length = (len < MAX_QUERY_LENGTH) ? len : MAX_QUERY_LENGTH; // Upper bound

    // Copie sécurisée de src
    strncpy(tmp, src, input_length);
    tmp[input_length] = '\0';

    // Parcours des tokens
    char* param = NULL;
    size_t param_index = 0;
    do {
        param = result[param_index] = strtok(tmp, delim);
        tmp = NULL;
    } while (param != NULL && ++param_index < MAX_QUERY_PARAM);
}
