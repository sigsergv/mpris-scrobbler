/**
 * @author Marius Orcsik <marius@habarnam.ro>
 */
#ifndef MPRIS_SCROBBLER_API_H
#define MPRIS_SCROBBLER_API_H

#include "md5.h"

#include <inttypes.h>
#include <json-c/json.h>
#include <stdbool.h>

#define MIN_TRACK_LENGTH                30.0L // seconds
#define NOW_PLAYING_DELAY               65.0L //seconds

#define MAX_HEADER_LENGTH               256
#define MAX_HEADER_NAME_LENGTH          128
#define MAX_HEADER_VALUE_LENGTH         512
#define MAX_BODY_SIZE                   16384

#define CONTENT_TYPE_XML            "application/xml"
#define CONTENT_TYPE_JSON           "application/json"

#define API_HEADER_AUTHORIZATION_NAME "Authorization"
#define API_HEADER_AUTHORIZATION_VALUE_TOKENIZED "Token %s"

#define VALUE_SEPARATOR ", "

enum api_return_status {
    status_failed  = 0, // failed == bool false
    status_ok           // ok == bool true
};

struct api_response {
    struct api_error *error;
    enum api_return_status status;
};

typedef enum message_types {
    api_call_authenticate,
    api_call_now_playing,
    api_call_scrobble,
} message_type;

struct http_header {
    char name[MAX_HEADER_NAME_LENGTH];
    char value[MAX_HEADER_VALUE_LENGTH];
};

struct http_response {
    long code;
    char *body;
    size_t body_length;
    struct http_header **headers;
};

typedef enum http_request_types {
    http_get,
    http_post,
    http_put,
    http_head,
    http_patch,
} http_request_type;

#define MAX_SCHEME_LENGTH 5
#define MAX_HOST_LENGTH 512

struct api_endpoint {
    char scheme[MAX_SCHEME_LENGTH + 1];
    char host[MAX_HOST_LENGTH + 1];
    char path[FILE_PATH_MAX + 1];
};

struct http_request {
    http_request_type request_type;
    size_t body_length;
    struct api_endpoint *end_point;
    char *url;
    char *query;
    char *body;
    struct http_header **headers;
};

#include "audioscrobbler_api.h"
#include "listenbrainz_api.h"

#define HTTP_HEADER_CONTENT_TYPE "Content-Type"

static char *http_response_headers_content_type(const struct http_response *res)
{

    const size_t headers_count = arrlen(res->headers);
    for (size_t i = 0; i < headers_count; i++) {
        struct http_header *current = res->headers[i];

        if(strncmp(current->name, HTTP_HEADER_CONTENT_TYPE, strlen(HTTP_HEADER_CONTENT_TYPE)) == 0) {
             return current->value;
        }
    }
    return NULL;
}

#if 0
static void http_response_parse_json_body(struct http_response *res)
{
    if (NULL == res) { return; }
    if (res->code >= 500) { return; }

    json_object *obj = json_tokener_parse(res->body);
    if (NULL == obj) {
        _warn("json::parse_error");
    }
    if (json_object_array_length(obj) < 1) {
        _warn("json::invalid_json_message");
    }
    _trace("json::obj_cnt: %d", json_object_array_length(obj));
#if 0
    if (json_object_type(object, json_type_object)) {
        json_object_object_get_ex();
    }
#endif
}
#endif

char *api_get_url(const struct api_endpoint *endpoint)
{
    if (NULL == endpoint) { return NULL; }
    char *url = get_zero_string(MAX_URL_LENGTH);
    if (NULL == url) { return NULL; }

    strncat(url, endpoint->scheme, MAX_SCHEME_LENGTH+1);
    strncat(url, "://", 4);
    strncat(url, endpoint->host, MAX_URL_LENGTH+1);
    strncat(url, endpoint->path, MAX_URL_LENGTH+1);

    return url;
}

static char *http_request_get_url(const struct http_request *request)
{
    if (NULL == request) { return NULL; }
    char *url = get_zero_string(MAX_URL_LENGTH);
    if (NULL == url) { return NULL; }

    strncat(url, request->url, MAX_URL_LENGTH);
    if (NULL == request->query) {
        goto _return;
    }

    size_t query_len = strlen(request->query);
    if (query_len > 0) {
        strncat(url, "?", 2);
        strncat(url, request->query, query_len + 1);
    }

_return:
    return url;
}

static void api_endpoint_free(struct api_endpoint *api)
{
    free(api);
}

