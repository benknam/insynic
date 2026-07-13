#include "insynic_scrcpy.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>

#include "audio_player.h"
#include "controller.h"
#include "decoder.h"
#include "delay_buffer.h"
#include "demuxer.h"
#include "events.h"
#include "file_pusher.h"
#include "keyboard_sdk.h"
#include "mouse_sdk.h"
#include "screen.h"
#include "sdl_hints.h"
#include "server.h"
#include "uhid/gamepad_uhid.h"
#include "uhid/keyboard_uhid.h"
#include "uhid/mouse_uhid.h"
#include "usb/aoa_hid.h"
#include "usb/gamepad_aoa.h"
#include "usb/keyboard_aoa.h"
#include "usb/mouse_aoa.h"
#include "usb/scrcpy_otg.h"
#include "util/acksync.h"
#include "util/log.h"
#include "util/rand.h"
#include "util/thread.h"
#include "util/tick.h"

struct insynic_scrcpy {
    struct scrcpy_options options;
    struct sc_server server;
    struct sc_screen screen;
    struct sc_audio_player audio_player;
    struct sc_demuxer video_demuxer;
    struct sc_demuxer audio_demuxer;
    struct sc_decoder video_decoder;
    struct sc_decoder audio_decoder;
    struct sc_controller controller;
    struct sc_file_pusher file_pusher;
    struct sc_uhid_devices uhid_devices;

    union {
        struct sc_keyboard_sdk keyboard_sdk;
        struct sc_keyboard_uhid keyboard_uhid;
    };
    union {
        struct sc_mouse_sdk mouse_sdk;
        struct sc_mouse_uhid mouse_uhid;
    };

    bool server_started;
    bool controller_started;
    bool screen_initialized;
    bool file_pusher_initialized;
    bool video_demuxer_started;
    bool audio_demuxer_started;

    enum insynic_scrcpy_state state;
    insynic_scrcpy_state_cb state_cb;
    void *state_cb_userdata;

    insynic_scrcpy_event_cb event_cb;
    void *event_cb_userdata;

    sc_thread thread;
    bool running;

    bool server_connected;
    bool screen_on;
    bool server_connection_failed;
    sc_mutex server_mutex;
    sc_cond server_cond;

    bool connection_ready;
    bool window_ready;

    void *nsview_ptr;
};

static bool
insynic_scrcpy_event_watch(void *userdata, SDL_Event *event);

static void
set_state(struct insynic_scrcpy *s, enum insynic_scrcpy_state state) {
    s->state = state;
    if (s->state_cb) {
        s->state_cb(state, s->state_cb_userdata);
    }
}

static uint32_t
generate_scid(void) {
    static struct sc_rand rand;
    sc_rand_init(&rand);
    return sc_rand_u32(&rand) & 0x7FFFFFFF;
}

bool
insynic_scrcpy_init_sdl(void) {
#ifdef __APPLE__
    SDL_SetHint(SDL_HINT_MAC_BACKGROUND_APP, "0");
    SDL_SetHint(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, "1");
#endif

    if (!SDL_Init(SDL_INIT_EVENTS)) {
        LOGE("Could not initialize SDL events: %s", SDL_GetError());
        return false;
    }

    sc_set_log_level(SC_LOG_LEVEL_INFO);
    sc_log_configure();

    return true;
}

static void
sc_server_on_connection_failed(struct sc_server *server, void *userdata) {
    (void)server;
    (void)userdata;
    LOGE("sc_server_on_connection_failed called!");
    sc_push_event(SC_EVENT_SERVER_CONNECTION_FAILED);
}

static void
sc_server_on_connected(struct sc_server *server, void *userdata) {
    (void)server;
    (void)userdata;
    sc_push_event(SC_EVENT_SERVER_CONNECTED);
}

static void
sc_server_on_disconnected(struct sc_server *server, void *userdata) {
    (void)server;
    (void)userdata;
    LOGD("Server disconnected");
}

static void
sc_controller_on_ended(struct sc_controller *controller, bool error, void *userdata) {
    (void)controller;
    struct insynic_scrcpy *s = userdata;
    sc_mutex_lock(&s->server_mutex);
    if (error) {
        s->server_connection_failed = true;
    } else {
        s->server_connected = false;
    }
    sc_cond_signal(&s->server_cond);
    sc_mutex_unlock(&s->server_mutex);
}

