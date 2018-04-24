/**
 * @author Marius Orcsik <marius@habarnam.ro>
 */
#ifndef MPRIS_SCROBBLER_API_H
#define MPRIS_SCROBBLER_API_H

#include <inttypes.h>
#include <json-c/json.h>
#include <stdbool.h>
#include "md5.h"

#define MAX_HEADER_LENGTH               256
#define MAX_HEADER_NAME_LENGTH          (MAX_URL_LENGTH / 2)
#define MAX_HEADER_VALUE_LENGTH         (MAX_URL_LENGTH / 2)
#define MAX_URL_LENGTH                  2048
#define MAX_BODY_SIZE                   16384

#define CONTENT_TYPE_XML            "application/xml"
#define CONTENT_TYPE_JSON           "application/json"

#define API_HEADER_AUTHORIZATION_NAME "Authorization"
#define API_HEADER_AUTHORIZATION_VALUE_TOKENIZED "Token %s"

enum api_return_status {
    status_failed  = 0, // failed == bool false
    status_ok           // ok == bool true
};

struct api_response {
    enum api_return_status status;
    struct api_error *error;
};

typedef enum message_types {
    api_call_authenticate,
    api_call_now_playing,
    api_call_scrobble,
} message_type;

struct http_header {
    char *name;
    char *value;
};

