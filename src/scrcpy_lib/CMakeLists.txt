set(SCRCPY_ROOT "${CMAKE_SOURCE_DIR}/scrcpy")
set(SCRCPY_APP_SRC "${SCRCPY_ROOT}/app/src")

set(SCRCPY_SOURCES
    ${SCRCPY_APP_SRC}/adb/adb.c
    ${SCRCPY_APP_SRC}/adb/adb_device.c
    ${SCRCPY_APP_SRC}/adb/adb_parser.c
    ${SCRCPY_APP_SRC}/adb/adb_tunnel.c
    ${SCRCPY_APP_SRC}/audio_player.c
    ${SCRCPY_APP_SRC}/audio_regulator.c
    ${SCRCPY_APP_SRC}/clock.c
    ${SCRCPY_APP_SRC}/compat.c
    ${SCRCPY_APP_SRC}/control_msg.c
    ${SCRCPY_APP_SRC}/controller.c
    ${SCRCPY_APP_SRC}/decoder.c
    ${SCRCPY_APP_SRC}/delay_buffer.c
    ${SCRCPY_APP_SRC}/demuxer.c
    ${SCRCPY_APP_SRC}/device_msg.c
    ${SCRCPY_APP_SRC}/disconnect.c
    ${SCRCPY_APP_SRC}/events.c
    ${SCRCPY_APP_SRC}/file_pusher.c
    ${SCRCPY_APP_SRC}/fps_counter.c
    ${SCRCPY_APP_SRC}/frame_buffer.c
    ${SCRCPY_APP_SRC}/input_manager.c
    ${SCRCPY_APP_SRC}/icon.c
    ${SCRCPY_APP_SRC}/keyboard_sdk.c
    ${SCRCPY_APP_SRC}/mouse_capture.c
    ${SCRCPY_APP_SRC}/mouse_sdk.c
    ${SCRCPY_APP_SRC}/opengl.c
    ${SCRCPY_APP_SRC}/options.c
    ${SCRCPY_APP_SRC}/packet_merger.c
    ${SCRCPY_APP_SRC}/receiver.c
    ${SCRCPY_APP_SRC}/recorder.c
    ${SCRCPY_APP_SRC}/screen.c
    ${SCRCPY_APP_SRC}/sdl_hints.c
    ${SCRCPY_APP_SRC}/server.c
    ${SCRCPY_APP_SRC}/texture.c
    ${SCRCPY_APP_SRC}/version.c
    ${SCRCPY_APP_SRC}/hid/hid_gamepad.c
    ${SCRCPY_APP_SRC}/hid/hid_keyboard.c
    ${SCRCPY_APP_SRC}/hid/hid_mouse.c
    ${SCRCPY_APP_SRC}/trait/frame_source.c
    ${SCRCPY_APP_SRC}/trait/packet_source.c
    ${SCRCPY_APP_SRC}/uhid/gamepad_uhid.c
    ${SCRCPY_APP_SRC}/uhid/keyboard_uhid.c
    ${SCRCPY_APP_SRC}/uhid/mouse_uhid.c
    ${SCRCPY_APP_SRC}/uhid/uhid_output.c
    ${SCRCPY_APP_SRC}/util/acksync.c
    ${SCRCPY_APP_SRC}/util/audiobuf.c
    ${SCRCPY_APP_SRC}/util/average.c
    ${SCRCPY_APP_SRC}/util/env.c
    ${SCRCPY_APP_SRC}/util/file.c
    ${SCRCPY_APP_SRC}/util/intmap.c
    ${SCRCPY_APP_SRC}/util/intr.c
    ${SCRCPY_APP_SRC}/util/log.c
    ${SCRCPY_APP_SRC}/util/memory.c
    ${SCRCPY_APP_SRC}/util/net.c
    ${SCRCPY_APP_SRC}/util/net_intr.c
    ${SCRCPY_APP_SRC}/util/process.c
    ${SCRCPY_APP_SRC}/util/process_intr.c
    ${SCRCPY_APP_SRC}/util/rand.c
    ${SCRCPY_APP_SRC}/util/sdl.c
    ${SCRCPY_APP_SRC}/util/strbuf.c
    ${SCRCPY_APP_SRC}/util/str.c
    ${SCRCPY_APP_SRC}/util/term.c
    ${SCRCPY_APP_SRC}/util/thread.c
    ${SCRCPY_APP_SRC}/util/tick.c
    ${SCRCPY_APP_SRC}/util/timeout.c
)

if(APPLE)
    list(APPEND SCRCPY_SOURCES
        ${SCRCPY_APP_SRC}/sys/unix/file.c
        ${SCRCPY_APP_SRC}/sys/unix/process.c
    )
endif()

option(HAVE_USB "Enable USB/AOA/OTG support" ON)
if(HAVE_USB)
    list(APPEND SCRCPY_SOURCES
        ${SCRCPY_APP_SRC}/usb/aoa_hid.c
        ${SCRCPY_APP_SRC}/usb/gamepad_aoa.c
        ${SCRCPY_APP_SRC}/usb/keyboard_aoa.c
        ${SCRCPY_APP_SRC}/usb/mouse_aoa.c
        ${SCRCPY_APP_SRC}/usb/scrcpy_otg.c
        ${SCRCPY_APP_SRC}/usb/usb.c
    )
endif()

add_library(scrcpy_static STATIC ${SCRCPY_SOURCES})

target_include_directories(scrcpy_static BEFORE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SCRCPY_APP_SRC}
    ${SDL3_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
    ${AVCODEC_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
    ${SWRESAMPLE_INCLUDE_DIRS}
    ${LIBUSB_INCLUDE_DIRS}
)

target_compile_definitions(scrcpy_static PUBLIC
    _GNU_SOURCE
    _POSIX_C_SOURCE=200809L
    _XOPEN_SOURCE=700
    _DARWIN_C_SOURCE
    SCRCPY_LAVF_REQUIRES_REGISTER_ALL=0
    PORTABLE
)

if(HAVE_USB)
    target_compile_definitions(scrcpy_static PUBLIC HAVE_USB=1)
endif()

target_compile_definitions(scrcpy_static PRIVATE
    SCRCPY_VERSION="4.0"
    DEFAULT_LOCAL_PORT_RANGE_FIRST="27183"
    DEFAULT_LOCAL_PORT_RANGE_LAST="27199"
)

target_link_libraries(scrcpy_static PUBLIC
    ${SDL3_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVCODEC_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${SWRESAMPLE_LIBRARIES}
    ${LIBUSB_LIBRARIES}
)

add_library(insynic_scrcpy_lib STATIC
    insynic_scrcpy.c
)

target_include_directories(insynic_scrcpy_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(insynic_scrcpy_lib PUBLIC
    scrcpy_static
)