static void
sc_video_demuxer_on_ended(struct sc_demuxer *demuxer, enum sc_demuxer_status status, void *userdata) {
    (void)demuxer;
    struct insynic_scrcpy *s = userdata;
    sc_mutex_lock(&s->server_mutex);
    if (status == SC_DEMUXER_STATUS_EOS) {
        s->server_connected = false;
    } else {
        s->server_connection_failed = true;
    }
    sc_cond_signal(&s->server_cond);
    sc_mutex_unlock(&s->server_mutex);
}

static void
sc_audio_demuxer_on_ended(struct sc_demuxer *demuxer, enum sc_demuxer_status status, void *userdata) {
    (void)demuxer;
    struct insynic_scrcpy *s = userdata;
    sc_mutex_lock(&s->server_mutex);
    if (status == SC_DEMUXER_STATUS_EOS) {
        s->server_connected = false;
    } else if (status == SC_DEMUXER_STATUS_ERROR) {
        s->server_connection_failed = true;
    }
    sc_cond_signal(&s->server_cond);
    sc_mutex_unlock(&s->server_mutex);
}

static bool
await_for_server(struct insynic_scrcpy *s, bool *connected) {
    sc_mutex_lock(&s->server_mutex);
    int timeout_ms = 30000;
    sc_tick deadline = sc_tick_now() + SC_TICK_FROM_MS(timeout_ms);
    while (!s->server_connected && !s->server_connection_failed && s->running) {
        bool timeout = sc_cond_timedwait(&s->server_cond, &s->server_mutex, deadline);
        if (timeout) {
            LOGE("await_for_server timeout after %dms", timeout_ms);
            break;
        }
    }
    
    bool result = s->server_connected;
    if (connected) {
        *connected = s->server_connected;
    }
    sc_mutex_unlock(&s->server_mutex);
    return result;
}

struct insynic_scrcpy *
insynic_scrcpy_create(const struct insynic_scrcpy_config *config) {
    struct insynic_scrcpy *s = calloc(1, sizeof(*s));
    if (!s) {
        LOGE("Failed to allocate insynic_scrcpy");
        return NULL;
    }

    s->options = scrcpy_options_default;
    s->state = INSYNIC_SCRCPY_STATE_IDLE;
    s->nsview_ptr = config->nsview_ptr;
    s->screen_on = true;

    if (!sc_mutex_init(&s->server_mutex)) {
        LOGE("Failed to initialize server mutex");
        free(s);
        return NULL;
    }
    if (!sc_cond_init(&s->server_cond)) {
        LOGE("Failed to initialize server cond");
        sc_mutex_destroy(&s->server_mutex);
        free(s);
        return NULL;
    }

    if (config->serial) {
        s->options.serial = strdup(config->serial);
    }

    if (config->server_path) {
        setenv("SCRCPY_SERVER_PATH", config->server_path, 1);
    }

    if (config->adb_path) {
        setenv("ADB", config->adb_path, 1);
        
        char adb_clear_cmd[512];
        snprintf(adb_clear_cmd, sizeof(adb_clear_cmd), "%s forward --remove-all", config->adb_path);
        system(adb_clear_cmd);
        
        char adb_kill_cmd[512];
        snprintf(adb_kill_cmd, sizeof(adb_kill_cmd), "%s shell pkill -f scrcpy-server", config->adb_path);
        system(adb_kill_cmd);
        
        SDL_Delay(100);
    }

    const char *icon_dir = "/Users/avenue/IDE/insynic/insynic/build/insynic.app/Contents/Resources";
    setenv("SCRCPY_ICON_DIR", icon_dir, 1);

    s->options.max_size = config->max_size;
    s->options.video = config->video_enabled;
    s->options.video_playback = config->video_enabled;
    s->options.audio = config->audio_enabled;
    s->options.audio_playback = config->audio_enabled;
    if (config->audio_enabled) {
        s->options.audio_source = SC_AUDIO_SOURCE_OUTPUT;
    }
    s->options.control = config->control_enabled;
    s->options.turn_screen_off = config->turn_screen_off;
    s->options.window = config->video_enabled;
    s->options.video_playback = config->video_enabled;
    s->options.render_fit = SC_RENDER_FIT_LETTERBOX;
    s->options.video_codec = SC_CODEC_H264;
    s->options.mipmaps = true;
    if (config->video_bit_rate > 0) {
        s->options.video_bit_rate = config->video_bit_rate * 1000000;
    }
    
    if (config->otg_mode) {
        s->options.keyboard_input_mode = SC_KEYBOARD_INPUT_MODE_AOA;
        s->options.mouse_input_mode = SC_MOUSE_INPUT_MODE_AOA;
        s->options.gamepad_input_mode = SC_GAMEPAD_INPUT_MODE_AOA;
    }

    if (config->window_x >= 0) {
        s->options.window_x = config->window_x;
    }
    if (config->window_y >= 0) {
        s->options.window_y = config->window_y;
    }
    if (config->window_width > 0) {
        s->options.window_width = config->window_width;
    }
    if (config->window_height > 0) {
        s->options.window_height = config->window_height;
    }

    if (config->max_fps > 0) {
        char *fps_buf = malloc(16);
        if (fps_buf) {
            snprintf(fps_buf, 16, "%d", config->max_fps);
            s->options.max_fps = fps_buf;
        }
    }

    s->nsview_ptr = config->nsview_ptr;

    return s;
}

