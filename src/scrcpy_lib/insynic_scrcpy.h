#ifndef INSYNIC_SCRCPY_H
#define INSYNIC_SCRCPY_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

struct insynic_scrcpy;
struct sc_screen;

enum insynic_scrcpy_state {
    INSYNIC_SCRCPY_STATE_IDLE = 0,
    INSYNIC_SCRCPY_STATE_CONNECTING = 1,
    INSYNIC_SCRCPY_STATE_CONNECTED = 2,
    INSYNIC_SCRCPY_STATE_DISCONNECTED = 3,
    INSYNIC_SCRCPY_STATE_ERROR = 4,
};

typedef void (*insynic_scrcpy_state_cb)(enum insynic_scrcpy_state state,
                                        void *userdata);

struct insynic_scrcpy_config {
    const char *serial;
    const char *server_path;
    const char *adb_path;
    int max_size;
    int max_fps;
    uint32_t video_bit_rate;
    bool video_enabled;
    bool audio_enabled;
    uint32_t audio_bit_rate;
    int audio_codec;  // 0=OPUS, 1=AAC
    int audio_source; // 0=OUTPUT, 1=MIC, 2=PLAYBACK
    bool control_enabled;
    bool turn_screen_off;
    bool stay_awake;
    bool power_on;
    bool disable_screensaver;
    bool otg_mode;
    // Recording
    const char *record_filename;  // NULL = no recording
    int record_format;            // 0=auto,1=mp4,2=mkv,3=m4a,4=mka,5=opus,6=aac,7=flac,8=wav
    bool record_video;            // record video stream
    bool record_audio;            // record audio stream
    int window_x;
    int window_y;
    int window_width;
    int window_height;
    void *nsview_ptr;
};

struct insynic_scrcpy *
insynic_scrcpy_create(const struct insynic_scrcpy_config *config);

struct insynic_scrcpy *
insynic_scrcpy_create_otg(const struct insynic_scrcpy_config *config);

bool
insynic_scrcpy_init_sdl(void);

void
insynic_scrcpy_destroy(struct insynic_scrcpy *s);

bool
insynic_scrcpy_start(struct insynic_scrcpy *s);

void
insynic_scrcpy_stop(struct insynic_scrcpy *s);

void
insynic_scrcpy_request_stop(struct insynic_scrcpy *s);

bool
insynic_scrcpy_is_running(struct insynic_scrcpy *s);

bool
insynic_scrcpy_is_thread_exited(struct insynic_scrcpy *s);

// Block until the scrcpy thread fully exits.
// This is used as a fallback when polling with insynic_scrcpy_is_thread_exited()
// times out: the main thread blocks, so SDL_PollEvent() is no longer called,
// allowing the screen thread's SDL_WaitEvent() to receive events and exit.
void
insynic_scrcpy_join(struct insynic_scrcpy *s);

bool
insynic_scrcpy_process_events(struct insynic_scrcpy *s);

void
insynic_scrcpy_handle_event(struct insynic_scrcpy *s, const SDL_Event *event);

SDL_Window *
insynic_scrcpy_get_window(struct insynic_scrcpy *s);

struct sc_screen *
insynic_scrcpy_get_screen(struct insynic_scrcpy *s);

bool
insynic_scrcpy_init_main_thread_safe(struct insynic_scrcpy *s);

bool
insynic_scrcpy_run_main_thread(struct insynic_scrcpy *s);

enum insynic_scrcpy_state
insynic_scrcpy_get_state(struct insynic_scrcpy *s);

void
insynic_scrcpy_set_state_callback(struct insynic_scrcpy *s,
                                  insynic_scrcpy_state_cb cb,
                                  void *userdata);

bool
insynic_scrcpy_inject_keycode(struct insynic_scrcpy *s, uint32_t keycode,
                              bool down);

bool
insynic_scrcpy_inject_back(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_home(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_recent(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_menu(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_power(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_volume_up(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_volume_down(struct insynic_scrcpy *s);

bool
insynic_scrcpy_expand_notification_panel(struct insynic_scrcpy *s);

bool
insynic_scrcpy_expand_settings_panel(struct insynic_scrcpy *s);

bool
insynic_scrcpy_collapse_panels(struct insynic_scrcpy *s);

bool
insynic_scrcpy_rotate_device(struct insynic_scrcpy *s);

bool
insynic_scrcpy_set_display_power(struct insynic_scrcpy *s, bool on);

bool
insynic_scrcpy_toggle_display(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_touch_action(struct insynic_scrcpy *s, int x, int y, int width, int height, bool is_down);

bool
insynic_scrcpy_inject_touch(struct insynic_scrcpy *s, int x, int y, int width, int height);

bool
insynic_scrcpy_get_device_size(struct insynic_scrcpy *s, int *width, int *height);

void *
insynic_scrcpy_get_nsview(struct insynic_scrcpy *s);

bool
insynic_scrcpy_get_window_position(struct insynic_scrcpy *s, int *x, int *y);

bool
insynic_scrcpy_get_window_size(struct insynic_scrcpy *s, int *width, int *height);

bool
insynic_scrcpy_is_screen_initialized(struct insynic_scrcpy *s);

void
insynic_scrcpy_hide_screen(struct insynic_scrcpy *s);

typedef void (*insynic_scrcpy_event_cb)(void *userdata);

void
insynic_scrcpy_set_event_callback(struct insynic_scrcpy *s,
                                  insynic_scrcpy_event_cb cb,
                                  void *userdata);

#ifdef __cplusplus
}
#endif

#endif