static size_t endpoint_get_scheme(char *result, const char *custom_url)
{
    if (NULL == result) { return 0; }

    const char *scheme = "https";
    if (NULL != custom_url && strncmp(custom_url, "http://", 7) == 0) {
        scheme = "http";
    }

    const size_t scheme_len = strlen(scheme);
    strncpy(result, scheme, scheme_len);

    return scheme_len;
}

static size_t endpoint_get_host(char *result, const enum api_type type, const enum end_point_type endpoint_type, const char *custom_url)
{
    if (NULL == result) { return 0; }

    const char* host = NULL;
    size_t host_len = 0;
    if (NULL != custom_url && strlen(custom_url) != 0) {
        bool url_has_scheme = false;
        size_t url_start = 0;
        if (strncmp(custom_url, "https://", 8) == 0) {
            url_has_scheme = true;
            url_start = 8;
        } else if (strncmp(custom_url, "http://", 7) == 0) {
            url_has_scheme = true;
            url_start = 7;
        }
        if (url_has_scheme) {
            host = (custom_url + url_start);
        } else {
            host = custom_url;
        }
        host_len = strlen(host);
    } else {
        switch (type) {
            case api_lastfm:
                switch (endpoint_type) {
                    case authorization_endpoint:
                        host = LASTFM_AUTH_URL;
                        host_len = 11;
                        break;
                    case scrobble_endpoint:
                        host = LASTFM_API_BASE_URL;
                        host_len = 21;
                        break;
                    case unknown_endpoint:
                    default:
                        host = "";
                }
                break;
            case api_librefm:
                switch (endpoint_type) {
                    case authorization_endpoint:
                        host = LIBREFM_AUTH_URL;
                        host_len = 8;
                        break;
                    case scrobble_endpoint:
                        host = LIBREFM_API_BASE_URL;
                        host_len = 8;
                        break;
                    case unknown_endpoint:
                    default:
                        host = "";
                }
                break;
            case api_listenbrainz:
                switch (endpoint_type) {
                    case authorization_endpoint:
                        host = LISTENBRAINZ_AUTH_URL;
                        host_len = 54;
                        break;
                    case scrobble_endpoint:
                        host = LISTENBRAINZ_API_BASE_URL;
                        host_len = 20;
                        break;
                    case unknown_endpoint:
                    default:
                        host = "";
                }
                break;
            case api_unknown:
            default:
                return 0;
        }
    }

    strncpy(result, host, host_len);
    return host_len;
}

static size_t endpoint_get_path(char *result, const enum api_type type, const enum end_point_type endpoint_type)
{
    if (NULL == result) { return 0; }
    const char *path = NULL;

    size_t path_len = 0;
    switch (type) {
        case api_lastfm:
            switch (endpoint_type) {
                case authorization_endpoint:
                    path = "/" LASTFM_AUTH_PATH;
                    path_len = 31;
                    break;
                case scrobble_endpoint:
                    path = "/" LASTFM_API_VERSION "/";
                    path_len = 5;
                    break;
                case unknown_endpoint:
                default:
                    path = "";
            }
            break;
        case api_librefm:
            switch (endpoint_type) {
                case authorization_endpoint:
                    path = "/" LIBREFM_AUTH_PATH;
                    path_len = 31;
                    break;
                case scrobble_endpoint:
                    path = "/" LIBREFM_API_VERSION "/";
                    path_len = 5;
                    break;
                case unknown_endpoint:
                default:
                    path = "";
            }
            break;
        case api_listenbrainz:
            switch (endpoint_type) {
                case authorization_endpoint:
                    path = "/" LISTENBRAINZ_API_VERSION "/";
                    path_len = 3;
                    break;
                case scrobble_endpoint:
                    path = "/" LISTENBRAINZ_API_VERSION "/";
                    path_len = 3;
                    break;
                case unknown_endpoint:
                default:
                    path = "";
            }
            break;
        case api_unknown:
        default:
            path = NULL;
            break;
    }
    if (NULL == path) {
        return 0;
    }
    strncpy(result, path, path_len);
    return path_len;
}

static struct api_endpoint *endpoint_new(const struct api_credentials *creds, const enum end_point_type api_endpoint)
{
    if (NULL == creds) { return NULL; }

    struct api_endpoint *result = calloc(1, sizeof(struct api_endpoint));

    const enum api_type type = creds->end_point;
    endpoint_get_scheme(result->scheme, creds->url);
    endpoint_get_host(result->host, type, api_endpoint, creds->url);
    endpoint_get_path(result->path, type, api_endpoint);