struct insynic_scrcpy *
insynic_scrcpy_create_otg(const struct insynic_scrcpy_config *config) {
    struct insynic_scrcpy *s = calloc(1, sizeof(*s));
    if (!s) {
        LOGE("Failed to allocate insynic_scrcpy");
        return NULL;
    }

    s->options = scrcpy_options_default;
    s->state = INSYNIC_SCRCPY_STATE_IDLE;
    s->nsview_ptr = config->nsview_ptr;
    s->screen_on = true;

    if (!sc_mutex_init(&s->server_mutex)) {
        LOGE("Failed to initialize server mutex");
        free(s);
        return NULL;
    }
    if (!sc_cond_init(&s->server_cond)) {
        LOGE("Failed to initialize server cond");
        sc_mutex_destroy(&s->server_mutex);
        free(s);
        return NULL;
    }

    if (config->serial) {
        s->options.serial = strdup(config->serial);
    }

    const char *icon_dir = "/Users/avenue/IDE/insynic/insynic/build/insynic.app/Contents/Resources";
    setenv("SCRCPY_ICON_DIR", icon_dir, 1);

    s->options.video = false;
    s->options.video_playback = false;
    s->options.audio = false;
    s->options.audio_playback = false;
    s->options.control = true;
    s->options.window = true;
    s->options.otg = true;

    s->options.keyboard_input_mode = SC_KEYBOARD_INPUT_MODE_AOA;
    s->options.mouse_input_mode = SC_MOUSE_INPUT_MODE_AOA;
    s->options.gamepad_input_mode = SC_GAMEPAD_INPUT_MODE_AOA;

    if (config->window_width > 0) {
        s->options.window_width = config->window_width;
    }
    if (config->window_height > 0) {
        s->options.window_height = config->window_height;
    }

    s->nsview_ptr = config->nsview_ptr;

    return s;
}

void
insynic_scrcpy_destroy(struct insynic_scrcpy *s) {
    if (!s) {
        return;
    }
    sc_cond_destroy(&s->server_cond);
    sc_mutex_destroy(&s->server_mutex);
    free((char *)s->options.serial);
    free(s);
}

