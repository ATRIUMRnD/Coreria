#include <windows.h>
#include <stdio.h>

int main() {
    HMODULE hModule = LoadLibrary("target\\release\\chaosforge_multiplayer.dll");
    if (hModule == NULL) {
        printf("Failed to load DLL. Error code: %lu\n", GetLastError());
        return 1;
    }
    
    printf("DLL loaded successfully!\n");
    
    // Try to find some common function names
    const char* function_names[] = {
        "coreria_initialize_engine",
        "_coreria_initialize_engine",
        "coreria_initialize_engine@0",
        "_coreria_initialize_engine@0",
        "initialize_engine",
        "_initialize_engine",
        NULL
    };
    
    for (int i = 0; function_names[i] != NULL; i++) {
        FARPROC proc = GetProcAddress(hModule, function_names[i]);
        if (proc != NULL) {
            printf("Found function: %s at address %p\n", function_names[i], proc);
        } else {
            printf("Function not found: %s\n", function_names[i]);
        }
    }
    
    FreeLibrary(hModule);
    return 0;
}