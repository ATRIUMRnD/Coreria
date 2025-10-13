#include <windows.h>
#include <stdio.h>

// Test ragdoll-specific functions
typedef struct {
    unsigned int id;
} RagdollHandle;

typedef int (*SpawnRagdollFunc)(unsigned int, int, float, float, float);
typedef int (*ApplyLimbForceFunc)(RagdollHandle, int, float, float, float, float, float, float);
typedef int (*GetRagdollPositionFunc)(RagdollHandle, float*, float*, float*, float*);
typedef int (*DestroyRagdollFunc)(RagdollHandle);
typedef int (*GetRagdollCountFunc)();

int main() {
    HMODULE hModule = LoadLibrary("D:\\Coreria\\chaosforge-multiplayer\\target\\release\\chaosforge_multiplayer.dll");
    
    if (!hModule) {
        printf("Failed to load DLL. Error: %lu\n", GetLastError());
        return 1;
    }
    
    printf("DLL loaded successfully!\n");
    
    // Test ragdoll function names
    const char* ragdoll_functions[] = {
        "coreria_spawn_ragdoll",
        "coreria_apply_limb_force", 
        "coreria_get_ragdoll_position",
        "coreria_destroy_ragdoll",
        "coreria_get_ragdoll_count"
    };
    
    printf("\n=== Testing Ragdoll Functions ===\n");
    int found_count = 0;
    
    for (int i = 0; i < sizeof(ragdoll_functions)/sizeof(ragdoll_functions[0]); i++) {
        FARPROC addr = GetProcAddress(hModule, ragdoll_functions[i]);
        printf("%-30s: %s\n", ragdoll_functions[i], addr ? "FOUND" : "NOT FOUND");
        if (addr) found_count++;
    }
    
    printf("\nFound %d out of %d ragdoll functions.\n", found_count, 5);
    
    // Test basic engine functions for comparison
    printf("\n=== Testing Engine Functions ===\n");
    const char* engine_functions[] = {
        "coreria_init_engine",
        "coreria_shutdown_engine",
        "coreria_step_simulation"
    };
    
    for (int i = 0; i < sizeof(engine_functions)/sizeof(engine_functions[0]); i++) {
        FARPROC addr = GetProcAddress(hModule, engine_functions[i]);
        printf("%-30s: %s\n", engine_functions[i], addr ? "FOUND" : "NOT FOUND");
    }
    
    if (found_count > 0) {
        printf("\n=== Testing Basic Ragdoll Workflow ===\n");
        
        // Get function pointers for the ones that exist
        SpawnRagdollFunc spawn_ragdoll = (SpawnRagdollFunc)GetProcAddress(hModule, "coreria_spawn_ragdoll");
        GetRagdollCountFunc get_ragdoll_count = (GetRagdollCountFunc)GetProcAddress(hModule, "coreria_get_ragdoll_count");
        
        if (spawn_ragdoll && get_ragdoll_count) {
            // Initialize engine first
            typedef int (*InitEngineFunc)();
            InitEngineFunc init_engine = (InitEngineFunc)GetProcAddress(hModule, "coreria_init_engine");
            
            if (init_engine) {
                printf("1. Initializing engine...\n");
                int result = init_engine();
                printf("   Engine init result: %d\n", result);
                
                if (result == 0) {
                    printf("2. Getting initial ragdoll count...\n");
                    int initial_count = get_ragdoll_count();
                    printf("   Initial ragdoll count: %d\n", initial_count);
                    
                    printf("3. Spawning ragdoll...\n");
                    RagdollHandle handle;
                    // spawn_ragdoll returns RagdollHandle, but we're treating it as int for simplicity
                    int spawn_result = spawn_ragdoll(1, 0, 0.0f, 1.0f, 0.0f);  // player_id=1, style=Brawler, pos=(0,1,0)
                    printf("   Spawn result: %d\n", spawn_result);
                    
                    printf("4. Getting new ragdoll count...\n");
                    int new_count = get_ragdoll_count();
                    printf("   New ragdoll count: %d\n", new_count);
                } else {
                    printf("Engine initialization failed, skipping ragdoll tests.\n");
                }
            } else {
                printf("Engine init function not found.\n");
            }
        } else {
            printf("Required ragdoll functions not found for testing.\n");
        }
    }
    
    FreeLibrary(hModule);
    return 0;
}