static void
insynic_scrcpy_init_main_thread(void *data) {
    struct insynic_scrcpy *s = data;
    struct scrcpy_options *opts = &s->options;
    struct sc_server_info *info = &s->server.info;
    const char *serial = s->server.serial;

    struct sc_file_pusher *fp = NULL;
    if (opts->window && opts->control) {
        if (!sc_file_pusher_init(&s->file_pusher, serial, opts->push_target)) {
            return;
        }
        fp = &s->file_pusher;
        s->file_pusher_initialized = true;
    }

    if (opts->video) {
        static const struct sc_demuxer_callbacks video_demuxer_cbs = {
            .on_ended = sc_video_demuxer_on_ended,
        };
        sc_demuxer_init(&s->video_demuxer, "video", s->server.video_socket,
                        &video_demuxer_cbs, s);
    }

    if (opts->audio) {
        static const struct sc_demuxer_callbacks audio_demuxer_cbs = {
            .on_ended = sc_audio_demuxer_on_ended,
        };
        sc_demuxer_init(&s->audio_demuxer, "audio", s->server.audio_socket,
                        &audio_demuxer_cbs, s);
    }

    bool needs_video_decoder = opts->video_playback;
    bool needs_audio_decoder = opts->audio_playback;

    if (needs_video_decoder) {
        sc_decoder_init(&s->video_decoder, "video");
        sc_packet_source_add_sink(&s->video_demuxer.packet_source,
                                  &s->video_decoder.packet_sink);
    }
    if (needs_audio_decoder) {
        sc_decoder_init(&s->audio_decoder, "audio");
        sc_packet_source_add_sink(&s->audio_demuxer.packet_source,
                                  &s->audio_decoder.packet_sink);
    }

    struct sc_controller *controller = NULL;
    struct sc_key_processor *kp = NULL;
    struct sc_mouse_processor *mp = NULL;

    if (opts->control) {
        static const struct sc_controller_callbacks controller_cbs = {
            .on_ended = sc_controller_on_ended,
        };
        if (!sc_controller_init(&s->controller, s->server.control_socket,
                               &controller_cbs, s)) {
            return;
        }

        controller = &s->controller;

        sc_keyboard_sdk_init(&s->keyboard_sdk, &s->controller,
                             opts->key_inject_mode, opts->forward_key_repeat);
        kp = &s->keyboard_sdk.key_processor;

        sc_mouse_sdk_init(&s->mouse_sdk, &s->controller, opts->mouse_hover);
        mp = &s->mouse_sdk.mouse_processor;

        sc_controller_configure(&s->controller, NULL, NULL);

        if (!sc_controller_start(&s->controller)) {
            return;
        }
        s->controller_started = true;
    }

    if (opts->window) {
        const char *window_title =
            opts->window_title ? opts->window_title : info->device_name;

        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

        if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
            LOGE("Could not initialize SDL video: %s", SDL_GetError());
            return;
        }

        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            LOGE("Could not initialize SDL audio: %s", SDL_GetError());
            return;
        }

        struct sc_screen_params screen_params = {
            .video = opts->video_playback,
            .camera = opts->video_source == SC_VIDEO_SOURCE_CAMERA,
            .flex_display = opts->flex_display,
            .controller = controller,
            .fp = fp,
            .kp = kp,
            .mp = mp,
            .mouse_bindings = opts->mouse_bindings,
            .legacy_paste = opts->legacy_paste,
            .clipboard_autosync = opts->clipboard_autosync,
            .shortcut_mods = opts->shortcut_mods,
            .window_title = window_title,
            .always_on_top = opts->always_on_top,
            .window_x = opts->window_x,
            .window_y = opts->window_y,
            .window_width = opts->window_width,
            .window_height = opts->window_height,
            .background_color = opts->background_color,
            .window_aspect_ratio_lock = opts->window_aspect_ratio_lock,
            .window_borderless = opts->window_borderless,
            .render_fit = opts->render_fit,
            .orientation = opts->display_orientation,
            .mipmaps = opts->mipmaps,
            .fullscreen = opts->fullscreen,
            .start_fps_counter = opts->start_fps_counter,
            .nsview_ptr = s->nsview_ptr,
        };

        if (!sc_screen_init(&s->screen, &screen_params)) {
            LOGE("sc_screen_init failed");
            return;
        }
        s->screen_initialized = true;

        SDL_AddEventWatch(insynic_scrcpy_event_watch, s);

        if (opts->video) {
            sc_demuxer_start(&s->video_demuxer);
            s->video_demuxer_started = true;
        }

        if (opts->audio) {
            sc_demuxer_start(&s->audio_demuxer);
            s->audio_demuxer_started = true;
        }

        if (opts->video_playback) {
            struct sc_frame_source *src = &s->video_decoder.frame_source;
            sc_frame_source_add_sink(src, &s->screen.frame_sink);
        }
    }

    if (opts->audio_playback) {
        sc_audio_player_init(&s->audio_player, opts->audio_buffer,
                             opts->audio_output_buffer);
        sc_frame_source_add_sink(&s->audio_decoder.frame_source,
                                 &s->audio_player.frame_sink);
    }

    if (opts->video && !s->video_demuxer_started) {
        if (!sc_demuxer_start(&s->video_demuxer)) {
            LOGE("Failed to start video demuxer");
            return;
        }
        s->video_demuxer_started = true;
    }

    if (opts->audio && !s->audio_demuxer_started) {
        if (!sc_demuxer_start(&s->audio_demuxer)) {
            LOGE("Failed to start audio demuxer");
            return;
        }
        s->audio_demuxer_started = true;
    }

    s->window_ready = true;
    sc_mutex_lock(&s->server_mutex);
    sc_cond_signal(&s->server_cond);
    sc_mutex_unlock(&s->server_mutex);
}

