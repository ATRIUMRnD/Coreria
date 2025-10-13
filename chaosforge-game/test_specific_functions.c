#include <windows.h>
#include <stdio.h>

// Test specific functions
typedef int (*InitEngineFunc)();
typedef int (*ShutdownEngineFunc)();
typedef int (*StepSimulationFunc)(float);

int main() {
    HMODULE hModule = LoadLibrary("D:\\Coreria\\chaosforge-multiplayer\\target\\release\\chaosforge_multiplayer.dll");
    
    if (!hModule) {
        printf("Failed to load DLL. Error: %lu\n", GetLastError());
        return 1;
    }
    
    printf("DLL loaded successfully!\n");
    
    // Test the functions we know exist
    ShutdownEngineFunc shutdown_func = (ShutdownEngineFunc)GetProcAddress(hModule, "coreria_shutdown_engine");
    StepSimulationFunc step_func = (StepSimulationFunc)GetProcAddress(hModule, "coreria_step_simulation");
    
    // Test the function that doesn't exist
    InitEngineFunc init_func = (InitEngineFunc)GetProcAddress(hModule, "coreria_initialize_engine");
    
    printf("shutdown_func: %p\n", shutdown_func);
    printf("step_func: %p\n", step_func);
    printf("init_func: %p\n", init_func);
    
    if (init_func) {
        printf("Calling init_func...\n");
        int result = init_func();
        printf("init_func returned: %d\n", result);
    } else {
        printf("init_func not found! Trying to run other functions first...\n");
        
        // Try running step simulation to see if it works
        if (step_func) {
            printf("Calling step_func(0.016)...\n");
            int result = step_func(0.016f);
            printf("step_func returned: %d\n", result);
        }
    }
    
    if (shutdown_func) {
        printf("Calling shutdown_func...\n");
        int result = shutdown_func();
        printf("shutdown_func returned: %d\n", result);
    }
    
    FreeLibrary(hModule);
    return 0;
}