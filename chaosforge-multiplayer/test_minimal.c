#include <stdio.h>
#include <windows.h>

// Simple test that loads the DLL dynamically
int main() {
    printf("Loading ChaosForge Multiplayer DLL...\n");
    
    HMODULE dll = LoadLibrary(".\\target\\release\\chaosforge_multiplayer.dll");
    if (!dll) {
        printf("Failed to load DLL. Error code: %lu\n", GetLastError());
        return 1;
    }
    
    printf("DLL loaded successfully!\n");
    
    // Try to get a function pointer to our initialize function
    typedef int (*InitFunc)(void);
    InitFunc initialize_engine = (InitFunc)GetProcAddress(dll, "coreria_initialize_engine");
    
    if (initialize_engine) {
        printf("Found coreria_initialize_engine function!\n");
        
        // Try calling it
        int result = initialize_engine();
        printf("Initialize result: %d\n", result);
    } else {
        printf("coreria_initialize_engine function not found.\n");
        printf("Error code: %lu\n", GetLastError());
        
        // Let's see what functions are exported
        printf("DLL loaded but function not found. This suggests the DLL is valid but function names may be different.\n");
    }
    
    FreeLibrary(dll);
    printf("Test completed.\n");
    return 0;
}