static int
insynic_scrcpy_thread(void *data) {
    struct insynic_scrcpy *s = data;
    struct scrcpy_options *opts = &s->options;

    uint32_t scid = generate_scid();

    struct sc_server_params params = {
        .scid = scid,
        .req_serial = opts->serial,
        .log_level = opts->log_level,
        .min_size_alignment = opts->min_size_alignment,
        .video_codec = opts->video_codec,
        .audio_codec = opts->audio_codec,
        .video_source = opts->video_source,
        .audio_source = opts->audio_source,
        .crop = opts->crop,
        .port_range = opts->port_range,
        .max_size = opts->max_size,
        .video_bit_rate = opts->video_bit_rate,
        .audio_bit_rate = opts->audio_bit_rate,
        .max_fps = opts->max_fps,
        .control = opts->control,
        .video = opts->video,
        .audio = opts->audio,
        .stay_awake = opts->stay_awake,
        .power_on = opts->power_on,
        .cleanup = opts->cleanup,
        .clipboard_autosync = opts->clipboard_autosync,
        .downsize_on_error = opts->downsize_on_error,
        .tcpip = opts->tcpip,
        .tcpip_dst = opts->tcpip_dst,
        .select_tcpip = opts->select_tcpip,
        .select_usb = opts->select_usb,
        .display_id = opts->display_id,
        .new_display = opts->new_display,
        .capture_orientation = opts->capture_orientation,
        .capture_orientation_lock = opts->capture_orientation_lock,
        .display_ime_policy = opts->display_ime_policy,
        .video_codec_options = opts->video_codec_options,
        .audio_codec_options = opts->audio_codec_options,
        .video_encoder = opts->video_encoder,
        .audio_encoder = opts->audio_encoder,
        .force_adb_forward = opts->force_adb_forward,
        .power_off_on_close = opts->power_off_on_close,
        .kill_adb_on_close = opts->kill_adb_on_close,
        .screen_off_timeout = opts->screen_off_timeout,
        .show_touches = opts->show_touches,
        .angle = opts->angle,
        .list = opts->list,
        .flex_display = opts->flex_display,
        .audio_dup = opts->audio_dup,
        .camera_high_speed = opts->camera_high_speed,
        .camera_torch = opts->camera_torch,
        .vd_destroy_content = opts->vd_destroy_content,
        .vd_system_decorations = opts->vd_system_decorations,
        .keep_active = opts->keep_active,
    };

    static const struct sc_server_callbacks server_cbs = {
        .on_connection_failed = sc_server_on_connection_failed,
        .on_connected = sc_server_on_connected,
        .on_disconnected = sc_server_on_disconnected,
    };

    sc_main_thread_init();

    if (!sc_server_init(&s->server, &params, &server_cbs, s)) {
        LOGE("Failed to initialize sc_server");
        set_state(s, INSYNIC_SCRCPY_STATE_ERROR);
        return -1;
    }

    sc_sdl_set_hints(opts->render_driver, opts->disable_screensaver);

    set_state(s, INSYNIC_SCRCPY_STATE_CONNECTING);

    if (!sc_server_start(&s->server)) {
        LOGE("Failed to start sc_server");
        set_state(s, INSYNIC_SCRCPY_STATE_ERROR);
        goto server_fail;
    }
    s->server_started = true;

    sc_mutex_lock(&s->server_mutex);
    while (s->running) {
        sc_cond_wait(&s->server_cond, &s->server_mutex);
    }
    sc_mutex_unlock(&s->server_mutex);

end:
    s->running = false;

    if (s->controller_started) {
        sc_controller_stop(&s->controller);
    }
    if (s->file_pusher_initialized) {
        sc_file_pusher_stop(&s->file_pusher);
    }
    if (s->screen_initialized) {
        sc_screen_interrupt(&s->screen);
    }

    if (s->server_started) {
        sc_server_stop(&s->server);
    }

    if (s->screen_initialized) {
        sc_screen_hide_window(&s->screen);
    }

    if (s->video_demuxer_started) {
        sc_demuxer_join(&s->video_demuxer);
    }
    if (s->audio_demuxer_started) {
        sc_demuxer_join(&s->audio_demuxer);
    }

    if (s->screen_initialized) {
        sc_screen_join(&s->screen);
        sc_screen_destroy(&s->screen);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        SDL_RemoveEventWatch(insynic_scrcpy_event_watch, s);
    }

    if (s->controller_started) {
        sc_controller_join(&s->controller);
    }
    sc_controller_destroy(&s->controller);

    if (s->file_pusher_initialized) {
        sc_file_pusher_join(&s->file_pusher);
        sc_file_pusher_destroy(&s->file_pusher);
    }

    if (s->server_started) {
        sc_server_join(&s->server);
    }

    sc_server_destroy(&s->server);
    sc_main_thread_destroy();

    return 0;

server_fail:
    sc_server_destroy(&s->server);
    return -1;
}

