#import <Cocoa/Cocoa.h>

void insynic_macos_show_window(void *sdl_window) {
    FILE *f = fopen("/tmp/scrcpy_error.log", "a");
    if (f) { fprintf(f, "insynic_macos_show_window called\n"); fclose(f); }
    
    NSArray *windows = [NSApp windows];
    
    FILE *f_count = fopen("/tmp/scrcpy_error.log", "a");
    if (f_count) { fprintf(f_count, "Number of NSWindows: %lu\n", [windows count]); fclose(f_count); }
    
    for (NSWindow *ns_win in windows) {
        const char *title = [[ns_win title] UTF8String];
        
        FILE *f_enum = fopen("/tmp/scrcpy_error.log", "a");
        if (f_enum) { fprintf(f_enum, "Found NSWindow: %p, title: %s\n", ns_win, title ? title : "(null)"); fclose(f_enum); }
        
        [ns_win setIsVisible:YES];
        [ns_win orderFrontRegardless];
        [ns_win makeKeyAndOrderFront:nil];
    }
}