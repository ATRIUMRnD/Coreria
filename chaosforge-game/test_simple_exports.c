#include <windows.h>
#include <stdio.h>

int main() {
    HMODULE hModule = LoadLibrary("D:\\Coreria\\chaosforge-multiplayer\\target\\release\\chaosforge_multiplayer.dll");
    
    if (!hModule) {
        printf("Failed to load DLL. Error: %lu\n", GetLastError());
        return 1;
    }
    
    printf("DLL loaded successfully!\n");
    
    // Try a different approach - look for any coreria functions
    const char* test_names[] = {
        "coreria_initialize_engine",
        "coreria_init_engine",           // <- Alternative name found in mod.rs!
        "coreria_shutdown_engine", 
        "coreria_step_simulation",
        "coreria_init_server",
        "coreria_init_client", 
        "coreria_get_performance_metrics",
        "initialize_engine",
        "shutdown_engine",
        "step_simulation"
    };
    
    printf("\nTesting function names:\n");
    for (int i = 0; i < sizeof(test_names)/sizeof(test_names[0]); i++) {
        FARPROC addr = GetProcAddress(hModule, test_names[i]);
        printf("%-35s: %s\n", test_names[i], addr ? "FOUND" : "NOT FOUND");
    }
    
    FreeLibrary(hModule);
    return 0;
}