static void *
insynic_scrcpy_otg_main_thread(void *data) {
    struct insynic_scrcpy *s = data;
    struct scrcpy_options *opts = &s->options;

    set_state(s, INSYNIC_SCRCPY_STATE_CONNECTING);

    enum scrcpy_exit_code ret = scrcpy_otg(opts);

    if (ret == SCRCPY_EXIT_DISCONNECTED) {
        set_state(s, INSYNIC_SCRCPY_STATE_DISCONNECTED);
    } else if (ret == SCRCPY_EXIT_FAILURE) {
        set_state(s, INSYNIC_SCRCPY_STATE_ERROR);
    } else {
        set_state(s, INSYNIC_SCRCPY_STATE_DISCONNECTED);
    }

    s->running = false;

    return NULL;
}

static int
insynic_scrcpy_otg_thread(void *data) {
    struct insynic_scrcpy *s = data;

    sc_main_thread_init();

    bool ok = sc_run_on_main_thread(insynic_scrcpy_otg_main_thread, s, false);
    if (!ok) {
        LOGE("Failed to schedule OTG initialization on main thread");
        set_state(s, INSYNIC_SCRCPY_STATE_ERROR);
        s->running = false;
        sc_main_thread_destroy();
        return -1;
    }

    while (s->running) {
        SDL_Delay(100);
    }

    sc_main_thread_destroy();

    return 0;
}

bool
insynic_scrcpy_start(struct insynic_scrcpy *s) {
    if (s->running) {
        return false;
    }
    s->running = true;

    if (s->options.otg) {
        if (!sc_thread_create(&s->thread, insynic_scrcpy_otg_thread, "insynic_otg", s)) {
            s->running = false;
            return false;
        }
    } else {
        if (!sc_thread_create(&s->thread, insynic_scrcpy_thread, "insynic", s)) {
            s->running = false;
            return false;
        }
    }

    return true;
}

bool
insynic_scrcpy_run_main_thread(struct insynic_scrcpy *s) {
    if (!s->running) {
        return false;
    }

    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                return true;
            case SC_EVENT_SERVER_CONNECTION_FAILED:
                LOGE("Server connection failed");
                set_state(s, INSYNIC_SCRCPY_STATE_ERROR);
                return false;
            case SC_EVENT_SERVER_CONNECTED:
                set_state(s, INSYNIC_SCRCPY_STATE_CONNECTED);
                insynic_scrcpy_init_main_thread(s);
                break;
            case SC_EVENT_DEVICE_DISCONNECTED:
                LOGW("Device disconnected");
                set_state(s, INSYNIC_SCRCPY_STATE_DISCONNECTED);
                return true;
            case SC_EVENT_DEMUXER_ERROR:
                LOGE("Demuxer error");
                set_state(s, INSYNIC_SCRCPY_STATE_ERROR);
                return true;
            default:
                if (s->screen_initialized) {
                    sc_screen_handle_event(&s->screen, &event);
                }
                break;
        }
    }

    LOGE("SDL_WaitEvent() error: %s", SDL_GetError());
    return false;
}

void
insynic_scrcpy_stop(struct insynic_scrcpy *s) {
    if (!s->running) {
        return;
    }
    
    s->running = false;
    
    sc_push_event(SDL_EVENT_QUIT);
    
    sc_mutex_lock(&s->server_mutex);
    sc_cond_signal(&s->server_cond);
    sc_mutex_unlock(&s->server_mutex);
    
    SDL_PumpEvents();
    insynic_scrcpy_process_events(s);
    
    sc_thread_join(&s->thread, NULL);
    
    sc_main_thread_stop();
    
    set_state(s, INSYNIC_SCRCPY_STATE_DISCONNECTED);
}

