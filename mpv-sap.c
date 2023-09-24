#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define MPV_ENABLE_DEPRECATED 0
#include "mpv/client.h"
#include ASAP_H

static ASAPSampleFormat arg_sample_format = ASAPSampleFormat_S16_L_E;
static int arg_duration = -1;
static int arg_mute_mask = 0;
static const char *arg_author = NULL;
static const char *arg_name = NULL;
static const char *arg_date = NULL;

static int current_song;
static const char *output_file = "pipe";

static mpv_handle *mpv = NULL;

static void fatal_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "asapconv: ");
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
    va_end(args);
    exit(1);
}

static void apply_tags(ASAPInfo *info)
{
    if (arg_author != NULL) {
        if (!ASAPInfo_SetAuthor(info, arg_author))
            fatal_error("invalid author");
    }
    if (arg_name != NULL) {
        if (!ASAPInfo_SetTitle(info, arg_name))
            fatal_error("invalid music name");
        arg_name = NULL;
    }
    if (arg_date != NULL) {
        if (!ASAPInfo_SetDate(info, arg_date))
            fatal_error("invalid date");
    }
}

static ASAP *load_module(const char *input_file, const unsigned char *module, int module_len)
{
    ASAP *asap = ASAP_New();
    if (asap == NULL)
        fatal_error("out of memory");
    if (!ASAP_Load(asap, input_file, module, module_len))
        fatal_error("%s: unsupported file", input_file);
    ASAPInfo *info = (ASAPInfo *) ASAP_GetInfo(asap); /* FIXME: avoid cast */
    apply_tags(info);
    return asap;
}

static int play_song(const char *input_file, ASAP *asap)
{
    int duration = arg_duration;
    if (duration < 0) {
        duration = ASAPInfo_GetDuration(ASAP_GetInfo(asap), current_song);
        if (duration < 0)
            duration = 180 * 1000;
    }
    if (!ASAP_PlaySong(asap, current_song, duration))
        fatal_error("%s: PlaySong failed", input_file);
    ASAP_MutePokeyChannels(asap, arg_mute_mask);
    return duration;
}

static int write_output_file(int fd, unsigned char *buf, size_t len)
{
    size_t sent = 0;
    while (sent < len)
    {
        ssize_t n = write(fd, buf + sent, len - sent);
        if (n == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                mpv_event *event = mpv_wait_event(mpv, 0);
                if (event->event_id != MPV_EVENT_END_FILE)
                    continue;
            }
            printf("error writing to %s\n", output_file);
            return 1;
        }
        sent += n;
    }
    return 0;
}

static void close_file(int fd)
{
    if (close(fd) != 0)
        fatal_error("error closing %s", output_file);
}

static void convert_to_wav(const char *input_file, const unsigned char *module, int module_len, int ofd)
{
    ASAP *asap = load_module(input_file, module, module_len);

    play_song(input_file, asap);
    unsigned char buffer[8192];
    int n_bytes;

    n_bytes = ASAP_GetWavHeader(asap, buffer, arg_sample_format, true);
    if (write_output_file(ofd, buffer, n_bytes) != 0)
        goto abort;

    do {
        n_bytes = ASAP_Generate(asap, buffer, sizeof(buffer), arg_sample_format);
        if (write_output_file(ofd, buffer, n_bytes) != 0)
            goto abort;
    } while (n_bytes == sizeof(buffer));

abort:
    close_file(ofd);
    ASAP_Delete(asap);
}

static void process_file(const char *input_file, int ofd)
{
    int ifd = open(input_file, O_RDONLY);
    if (ifd == -1)
        fatal_error("cannot open %s", input_file);
    static unsigned char module[ASAPInfo_MAX_MODULE_LENGTH];
    int module_len = read(ifd, module, sizeof(module));
    close_file(ifd);

    convert_to_wav(input_file, module, module_len, ofd);
}

static void get_and_convert_file(mpv_event_hook *hook)
{
    char *input_filename = mpv_get_property_string(mpv, "stream-open-filename");
    const char *extension = strrchr(input_filename, '.');
    if (!extension || strcasecmp(extension, ".sap") != 0)
    {
        /* not an SAP file */
        mpv_free(input_filename);
        mpv_hook_continue(mpv, hook->id);
    }
    else
    {
        printf("mpv-sap: Converting SAP file %s\n", input_filename);

        int fds[2] = { 0, 0 };
        if (pipe(fds) != 0)
            fatal_error("Failed to open pipe.");
        fcntl(fds[1], F_SETFL, fcntl(fds[1], F_GETFL) | O_NONBLOCK);

        char output_filename[256];
        sprintf(output_filename, "/proc/self/fd/%d", fds[0]);
        mpv_set_property_string(mpv, "stream-open-filename", output_filename);
        mpv_hook_continue(mpv, hook->id);

        process_file(input_filename, fds[1]);
        mpv_free(input_filename);
    }
}

int mpv_open_cplugin(mpv_handle *_mpv)
{
    mpv = _mpv;
    mpv_hook_add(mpv, 0, "on_load", 0);
    while (1) {
        mpv_event *event = mpv_wait_event(mpv, -1);
        if (event->event_id == MPV_EVENT_SHUTDOWN)
            break;
        if (event->event_id == MPV_EVENT_HOOK)
            get_and_convert_file(event->data);
    }
    return 0;
}
