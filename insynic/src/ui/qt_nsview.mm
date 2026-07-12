#include "qt_nsview.h"
#include <Cocoa/Cocoa.h>

extern "C" {
    void* getQtWindowNSView(void* windowHandle) {
        NSWindow *window = reinterpret_cast<NSWindow*>(windowHandle);
        if (window) {
            return [window contentView];
        }
        return NULL;
    }
}