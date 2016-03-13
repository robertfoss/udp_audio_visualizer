#include <stdio.h>
#include <pulse/pulseaudio.h>

#include "pulseaudio.h"


static void context_state_cb(pa_context* context, void* mainloop);
static void stream_state_cb(pa_stream *s, void *mainloop);
static void stream_success_cb(pa_stream *stream, int success, void *data);
static void stream_read_cb(pa_stream *stream, size_t nbr_bytes, void *data);

char *default_sink_monitor;
pa_threaded_mainloop *mainloop;
pa_mainloop_api *mainloop_api;
pa_context *context;
pa_stream *stream;

void server_info_cb(pa_context* context, const pa_server_info *i, void *userdata) {
    UNUSED(context);
    log_info("server_info_cb called");

    pa_server_info *info = (pa_server_info *) userdata;
    info->default_sink_name = i->default_sink_name;
    info->default_source_name = i->default_source_name;

    pa_threaded_mainloop_signal(mainloop, 0);
}

char * get_device_to_record(pa_context *context, pa_threaded_mainloop *mainloop) {
    log_info("get_device_to_record called");
    pa_server_info info;
    pa_operation *op = pa_context_get_server_info(context, server_info_cb, &info);
    while (pa_operation_get_state(op) != PA_OPERATION_DONE) {
        log_infof("get_device_to_record waiting, status: %d", pa_operation_get_state(op));
        pa_threaded_mainloop_wait(mainloop);
    }

    const char *monitor = ".monitor";
    size_t dev_monitor_strlen = strlen(info.default_sink_name) + strlen(monitor) + 1;
    default_sink_monitor = realloc(default_sink_monitor, dev_monitor_strlen);
    snprintf(default_sink_monitor, dev_monitor_strlen, "%s%s", info.default_sink_name, monitor);
    log_infof("default_sink_monitor: %s", default_sink_monitor);
    return default_sink_monitor;
}


ret_code pulseaudio_init() {
    // Get a mainloop and its context
    mainloop = pa_threaded_mainloop_new();
    ASSERT(mainloop);

    // Lock the mainloop so that it does not run and crash before the context is ready
    pa_threaded_mainloop_lock(mainloop);

    mainloop_api = pa_threaded_mainloop_get_api(mainloop);
    context = pa_context_new(mainloop_api, "udp_audio_visualizer");
    ASSERT(context);

    // Set a callback so we can wait for the context to be ready
    pa_context_set_state_callback(context, &context_state_cb, mainloop);

    // Start the mainloop
    ASSERT(pa_threaded_mainloop_start(mainloop) == 0);
    ASSERT(pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) == 0);

    // Wait for the context to be ready
    for (;;) {
        pa_context_state_t context_state = pa_context_get_state(context);
        ASSERT(PA_CONTEXT_IS_GOOD(context_state));
        if (context_state == PA_CONTEXT_READY) break;
        pa_threaded_mainloop_wait(mainloop);
    }

    // Create a playback stream
    pa_sample_spec sample_specifications;
    sample_specifications.format = PULSEAUDIO_FORMAT;
    sample_specifications.rate = AUDIO_PROCESS_SAMPLE_RATE;
    sample_specifications.channels = 2;

    // Recommended settings, i.e. server uses sensible values
    pa_buffer_attr buffer_attr;
    buffer_attr.maxlength = (uint32_t) PULSEAUDIO_BUFSIZE;
    buffer_attr.fragsize  = (uint32_t) - 1; // Let PA determine frag size

    pa_channel_map map;
    pa_channel_map_init_stereo(&map);

    stream = pa_stream_new(context, "evesdrop", &sample_specifications, &map);
    pa_stream_set_state_callback(stream, stream_state_cb, mainloop);
    pa_stream_set_read_callback(stream, stream_read_cb, mainloop);

    // Settings copied as per the chromium browser source
    pa_stream_flags_t stream_flags;
    stream_flags = PA_STREAM_START_CORKED | PA_STREAM_INTERPOLATE_TIMING |
                   PA_STREAM_NOT_MONOTONIC | PA_STREAM_AUTO_TIMING_UPDATE |
                   PA_STREAM_ADJUST_LATENCY;

    // Connect stream to the default audio output sink
    char *device = get_device_to_record(context, mainloop);
    ASSERT(pa_stream_connect_record(stream, device, &buffer_attr, stream_flags) == 0);

    // Wait for the stream to be ready
    for (;;) {
        pa_stream_state_t stream_state = pa_stream_get_state(stream);
        ASSERT(PA_STREAM_IS_GOOD(stream_state));
        if (stream_state == PA_STREAM_READY) break;
        pa_threaded_mainloop_wait(mainloop);
    }

    pa_threaded_mainloop_unlock(mainloop);

    // Uncork the stream so it will start playing
    pa_stream_cork(stream, 0, stream_success_cb, mainloop);

    return RET_OK;
}