    return result;
}

static struct api_endpoint *auth_endpoint_new(const struct api_credentials *creds)
{
    return endpoint_new(creds, authorization_endpoint);
}

struct api_endpoint *api_endpoint_new(const struct api_credentials *creds)
{
    return endpoint_new(creds, scrobble_endpoint);
}

#if 0
static const char *get_api_error_message(api_return_code code)
{
    switch (code) {
        case unavaliable:
            return "The service is temporarily unavailable, please try again.";
        case invalid_service:
            return "Invalid service - This service does not exist";
        case invalid_method:
            return "Invalid Method - No method with that name in this package";
        case authentication_failed:
            return "Authentication Failed - You do not have permissions to access the service";
        case invalid_format:
            return "Invalid format - This service doesn't exist in that format";
        case invalid_parameters:
            return "Invalid parameters - Your request is missing a required parameter";
        case invalid_resource:
            return "Invalid resource specified";
        case operation_failed:
            return "Operation failed - Something else went wrong";
        case invalid_session_key:
            return "Invalid session key - Please re-authenticate";
        case invalid_apy_key:
            return "Invalid API key - You must be granted a valid key by last.fm";
        case service_offline:
            return "Service Offline - This service is temporarily offline. Try again later.";
        case invalid_signature:
            return "Invalid method signature supplied";
        case temporary_error:
            return "There was a temporary error processing your request. Please try again";
        case suspended_api_key:
            return "Suspended API key - Access for your account has been suspended, please contact Last.fm";
        case rate_limit_exceeded:
            return "Rate limit exceeded - Your IP has made too many requests in a short period";
    }
    return "Unkown";
}
#endif

static void http_header_free(struct http_header *header)
{
    if (NULL == header) { return; }

    //if (NULL != header->name) { string_free(header->name); }
    //if (NULL != header->value) { string_free(header->value); }

    free(header);
}

static void http_headers_free(struct http_header **headers)
{
    if (NULL == headers) { return; }
    if (NULL == *headers) { return; }
    const size_t headers_count = arrlen(headers);
    if (headers_count > 0) {
        for (int i = (int)headers_count - 1; i >= 0 ; i--) {
            if (NULL != headers[i]) { http_header_free(headers[i]); }
            (void)arrpop(headers);
        }
        assert(arrlen(headers) == 0);
        arrfree(headers);
    }
}

static void http_request_free(struct http_request *req)
{
    if (NULL == req) { return; }
    if (NULL != req->body) { string_free(req->body); }
    if (NULL != req->query) { string_free(req->query); }
    if (NULL != req->url) { string_free(req->url); }

    api_endpoint_free(req->end_point);
    http_headers_free(req->headers);

    free(req);
}

struct http_request *http_request_new(void)
{
    struct http_request *req = malloc(sizeof(struct http_request));
    req->url         = NULL;
    req->body        = NULL;
    req->body_length = 0;
    req->query       = NULL;
    req->end_point   = NULL;
    req->headers     = NULL;

    return req;
}

static void print_http_request(const struct http_request *req)
{
    char *url = http_request_get_url(req);
    _trace("http::req[%p]%s: %s", req, (req->request_type == http_get ? "GET" : "POST"), url);
    const size_t headers_count = arrlen(req->headers);
    if (headers_count > 0) {
        _trace("http::req::headers[%zd]:", headers_count);
        for (size_t i = 0; i < headers_count; i++) {
            _trace("\theader[%zd]: %s:%s", i, req->headers[i]->name, req->headers[i]->value);
        }
    }
    if (req->request_type != http_get) {
        _trace("http::req[%zu]: %s", req->body_length, req->body);
    }
    string_free(url);
}

static void print_http_response(struct http_response *resp)
{
    _trace("http::resp[%p]: %u", resp, resp->code);
    const size_t headers_count = arrlen(resp->headers);
    if (headers_count > 0) {
        _trace("http::resp::headers[%zd]:", headers_count);
        for (size_t i = 0; i < headers_count; i++) {
            _trace("\theader[%zd]: %s:%s", i, resp->headers[i]->name, resp->headers[i]->value);
        }
    }
}

static bool credentials_valid(const struct api_credentials *c)
{
    return listenbrainz_valid_credentials(c) || audioscrobbler_valid_credentials(c);
}

