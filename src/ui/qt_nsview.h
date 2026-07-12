#ifndef QT_NSVIEW_H
#define QT_NSVIEW_H

#ifdef __APPLE__
extern "C" {
    void* getQtWindowNSView(void* windowHandle);
}
#endif

#endif