bool
insynic_scrcpy_process_events(struct insynic_scrcpy *s) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SC_EVENT_SERVER_CONNECTED:
                if (!s->window_ready) {
                    insynic_scrcpy_init_main_thread(s);
                }
                set_state(s, INSYNIC_SCRCPY_STATE_CONNECTED);
                break;
            case SC_EVENT_SERVER_CONNECTION_FAILED:
                LOGE("Server connection failed");
                set_state(s, INSYNIC_SCRCPY_STATE_ERROR);
                break;
            case SC_EVENT_DEVICE_DISCONNECTED:
                LOGW("Device disconnected");
                set_state(s, INSYNIC_SCRCPY_STATE_DISCONNECTED);
                break;
            default:
                if (s->screen_initialized) {
                    sc_screen_handle_event(&s->screen, &event);
                }
                break;
        }
    }
    return true;
}

enum insynic_scrcpy_state
insynic_scrcpy_get_state(struct insynic_scrcpy *s) {
    return s->state;
}

void
insynic_scrcpy_set_state_callback(struct insynic_scrcpy *s,
                                  insynic_scrcpy_state_cb cb,
                                  void *userdata) {
    s->state_cb = cb;
    s->state_cb_userdata = userdata;
}

void
insynic_scrcpy_set_event_callback(struct insynic_scrcpy *s,
                                  insynic_scrcpy_event_cb cb,
                                  void *userdata) {
    s->event_cb = cb;
    s->event_cb_userdata = userdata;
}

static bool
insynic_scrcpy_event_watch(void *userdata, SDL_Event *event) {
    struct insynic_scrcpy *s = userdata;
    if (s->event_cb) {
        s->event_cb(s->event_cb_userdata);
    }
    return false;
}

bool
insynic_scrcpy_inject_keycode(struct insynic_scrcpy *s, uint32_t keycode,
                              bool down) {
    if (!s->controller_started) {
        return false;
    }
    struct sc_control_msg msg;
    msg.type = SC_CONTROL_MSG_TYPE_INJECT_KEYCODE;
    msg.inject_keycode.action = down ? AKEY_EVENT_ACTION_DOWN
                                     : AKEY_EVENT_ACTION_UP;
    msg.inject_keycode.keycode = keycode;
    msg.inject_keycode.repeat = 0;
    msg.inject_keycode.metastate = 0;
    return sc_controller_push_msg(&s->controller, &msg);
}

bool
insynic_scrcpy_inject_back(struct insynic_scrcpy *s) {
    insynic_scrcpy_inject_keycode(s, AKEYCODE_BACK, true);
    SDL_Delay(50);
    return insynic_scrcpy_inject_keycode(s, AKEYCODE_BACK, false);
}

bool
insynic_scrcpy_inject_home(struct insynic_scrcpy *s) {
    insynic_scrcpy_inject_keycode(s, AKEYCODE_HOME, true);
    SDL_Delay(50);
    return insynic_scrcpy_inject_keycode(s, AKEYCODE_HOME, false);
}

bool
insynic_scrcpy_inject_recent(struct insynic_scrcpy *s) {
    insynic_scrcpy_inject_keycode(s, AKEYCODE_APP_SWITCH, true);
    SDL_Delay(50);
    return insynic_scrcpy_inject_keycode(s, AKEYCODE_APP_SWITCH, false);
}

bool
insynic_scrcpy_inject_menu(struct insynic_scrcpy *s) {
    insynic_scrcpy_inject_keycode(s, AKEYCODE_MENU, true);
    SDL_Delay(50);
    return insynic_scrcpy_inject_keycode(s, AKEYCODE_MENU, false);
}

bool
insynic_scrcpy_inject_power(struct insynic_scrcpy *s) {
    return insynic_scrcpy_inject_keycode(s, AKEYCODE_POWER, true);
}

bool
insynic_scrcpy_inject_volume_up(struct insynic_scrcpy *s) {
    return insynic_scrcpy_inject_keycode(s, AKEYCODE_VOLUME_UP, true);
}

bool
insynic_scrcpy_inject_volume_down(struct insynic_scrcpy *s) {
    return insynic_scrcpy_inject_keycode(s, AKEYCODE_VOLUME_DOWN, true);
}