static const char *api_get_application_secret(const enum api_type type)
{
    switch (type) {
#ifdef LIBREFM_API_SECRET
        case api_librefm:
            return LIBREFM_API_SECRET;
            break;
#endif
#ifdef LASTFM_API_SECRET
        case api_lastfm:
            return LASTFM_API_SECRET;
            break;
#endif
#ifdef LISTENBRAINZ_API_SECRET
        case api_listenbrainz:
            return LISTENBRAINZ_API_SECRET;
#endif
            break;
        case api_unknown:
        default:
            return NULL;
    }
}

static const char *api_get_application_key(const enum api_type type)
{
    switch (type) {
#ifdef LIBREFM_API_KEY
        case api_librefm:
            return LIBREFM_API_KEY;
            break;
#endif
#ifdef LASTFM_API_KEY
        case api_lastfm:
            return LASTFM_API_KEY;
            break;
#endif
#ifdef LISTENBRAINZ_API_KEY
        case api_listenbrainz:
            return LISTENBRAINZ_API_KEY;
            break;
#endif
        case api_unknown:
        default:
            return NULL;
    }
}

static char *api_get_auth_url(const struct api_credentials *credentials)
{
    if (NULL == credentials) { return NULL; }

    const enum api_type type = credentials->end_point;
    const char *token = credentials->token;
    if (NULL == token) { return NULL; }

    struct api_endpoint *auth_endpoint = auth_endpoint_new(credentials);
    const char* base_url = NULL;

    switch(type) {
        case api_lastfm:
        case api_librefm:
           base_url = api_get_url(auth_endpoint);
           break;
        case api_listenbrainz:
        case api_unknown:
        default:
           base_url = get_zero_string(0);
           break;
    }
    const char *api_key = api_get_application_key(type);
    const size_t token_len = strlen(token);
    const size_t key_len = strlen(api_key);
    const size_t base_url_len = strlen(base_url);

    const size_t url_len = base_url_len + token_len + key_len;
    char *url = get_zero_string(url_len);

    if (NULL != base_url) {
        snprintf(url, url_len, base_url, api_key, token);
        string_free((char*)base_url);
    }
    api_endpoint_free(auth_endpoint);

    return url;
}

static struct http_request *api_build_request_get_token(const struct api_credentials *auth, CURL *handle)
{
    switch (auth->end_point) {
        case api_listenbrainz:
            break;
        case api_lastfm:
        case api_librefm:
            return audioscrobbler_api_build_request_get_token(auth, handle);
            break;
        case api_unknown:
        default:
            break;
    }
    return NULL;
}

static struct http_request *api_build_request_get_session(const struct api_credentials *auth, CURL *handle)
{
    switch (auth->end_point) {
        case api_listenbrainz:
            break;
        case api_lastfm:
        case api_librefm:
            return audioscrobbler_api_build_request_get_session(auth, handle);
            break;
        case api_unknown:
        default:
            break;
    }
    return NULL;
}

static struct http_request *api_build_request_now_playing(const struct scrobble *tracks[], const unsigned track_count,
    const struct api_credentials *auth, CURL *handle)
{
    switch (auth->end_point) {
        case api_listenbrainz:
            return listenbrainz_api_build_request_now_playing(tracks, track_count, auth);
            break;
        case api_lastfm:
        case api_librefm:
            return audioscrobbler_api_build_request_now_playing(tracks, track_count, auth, handle);
            break;
        case api_unknown:
        default:
            break;
    }
    return NULL;
}

static struct http_request *api_build_request_scrobble(const struct scrobble *tracks[MAX_QUEUE_LENGTH],
    const unsigned track_count, const struct api_credentials *auth, CURL *handle)
{
    switch (auth->end_point) {
        case api_listenbrainz:
            return listenbrainz_api_build_request_scrobble(tracks, track_count, auth);
            break;
        case api_lastfm:
        case api_librefm:
            return audioscrobbler_api_build_request_scrobble(tracks, track_count, auth, handle);
            break;
        case api_unknown:
        default:
            break;
    }
    return NULL;
}

static struct http_header *http_header_new(void)
{
    struct http_header *header = calloc(1, sizeof(struct http_header));
    return header;
}

struct http_header *http_content_type_header_new (void)
{
    struct http_header *header = http_header_new();
    strncpy(header->name, HTTP_HEADER_CONTENT_TYPE, (MAX_HEADER_NAME_LENGTH - 1));
    strncpy(header->value, CONTENT_TYPE_JSON, (MAX_HEADER_VALUE_LENGTH - 1));

    return header;
}

