#include <windows.h>
#include <stdio.h>

// Test the actual working functions
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
    
    // Get the correct function names
    InitEngineFunc init_func = (InitEngineFunc)GetProcAddress(hModule, "coreria_init_engine");
    StepSimulationFunc step_func = (StepSimulationFunc)GetProcAddress(hModule, "coreria_step_simulation");
    ShutdownEngineFunc shutdown_func = (ShutdownEngineFunc)GetProcAddress(hModule, "coreria_shutdown_engine");
    
    printf("init_func: %p\n", init_func);
    printf("step_func: %p\n", step_func);
    printf("shutdown_func: %p\n", shutdown_func);
    
    if (init_func) {
        printf("\n=== Testing Full FFI Workflow ===\n");
        
        printf("1. Initializing engine...\n");
        int result = init_func();
        printf("   init_engine returned: %d\n", result);
        
        if (result == 0) {
            printf("2. Running simulation step...\n");
            result = step_func(0.016f);
            printf("   step_simulation returned: %d\n", result);
            
            printf("3. Shutting down engine...\n");
            result = shutdown_func();
            printf("   shutdown_engine returned: %d\n", result);
        } else {
            printf("   Engine initialization failed, skipping other tests.\n");
        }
    } else {
        printf("init_func not found!\n");
    }
    
    printf("\n=== FFI Integration Test Complete ===\n");
    FreeLibrary(hModule);
    return 0;
}