static void context_state_cb(pa_context* context, void* mainloop) {
    UNUSED(context);

    pa_threaded_mainloop_signal(mainloop, 0);
}


static void stream_state_cb(pa_stream *stream, void *mainloop) {
    UNUSED(stream);

    pa_threaded_mainloop_signal(mainloop, 0);
}


static void stream_read_cb(pa_stream *stream, size_t nbr_bytes, void *data) {
    UNUSED(data);
    PULSEAUDIO_SAMPLE_TYPE *sample_buf;
    if (pa_stream_peek(stream, (const void **) &sample_buf, &nbr_bytes) < 0) {

        DIE("pa_stream_peek() failed");
    }

    size_t channel_buf_size = DIV_ROUND(nbr_bytes, PULSEAUDIO_NBR_CHANNELS);
    PULSEAUDIO_SAMPLE_TYPE *data_left = malloc(channel_buf_size);
    PULSEAUDIO_SAMPLE_TYPE *ptr_data_left = data_left;

    PULSEAUDIO_SAMPLE_TYPE *data_right = malloc(channel_buf_size);
    PULSEAUDIO_SAMPLE_TYPE *ptr_data_right = data_right;

    size_t nbr_samples = DIV_ROUND(nbr_bytes, PULSEAUDIO_SAMPLE_BYTES);
    size_t nbr_chan_samples = DIV_ROUND(nbr_samples, PULSEAUDIO_NBR_CHANNELS);

    for (size_t i = 0; i < nbr_chan_samples; i++) {
        *ptr_data_left++ = *sample_buf++;
        *ptr_data_right++ = *sample_buf++;
    }

    log_infof("copied  %u bytes / %u channel_buf_size / %u channel samples / %f ms", (uint32_t) nbr_bytes, (uint32_t) channel_buf_size, (uint32_t) nbr_chan_samples, (double)(nbr_chan_samples * 1000) / AUDIO_PROCESS_SAMPLE_RATE);
    log_infof("ap_sample_rate: %u  ap_target_latency: %u  ap_ffts_per_sec: %u  pa_chunk_bytes: %u  pa_bufsize: %u",
              AUDIO_PROCESS_SAMPLE_RATE,
              AUDIO_PROCESS_TARGET_LATENCY,
              AUDIO_PROCESS_FFTS_PER_SEC,
              PULSEAUDIO_CHUNK_BYTES,
              PULSEAUDIO_BUFSIZE);
    log_infof("left:  %5d %5d %5d %5d %5d", data_left[0], data_left[1], data_left[2], data_left[3], data_left[4]);
    log_infof("right: %5d %5d %5d %5d %5d", data_right[0], data_right[1], data_right[2], data_right[3], data_right[4]);

    ASSERT((PULSEAUDIO_NBR_CHANNELS * nbr_chan_samples) == nbr_samples);
    ASSERT(stream);
    ASSERT(nbr_bytes > 0);

    audio_process_add_samples((PULSEAUDIO_SAMPLE_TYPE *) data_left, (PULSEAUDIO_SAMPLE_TYPE *) data_right, nbr_chan_samples);

    // Pop that data that we peeked
    pa_stream_drop(stream);
}


static void stream_success_cb(pa_stream *stream, int success, void *data) {
    UNUSED(stream);
    UNUSED(success);
    UNUSED(data);

    return;
}