struct http_header *http_authorization_header_new (const char *token)
{
    struct http_header *header = http_header_new();
    strncpy(header->name, API_HEADER_AUTHORIZATION_NAME, (MAX_HEADER_NAME_LENGTH - 1));
    snprintf(header->value, MAX_HEADER_VALUE_LENGTH, API_HEADER_AUTHORIZATION_VALUE_TOKENIZED, token);

    return header;
}

static void http_response_clean(struct http_response *res)
{
    if (NULL == res) { return; }

    if (NULL != res->body) {
        memset(res->body, 0, MAX_BODY_SIZE);
        res->body_length = 0;
    }
    http_headers_free(res->headers);
    res->headers = NULL;
}

static void http_response_free(struct http_response *res)
{
    if (NULL == res) { return; }

    if (NULL != res->body) {
        string_free(res->body);
        res->body = NULL;
        res->body_length = 0;
    }
    http_headers_free(res->headers);
    free(res);
}

static void http_request_print(const struct http_request *req, enum log_levels log)
{
    if (NULL == req) { return; }

    char *url = http_request_get_url(req);
    _log(log, "  request[%s]: %s", (req->request_type == http_get ? "GET" : "POST"), url);
    string_free(url);

    if (req->body_length > 0 && NULL != req->body) {
        _log(log, "    request::body(%p:%zu): %s", req, req->body_length, req->body);
    }
    if (log != log_tracing2) { return; }

    const size_t headers_count = arrlen(req->headers);
    if (headers_count == 0) {
        return;
    }
    for (size_t i = 0; i < headers_count; i++) {
        struct http_header *h = req->headers[i];
        if (NULL == h) { continue; }
        _log(log, "    request::headers[%zd]: %s: %s", i, h->name, h->value);
    }
}

static void http_response_print(const struct http_response *res, enum log_levels log)
{
    if (NULL == res) { return; }

    _log(log, "  response::status: %zd ", res->code);

    if (res->body_length > 0 && NULL != res->body) {
        _log(log, "    response(%p:%lu): %s", res, res->body_length, res->body);
    }
    if (log != log_tracing2) { return; }

    size_t headers_count = arrlen(res->headers);
    if (headers_count == 0) {
        return;
    }
    for (size_t i = 0; i < headers_count; i++) {
        struct http_header *h = res->headers[i];
        if (NULL == h) { continue; }
        _log(log, "    response::headers[%zd]: %s: %s", i, h->name, h->value);
    }
}

static struct http_response *http_response_new(void)
{
    struct http_response *res = malloc(sizeof(struct http_response));

    res->body = get_zero_string(MAX_BODY_SIZE);
    if (NULL == res->body) { return NULL; }
    res->code = -1;
    res->body_length = 0;
    res->headers = NULL;

    return res;
}

static void http_header_load(const char *data, size_t length, struct http_header *h)
{
    if (NULL == data) { return; }
    if (NULL == h) { return; }
    if (length == 0) { return; }
    char *scol_pos = strchr(data, ':');
    if (NULL == scol_pos) { return; }

    size_t name_length = (size_t)(scol_pos - data);

    if (name_length < 2) { return; }

    size_t value_length = length - name_length - 1;
    strncpy(h->name, data, name_length);

    strncpy(h->value, scol_pos + 2, value_length - 2); // skip : and space
}

static bool json_document_is_error(const char *buffer, const size_t length, enum api_type type)
{
    switch (type) {
        case api_lastfm:
        case api_librefm:
            return audioscrobbler_json_document_is_error(buffer, length);
            break;
        case api_listenbrainz:
            return listenbrainz_json_document_is_error(buffer, length);
            break;
        case api_unknown:
        default:
            break;
    }
    return false;
}

static void api_response_get_token_json(const char *buffer, const size_t length, struct api_credentials *credentials)
{
    switch (credentials->end_point) {
        case api_lastfm:
        case api_librefm:
            audioscrobbler_api_response_get_token_json(buffer, length, credentials);
            break;
        case api_listenbrainz:
        case api_unknown:
        default:
            break;
    }
}

static void api_response_get_session_key_json(const char *buffer, const size_t length, struct api_credentials *credentials)
{
    switch (credentials->end_point) {
        case api_lastfm:
        case api_librefm:
            audioscrobbler_api_response_get_session_key_json(buffer, length, credentials);
            break;
        case api_listenbrainz:
        case api_unknown:
        default:
            break;
    }
}

#endif // MPRIS_SCROBBLER_API_H