struct http_response {
    unsigned short code;
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

struct api_endpoint {
    char *scheme;
    char *host;
    char *path;
};

struct http_request {
    http_request_type request_type;
    struct api_endpoint *end_point;
    char *url;
    char *query;
    char *body;
    size_t body_length;
    struct http_header **headers;
};

#include "audioscrobbler_api.h"
#include "listenbrainz_api.h"

#define HTTP_HEADER_CONTENT_TYPE "Content-Type"

char *http_response_headers_content_type(struct http_response *res)
{

    int headers_count = sb_count(res->headers);
    for (int i = 0; i < headers_count; i++) {
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

char *api_get_url(struct api_endpoint *endpoint)
{
    if (NULL == endpoint) { return NULL; }
    char *url = get_zero_string(MAX_URL_LENGTH);

    strncat(url, endpoint->scheme, strlen(endpoint->scheme));
    strncat(url, "://", 3);
    strncat(url, endpoint->host, strlen(endpoint->host));
    strncat(url, endpoint->path, strlen(endpoint->path));

    return url;
}

char *http_request_get_url(const struct http_request *request)
{
    if (NULL == request) { return NULL; }
    char *url = get_zero_string(MAX_URL_LENGTH);

    strncat(url, request->url, strlen(request->url));
    if (NULL != request->query) {
        strncat(url, "?", 1);
        strncat(url, request->query, strlen(request->query));
    }

    return url;
}

void api_endpoint_free(struct api_endpoint *api)
{
    if (NULL != api->host) { free(api->host); }
    if (NULL != api->path) { free(api->path); }
    if (NULL != api->scheme) { free(api->scheme); }
    free(api);
}

char *endpoint_get_scheme(const char *custom_url)
{
    const char *scheme = NULL;
    if (NULL != custom_url && strlen(custom_url) != 0) {
        if (strncmp(custom_url, "https://", 8) == 0) {
            scheme = "https";
        } else if (strncmp(custom_url, "http://", 7) == 0) {
            scheme = "http";
        }
    } else {
        scheme = "https";
    }

    char *result = get_zero_string(strlen(scheme));
    strncpy(result, scheme, strlen(scheme));

    return result;
}

char *endpoint_get_host(const enum api_type type, const enum end_point_type endpoint_type, const char *custom_url)
{
    const char* host = NULL;
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
    } else {
        switch (type) {
            case lastfm:
                switch (endpoint_type) {
                    case auth_endpoint:
                        host = LASTFM_AUTH_URL;
                        break;
                    case scrobble_endpoint:
                        host = LASTFM_API_BASE_URL;
                        break;
                    case unknown_endpoint:
                    default:
                        host = "";
                }
                break;
            case librefm:
                switch (endpoint_type) {
                    case auth_endpoint:
                        host = LIBREFM_AUTH_URL;
                        break;
                    case scrobble_endpoint:
                        host = LIBREFM_API_BASE_URL;
                        break;
                    case unknown_endpoint:
                    default:
                        host = "";
                }
                break;
            case listenbrainz:
                switch (endpoint_type) {
                    case auth_endpoint:
                        host = LISTENBRAINZ_AUTH_URL;
                        break;
                    case scrobble_endpoint:
                        host = LISTENBRAINZ_API_BASE_URL;
                        break;
                    case unknown_endpoint:
                    default:
                        host = "";
                }
                break;
            case unknown:
            default:
                host = NULL;
                break;
        }
    }
    char *result = get_zero_string(strlen(host));
    strncpy(result, host, strlen(host));

    return result;
}

char *endpoint_get_path(const enum api_type type, const enum end_point_type endpoint_type)
{
    const char* path = NULL;
    switch (type) {
        case lastfm:
            switch (endpoint_type) {
                case auth_endpoint:
                    path = "/" LASTFM_AUTH_PATH;
                    break;
                case scrobble_endpoint:
                    path = "/" LASTFM_API_VERSION "/";
                    break;
                case unknown_endpoint:
                default:
                    path = "";
            }
            break;
        case librefm:
            switch (endpoint_type) {
                case auth_endpoint:
                    path = "/" LIBREFM_AUTH_PATH;
                    break;
                case scrobble_endpoint:
                    path = "/" LIBREFM_API_VERSION "/";
                    break;
                case unknown_endpoint:
                default:
                    path = "";
            }
            break;
        case listenbrainz:
            switch (endpoint_type) {
                case auth_endpoint:
                    path = "/" LISTENBRAINZ_API_VERSION "/";
                    break;
                case scrobble_endpoint:
                    path = "/" LISTENBRAINZ_API_VERSION "/";
                    break;
                case unknown_endpoint:
                default:
                    path = "";
            }
            break;
        case unknown:
        default:
            path = NULL;
            break;
    }
    char *result = get_zero_string(strlen(path));
    strncpy(result, path, strlen(path));

    return result;
}

struct api_endpoint *endpoint_new(const struct api_credentials *creds, const enum end_point_type api_endpoint)
{
    if (NULL == creds) { return NULL; }

    struct api_endpoint *result = malloc(sizeof(struct api_endpoint));

    enum api_type type = creds->end_point;
    result->scheme = endpoint_get_scheme(creds->url);
    result->host = endpoint_get_host(type, api_endpoint, creds->url);
    result->path = endpoint_get_path(type, api_endpoint);

    return result;
}

struct api_endpoint *auth_endpoint_new(const struct api_credentials *creds)
{
    return endpoint_new(creds, auth_endpoint);
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

    if (NULL != header->name) { free(header->name); }
    if (NULL != header->value) { free(header->value); }

    free(header);
}

void http_headers_free(struct http_header **headers)
{
    if (NULL == headers) { return; }

    int headers_count = sb_count(headers);
    if (headers_count > 0) {
        for (int i = 0; i < headers_count; i++) {
            if (NULL != headers[i]) { http_header_free(headers[i]); }
            (void)sb_add(headers, (-1));
        }
        assert(sb_count(headers) == 0);
        sb_free(headers);
    }
}

void http_request_free(struct http_request *req)
{
    if (NULL == req) { return; }
    if (NULL != req->body) { free(req->body); }
    if (NULL != req->query) { free(req->query); }
    if (NULL != req->url) { free(req->url); }

    api_endpoint_free(req->end_point);
    http_headers_free(req->headers);

    free(req);
}

struct http_request *http_request_new(void)
{
    struct http_request *req = malloc(sizeof(struct http_request));
    req->url = NULL;
    req->body = NULL;
    req->body_length = 0;
    req->query = NULL;
    req->end_point = NULL;
    req->headers = NULL;

    return req;
}

void print_http_request(struct http_request *req)
{
    char *url = http_request_get_url(req);
    _trace("http::req[%p]%s: %s", req, (req->request_type == http_get ? "GET" : "POST"), url);
    int headers_count = sb_count(req->headers);
    if (headers_count > 0) {
        _trace("http::req::headers[%zd]:", headers_count);
        for (int i = 0; i < headers_count; i++) {
            _trace("\theader[%zd]: %s:%s", i, req->headers[i]->name, req->headers[i]->value);
        }
    }
    if (req->request_type != http_get) {
        _trace("http::req[%zu]: %s", req->body_length, req->body);
    }
    free(url);
}

void print_http_response(struct http_response *resp)
{
    _trace("http::resp[%p]: %u", resp, resp->code);
    int headers_count = sb_count(resp->headers);
    if (headers_count > 0) {
        _trace("http::resp::headers[%zd]:", headers_count);
        for (int i = 0; i < headers_count; i++) {
            _trace("\theader[%zd]: %s:%s", i, resp->headers[i]->name, resp->headers[i]->value);
        }
    }
}

#define MD5_DIGEST_LENGTH 16
char *api_get_signature(const char *string, const char *secret)
{
    if (NULL == string) { return NULL; }
    if (NULL == secret) { return NULL; }
    size_t string_len = strlen(string);
    size_t secret_len = strlen(secret);
    size_t len = string_len + secret_len;
    char *sig = get_zero_string(len);

    // TODO(marius): this needs to change to memcpy or strcpy
    strncat(sig, string, string_len);
    strncat(sig, secret, secret_len);

    unsigned char sig_hash[MD5_DIGEST_LENGTH];

    md5((uint8_t*)sig, len, sig_hash);

    char *result = get_zero_string(MD5_DIGEST_LENGTH * 2 + 2);
    for (size_t n = 0; n < MD5_DIGEST_LENGTH; n++) {
        snprintf(result + 2 * n, 3, "%02x", sig_hash[n]);
    }

    free(sig);
    return result;
}

bool credentials_valid(struct api_credentials *c)
{
    switch (c->end_point) {
        case librefm:
#ifndef LIBREFM_API_SECRET
            return false;
#endif
            break;
        case lastfm:
#ifndef LASTFM_API_SECRET
            return false;
#endif
            break;
        case listenbrainz:
#ifndef LISTENBRAINZ_API_SECRET
            return false;
#endif
            break;
        case unknown:
        default:
            return false;
            break;
    }
    return (c->enabled && c->authenticated);
}

const char *api_get_application_secret(enum api_type type)
{
    switch (type) {
#ifdef LIBREFM_API_SECRET
        case librefm:
            return LIBREFM_API_SECRET;
            break;
#endif
#ifdef LASTFM_API_SECRET
        case lastfm:
            return LASTFM_API_SECRET;
            break;
#endif
#ifdef LISTENBRAINZ_API_SECRET
        case listenbrainz:
            return LISTENBRAINZ_API_SECRET;
#endif
            break;
        default:
            return NULL;
    }
    return NULL;
}

const char *api_get_application_key(enum api_type type)
{
    switch (type) {
#ifdef LIBREFM_API_KEY
        case librefm:
            return LIBREFM_API_KEY;
            break;
#endif
#ifdef LASTFM_API_KEY
        case lastfm:
            return LASTFM_API_KEY;
            break;
#endif
#ifdef LISTENBRAINZ_API_KEY
        case listenbrainz:
            return LISTENBRAINZ_API_KEY;
            break;
#endif
        default:
            return NULL;
    }
    return NULL;
}

char *api_get_auth_url(struct api_credentials *credentials)
{
    if (NULL == credentials) { return NULL; }

    enum api_type type = credentials->end_point;
    const char *token = credentials->token;
    if (NULL == token) { return NULL; }

    struct api_endpoint *auth_endpoint = auth_endpoint_new(credentials);
    const char* base_url = NULL;

    switch(type) {
        case lastfm:
        case librefm:
           base_url = api_get_url(auth_endpoint);
           break;
        case listenbrainz:
        case unknown:
        default:
           base_url = get_zero_string(0);
           break;
    }
    const char *api_key = api_get_application_key(type);
    size_t token_len = strlen(token);
    size_t key_len = strlen(api_key);
    size_t base_url_len = strlen(base_url);

    size_t url_len = base_url_len + token_len + key_len;
    char *url = get_zero_string(url_len);

    snprintf(url, url_len, base_url, api_key, token);
    free((char*)base_url);
    api_endpoint_free(auth_endpoint);

    return url;
}

struct http_request *api_build_request_get_token(CURL *handle, const struct api_credentials *auth)
{
    switch (auth->end_point) {
        case listenbrainz:
            break;
        case lastfm:
        case librefm:
            return audioscrobbler_api_build_request_get_token(handle, auth);
            break;
        default:
            break;
    }
    return NULL;
}

struct http_request *api_build_request_get_session(CURL *handle, const struct api_credentials *auth)
{
    switch (auth->end_point) {
        case listenbrainz:
            break;
        case lastfm:
        case librefm:
            return audioscrobbler_api_build_request_get_session(handle, auth);
            break;
        default:
            break;
    }
    return NULL;
}

struct http_request *api_build_request_now_playing(const struct scrobble *track, CURL *handle, const struct api_credentials *auth)
{
    switch (auth->end_point) {
        case listenbrainz:
            return listenbrainz_api_build_request_now_playing(track, handle, auth);
            break;
        case lastfm:
        case librefm:
            return audioscrobbler_api_build_request_now_playing(track, handle, auth);
            break;
        default:
            break;
    }
    return NULL;
}

struct http_request *api_build_request_scrobble(const struct scrobble *tracks[], size_t track_count, CURL *handle, const struct api_credentials *auth)
{
    switch (auth->end_point) {
        case listenbrainz:
            return listenbrainz_api_build_request_scrobble(tracks, track_count, handle, auth);
            break;
        case lastfm:
        case librefm:
            return audioscrobbler_api_build_request_scrobble(tracks, track_count, handle, auth);
            break;
        default:
            break;
    }
    return NULL;
}

static struct http_header *http_header_new(void)
{
    struct http_header *header = malloc(sizeof(struct http_header));
    header->name = get_zero_string(MAX_HEADER_NAME_LENGTH);
    header->value = get_zero_string(MAX_HEADER_VALUE_LENGTH);

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
    strncpy(header->name, API_HEADER_AUTHORIZATION_NAME, (MAX_HEADER_VALUE_LENGTH - 1));
    snprintf(header->value, (MAX_HEADER_VALUE_LENGTH - 1), API_HEADER_AUTHORIZATION_VALUE_TOKENIZED, token);

    return header;
}

void http_response_free(struct http_response *res)
{
    if (NULL == res) { return; }

    if (NULL != res->body) {
        free(res->body);
        res->body = NULL;
    }
    http_headers_free(res->headers);

    free(res);
}

#if 0
void http_response_print(struct http_response *res)
{
}
#endif

struct http_response *http_response_new(void)
{
    struct http_response *res = malloc(sizeof(struct http_response));

    res->body = get_zero_string(MAX_BODY_SIZE);
    res->code = -1;
    res->body_length = 0;
    res->headers = NULL;

    return res;
}

void http_header_load(const char *data, size_t length, struct http_header *h)
{
    if (NULL == data) { return; }
    if (NULL == h) { return; }
    if (length == 0) { return; }
    char *scol_pos = strchr(data, ':');
    if (NULL == scol_pos) { return; }

    size_t name_length = (size_t)(scol_pos - data);
    size_t value_length = length - name_length - 1;
    strncpy(h->name, data, name_length);
#if 0
    _trace3("mem::loading_header_name[%lu:%lu] %s", length, name_length, h->name);
#endif

    strncpy(h->value, scol_pos + 2, value_length); // skip : and space
#if 0
    _trace3("mem::loading_header_value[%lu] %s", value_length, h->value);
#endif
}

static size_t http_response_write_headers(char *buffer, size_t size, size_t nitems, void* data)
{
    if (NULL == buffer) { return 0; }
    if (NULL == data) { return 0; }
    if (0 == size) { return 0; }
    if (0 == nitems) { return 0; }

    struct http_response *res = data;

    size_t new_size = size * nitems;

    //string_trim(&buffer, nitems, "\n\r ");
    //_trace("curl::header[%lu]: %s", new_size, buffer);

    struct http_header *h = http_header_new();

    http_header_load(buffer, nitems, h);
    if (NULL == h->name) { goto _err_exit; }

    sb_push(res->headers, h);
    return new_size;

_err_exit:
    free(h);
    return new_size;
}

size_t http_response_write_body(void *buffer, size_t size, size_t nmemb, void* data)
{
    if (NULL == buffer) { return 0; }
    if (NULL == data) { return 0; }
    if (0 == size) { return 0; }
    if (0 == nmemb) { return 0; }

    struct http_response *res = (struct http_response*)data;

    size_t new_size = size * nmemb;

    strncat(res->body, buffer, new_size);
    res->body_length += new_size;

    assert (res->body_length < MAX_BODY_SIZE);
    memset(res->body + res->body_length, 0x0, MAX_BODY_SIZE - res->body_length);

    return new_size;
}

enum api_return_status curl_request(CURL *handle, const struct http_request *req, const http_request_type t, struct http_response *res)
{
    enum api_return_status ok = status_failed;
    if (t == http_post) {
        curl_easy_setopt(handle, CURLOPT_POSTFIELDS, req->body);
        curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, (long)req->body_length);
    }

    char *url = http_request_get_url(req);
    if (req->body_length == 0 || NULL == req->body) {
        _trace("curl::request[%s]: %s", (t == http_get ? "GET" : "POST"), url);
    } else {
        _trace("curl::request[%s]: %s\ncurl::body[%zu]: %s", (t == http_get ? "GET" : "POST"), url, req->body_length, req->body);
    }

    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_HEADER, 0L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, http_response_write_body);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, res);
    curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, http_response_write_headers);
    curl_easy_setopt(handle, CURLOPT_HEADERDATA, res);
    struct curl_slist *headers = NULL;
    bool free_headers = false;
    int headers_count = sb_count(req->headers);
    if (headers_count > 0) {
        for (int i = 0; i < headers_count; i++) {
            struct http_header *header = req->headers[i];
            char *full_header = get_zero_string(MAX_URL_LENGTH);
            snprintf(full_header, MAX_URL_LENGTH, "%s: %s", header->name, header->value);

            _trace("curl::header[%zd]: %s", i, full_header);
            headers = curl_slist_append(headers, full_header);

            free(full_header);
        }
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
        free_headers = true;
    }

    CURLcode cres = curl_easy_perform(handle);
    /* Check for errors */
    if(cres != CURLE_OK) {
        _error("curl::error: %s", curl_easy_strerror(cres));
        goto _exit;
    }
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &res->code);
    _trace("curl::response(%p:%lu): %s", res, res->body_length, res->body);

    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &res->code);
