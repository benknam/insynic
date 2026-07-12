#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

extern "C" {
    void createStatusBarMenu();
    void setupNativeMenuBar();
    void removeStatusBarMenu();
    void recreateMacOSMenus();
}

static NSStatusItem *statusItem = nil;

void setupNativeMenuBar() {
    NSApplication *app = [NSApplication sharedApplication];
    
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];
    
    [app activateIgnoringOtherApps:YES];
    
    NSMenu *existingMainMenu = [app mainMenu];
    if (existingMainMenu) {
        [existingMainMenu removeAllItems];
    }
    
    NSMenu *mainMenu = [[NSMenu alloc] initWithTitle:@"insynic"];
    [app setMainMenu:mainMenu];
    
    NSMenuItem *appMenuItem = [mainMenu addItemWithTitle:@"insynic" action:nil keyEquivalent:@""];
    NSMenu *appMenu = [[NSMenu alloc] initWithTitle:@"insynic"];
    [appMenuItem setSubmenu:appMenu];
    
    [appMenu addItemWithTitle:@"About insynic" action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
    
    [appMenu addItem:[NSMenuItem separatorItem]];
    
    NSMenuItem *settingMenuItem = [appMenu addItemWithTitle:@"Setting" action:nil keyEquivalent:@""];
    NSMenu *settingMenu = [[NSMenu alloc] initWithTitle:@"Setting"];
    [settingMenuItem setSubmenu:settingMenu];
    
    NSMenuItem *languageMenuItem = [settingMenu addItemWithTitle:@"Language" action:nil keyEquivalent:@""];
    NSMenu *languageMenu = [[NSMenu alloc] initWithTitle:@"Language"];
    [languageMenuItem setSubmenu:languageMenu];
    
    [languageMenu addItemWithTitle:@"English" action:nil keyEquivalent:@""];
    [languageMenu addItemWithTitle:@"Chinese" action:nil keyEquivalent:@""];
    
    [appMenu addItem:[NSMenuItem separatorItem]];
    
    [appMenu addItemWithTitle:@"Quit insynic" action:@selector(terminate:) keyEquivalent:@"q"];
    
    NSMenuItem *fileMenuItem = [mainMenu addItemWithTitle:@"File" action:nil keyEquivalent:@""];
    NSMenu *fileMenu = [[NSMenu alloc] initWithTitle:@"File"];
    [fileMenuItem setSubmenu:fileMenu];
    
    NSMenuItem *editMenuItem = [mainMenu addItemWithTitle:@"Edit" action:nil keyEquivalent:@""];
    NSMenu *editMenu = [[NSMenu alloc] initWithTitle:@"Edit"];
    [editMenuItem setSubmenu:editMenu];
    
    [editMenu addItemWithTitle:@"Cut" action:@selector(cut:) keyEquivalent:@"x"];
    [editMenu addItemWithTitle:@"Copy" action:@selector(copy:) keyEquivalent:@"c"];
    [editMenu addItemWithTitle:@"Paste" action:@selector(paste:) keyEquivalent:@"v"];
    [editMenu addItemWithTitle:@"Select All" action:@selector(selectAll:) keyEquivalent:@"a"];
    
    NSMenuItem *windowMenuItem = [mainMenu addItemWithTitle:@"Window" action:nil keyEquivalent:@""];
    NSMenu *windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    [windowMenuItem setSubmenu:windowMenu];
    
    [windowMenu addItemWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
    [windowMenu addItemWithTitle:@"Zoom" action:@selector(performZoom:) keyEquivalent:@""];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        ProcessSerialNumber psn = {0, kCurrentProcess};
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
        SetFrontProcess(&psn);

        [app activateIgnoringOtherApps:YES];
    });
}

void createStatusBarMenu() {
    if (statusItem) {
        removeStatusBarMenu();
    }
    
    NSStatusBar *statusBar = [NSStatusBar systemStatusBar];
    statusItem = [statusBar statusItemWithLength:NSVariableStatusItemLength];
    
    if (!statusItem) {
        return;
    }
    
    [statusItem setTitle:@"insynic"];
    [statusItem setHighlightMode:YES];
    
    NSMenu *statusMenu = [[NSMenu alloc] initWithTitle:@"insynic"];
    
    NSMenuItem *aboutItem = [[NSMenuItem alloc] initWithTitle:@"About insynic" action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
    [statusMenu addItem:aboutItem];
    
    [statusMenu addItem:[NSMenuItem separatorItem]];
    
    NSMenu *settingMenu = [[NSMenu alloc] initWithTitle:@"Setting"];
    NSMenu *languageMenu = [[NSMenu alloc] initWithTitle:@"Language"];
    
    NSMenuItem *englishItem = [[NSMenuItem alloc] initWithTitle:@"English" action:nil keyEquivalent:@""];
    [languageMenu addItem:englishItem];
    
    NSMenuItem *chineseItem = [[NSMenuItem alloc] initWithTitle:@"Chinese" action:nil keyEquivalent:@""];
    [languageMenu addItem:chineseItem];
    
    NSMenuItem *languageMenuItem = [[NSMenuItem alloc] initWithTitle:@"Language" action:nil keyEquivalent:@""];
    [languageMenuItem setSubmenu:languageMenu];
    [settingMenu addItem:languageMenuItem];
    
    NSMenuItem *settingMenuItem = [[NSMenuItem alloc] initWithTitle:@"Setting" action:nil keyEquivalent:@""];
    [settingMenuItem setSubmenu:settingMenu];
    [statusMenu addItem:settingMenuItem];
    
    [statusMenu addItem:[NSMenuItem separatorItem]];
    
    NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:@"Quit insynic" action:@selector(terminate:) keyEquivalent:@""];
    [statusMenu addItem:quitItem];
    
    [statusItem setMenu:statusMenu];
}

void removeStatusBarMenu() {
    if (statusItem) {
        NSStatusBar *statusBar = [NSStatusBar systemStatusBar];
        [statusBar removeStatusItem:statusItem];
        statusItem = nil;
    }
}

void recreateMacOSMenus() {
    removeStatusBarMenu();

    NSApplication *app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];
    [app activateIgnoringOtherApps:YES];

    [app setMainMenu:nil];
    setupNativeMenuBar();
    createStatusBarMenu();

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        ProcessSerialNumber psn = {0, kCurrentProcess};
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
        SetFrontProcess(&psn);

        [app activateIgnoringOtherApps:YES];
    });
}