bool
insynic_scrcpy_expand_notification_panel(struct insynic_scrcpy *s) {
    insynic_scrcpy_inject_keycode(s, AKEYCODE_NOTIFICATION, true);
    SDL_Delay(50);
    return insynic_scrcpy_inject_keycode(s, AKEYCODE_NOTIFICATION, false);
}

bool
insynic_scrcpy_expand_settings_panel(struct insynic_scrcpy *s) {
    insynic_scrcpy_inject_keycode(s, AKEYCODE_NOTIFICATION, true);
    SDL_Delay(50);
    return insynic_scrcpy_inject_keycode(s, AKEYCODE_NOTIFICATION, false);
}

bool
insynic_scrcpy_collapse_panels(struct insynic_scrcpy *s) {
    insynic_scrcpy_inject_keycode(s, AKEYCODE_ESCAPE, true);
    SDL_Delay(50);
    return insynic_scrcpy_inject_keycode(s, AKEYCODE_ESCAPE, false);
}

bool
insynic_scrcpy_rotate_device(struct insynic_scrcpy *s) {
    if (!s->controller_started) {
        return false;
    }
    struct sc_control_msg msg;
    msg.type = SC_CONTROL_MSG_TYPE_ROTATE_DEVICE;
    return sc_controller_push_msg(&s->controller, &msg);
}

bool
insynic_scrcpy_set_display_power(struct insynic_scrcpy *s, bool on) {
    if (!s->controller_started) {
        return false;
    }
    struct sc_control_msg msg;
    msg.type = SC_CONTROL_MSG_TYPE_SET_DISPLAY_POWER;
    msg.set_display_power.on = on;
    return sc_controller_push_msg(&s->controller, &msg);
}

bool
insynic_scrcpy_toggle_display(struct insynic_scrcpy *s) {
    if (!s->controller_started) {
        return false;
    }
    
    s->screen_on = !s->screen_on;
    return insynic_scrcpy_set_display_power(s, s->screen_on);
}

bool
insynic_scrcpy_inject_touch_action(struct insynic_scrcpy *s, int x, int y, int width, int height, bool is_down) {
    if (!s->controller_started) {
        return false;
    }

    struct sc_control_msg msg;
    msg.type = SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT;
    msg.inject_touch_event.action = is_down ? AMOTION_EVENT_ACTION_DOWN : AMOTION_EVENT_ACTION_UP;
    msg.inject_touch_event.action_button = AMOTION_EVENT_BUTTON_PRIMARY;
    msg.inject_touch_event.buttons = is_down ? AMOTION_EVENT_BUTTON_PRIMARY : 0;
    msg.inject_touch_event.pointer_id = SC_POINTER_ID_GENERIC_FINGER;
    msg.inject_touch_event.position.point.x = x;
    msg.inject_touch_event.position.point.y = y;
    msg.inject_touch_event.position.screen_size.width = width;
    msg.inject_touch_event.position.screen_size.height = height;
    msg.inject_touch_event.pressure = is_down ? 1.0f : 0.0f;
    return sc_controller_push_msg(&s->controller, &msg);
}

bool
insynic_scrcpy_inject_touch(struct insynic_scrcpy *s, int x, int y, int width, int height) {
    if (!insynic_scrcpy_inject_touch_action(s, x, y, width, height, true)) {
        return false;
    }
    SDL_Delay(50);
    return insynic_scrcpy_inject_touch_action(s, x, y, width, height, false);
}

bool
insynic_scrcpy_get_device_size(struct insynic_scrcpy *s, int *width, int *height) {
    if (!s || !s->screen_initialized) {
        return false;
    }
    *width = s->screen.content_size.width;
    *height = s->screen.content_size.height;
    return true;
}

void *
insynic_scrcpy_get_nsview(struct insynic_scrcpy *s) {
    (void)s;
    return NULL;
}

bool
insynic_scrcpy_get_window_position(struct insynic_scrcpy *s, int *x, int *y) {
    if (!s || !s->screen_initialized || !s->screen.window) {
        return false;
    }
    SDL_GetWindowPosition(s->screen.window, x, y);
    return true;
}

bool
insynic_scrcpy_get_window_size(struct insynic_scrcpy *s, int *width, int *height) {
    if (!s || !s->screen_initialized || !s->screen.window) {
        return false;
    }
    SDL_GetWindowSize(s->screen.window, width, height);
    return true;
}