#if 0
    if (res->code < 500) { http_response_print(res); }
#endif
    if (res->code == 200) { ok = status_ok; }
_exit:
    free(url);
    if (free_headers) {
        curl_slist_free_all(headers);
    }
    return ok;
}

enum api_return_status api_get_request(CURL *handle, const struct http_request *req, struct http_response *res)
{
    return curl_request(handle, req, http_get, res);
}

enum api_return_status api_post_request(CURL *handle, const struct http_request *req, struct http_response *res)
{
    return curl_request(handle, req, http_post, res);
}

bool json_document_is_error(const char *buffer, const size_t length, enum api_type type)
{
    switch (type) {
        case lastfm:
        case librefm:
            return audioscrobbler_json_document_is_error(buffer, length);
            break;
        case listenbrainz:
            return listenbrainz_json_document_is_error(buffer, length);
            break;
        case unknown:
        default:
            break;
    }
    return false;
}

void api_response_get_token_json(const char *buffer, const size_t length, char **token, enum api_type type)
{
    switch (type) {
        case lastfm:
        case librefm:
            audioscrobbler_api_response_get_token_json(buffer, length, token);
            break;
        case listenbrainz:
        case unknown:
        default:
            break;
    }
}

void api_response_get_session_key_json(const char *buffer, const size_t length, char **session_key, char **name, enum api_type type)
{
    switch (type) {
        case lastfm:
        case librefm:
            audioscrobbler_api_response_get_session_key_json(buffer, length, session_key, name);
            break;
        case listenbrainz:
        case unknown:
        default:
            break;
    }
}

#endif // MPRIS_SCROBBLER_API_H
