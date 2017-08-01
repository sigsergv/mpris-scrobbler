/**
 * @author Marius Orcsik <marius@habarnam.ro>
 */

#include <time.h>
#include "structs.h"
#include "ini.c"
#include "utils.h"
#include "api.h"
#include "smpris.h"
#include "scrobble.h"
#include "sdbus.h"
#include "sevents.h"

log_level _log_level = warning;
struct configuration global_config = { .credentials = {NULL, NULL}, .credentials_length = 0};
struct timeval now_playing_tv;

/**
 * TODO list
 *  1. Build our own last.fm API functionality
 *  2. Add support for libre.fm in the API
 *  3. Add support for credentials on multiple accounts
 *  4. Add support to blacklist players (I don't want to submit videos from vlc, for example)
 */
int main (int argc, char** argv)
{
    char* command = NULL;
    if (argc > 0) { command = argv[1]; }

    if (NULL != command) {
        if (strncmp(command, "-q", 2) == 0) {
            _log_level = error;
        }
        if (strncmp(command, "-v", 2) == 0) {
            _log_level = info;
        }
        if (strncmp(command, "-vv", 3) == 0) {
            _log_level = debug;
        }
        if (strncmp(command, "-vvv", 4) == 0) {
#ifndef DEBUG
            _warn("main::no_debug: tracing and debug output are disabled");
            _log_level = info;
#else
            _log_level = tracing;
#endif
        }
    }
    // TODO(marius): make this asynchronous to be requested when submitting stuff
    load_configuration(&global_config);

    struct state *state = state_new();
    if (NULL == state) { return EXIT_FAILURE; }

    event_base_dispatch(state->events->base);
    state_free(state);
    free_configuration(&global_config);

    return EXIT_SUCCESS;
}
