#include <windows.h>
#include <stdio.h>
#include <math.h>

// Ragdoll handle structure matching Rust
typedef struct {
    unsigned int id;
} RagdollHandle;

// Fighting style constants matching Rust enum
#define STYLE_BRAWLER 0
#define STYLE_STRIKER 1  
#define STYLE_PHANTOM 2
#define STYLE_TITAN   3

// Limb indices matching Rust enum
#define LIMB_HEAD      0
#define LIMB_TORSO     1
#define LIMB_LEFT_ARM  2
#define LIMB_RIGHT_ARM 3
#define LIMB_LEFT_LEG  4
#define LIMB_RIGHT_LEG 5

// Function pointer types for ragdoll API
typedef int (*InitEngineFunc)();
typedef int (*ShutdownEngineFunc)();
typedef int (*StepSimulationFunc)(float);
typedef RagdollHandle (*SpawnRagdollFunc)(unsigned int, int, float, float, float);
typedef int (*ApplyLimbForceFunc)(RagdollHandle, int, float, float, float, float, float, float);
typedef int (*GetRagdollPositionFunc)(RagdollHandle, float*, float*, float*);
typedef int (*DestroyRagdollFunc)(RagdollHandle);
typedef int (*GetRagdollCountFunc)();

// Global function pointers
static InitEngineFunc init_engine;
static ShutdownEngineFunc shutdown_engine;
static StepSimulationFunc step_simulation;
static SpawnRagdollFunc spawn_ragdoll;
static ApplyLimbForceFunc apply_limb_force;
static GetRagdollPositionFunc get_ragdoll_position;
static DestroyRagdollFunc destroy_ragdoll;
static GetRagdollCountFunc get_ragdoll_count;

// Combat move implementations
void striker_tornado_kick(RagdollHandle ragdoll) {
    printf("   🌪️  TORNADO KICK: Spinning arms with high angular velocity\n");
    
    // Spin both arms with high torque (tornado effect)
    apply_limb_force(ragdoll, LIMB_LEFT_ARM,  0, 0, 0,  0, 1000, 0);  // Left arm spin
    apply_limb_force(ragdoll, LIMB_RIGHT_ARM, 0, 0, 0,  0, -1000, 0); // Right arm counter-spin
    
    // Add upward force to torso for jumping effect
    apply_limb_force(ragdoll, LIMB_TORSO, 0, 500, 0, 0, 0, 0);
    
    step_simulation(0.016f); // Advance physics
}

void brawler_berserker_rage(RagdollHandle ragdoll) {
    printf("   🔥 BERSERKER RAGE: Massive forces on all limbs\n");
    
    // Apply heavy forces to all limbs (berserker effect)
    apply_limb_force(ragdoll, LIMB_HEAD,      100, 0, 0,    50, 0, 0);
    apply_limb_force(ragdoll, LIMB_TORSO,     200, 0, 0,   100, 0, 0);
    apply_limb_force(ragdoll, LIMB_LEFT_ARM,  300, 0, 100, 150, 0, 50);
    apply_limb_force(ragdoll, LIMB_RIGHT_ARM, 300, 0,-100, 150, 0,-50);
    apply_limb_force(ragdoll, LIMB_LEFT_LEG,  150, 0, 50,   75, 0, 25);
    apply_limb_force(ragdoll, LIMB_RIGHT_LEG, 150, 0,-50,   75, 0,-25);
    
    step_simulation(0.016f);
}

void titan_ground_slam(RagdollHandle ragdoll) {
    printf("   💥 GROUND SLAM: Devastating downward force\n");
    
    // Massive downward force on arms (ground slam)
    apply_limb_force(ragdoll, LIMB_LEFT_ARM,  0, -800, 0,  0, 0, 200);
    apply_limb_force(ragdoll, LIMB_RIGHT_ARM, 0, -800, 0,  0, 0,-200);
    
    // Heavy torso impact
    apply_limb_force(ragdoll, LIMB_TORSO, 0, -1200, 0, 0, 0, 0);
    
    step_simulation(0.016f);
}

void phantom_phase_step(RagdollHandle ragdoll) {
    printf("   👻 PHASE STEP: Light, ethereal movement\n");
    
    // Light, precise forces (phase effect)
    apply_limb_force(ragdoll, LIMB_LEFT_LEG,  50, 200, 0,   25, 100, 0);
    apply_limb_force(ragdoll, LIMB_RIGHT_LEG, 50, 200, 0,   25,-100, 0);
    
    // Subtle torso adjustment
    apply_limb_force(ragdoll, LIMB_TORSO, 0, 100, 0, 0, 0, 0);
    
    step_simulation(0.016f);
}

int load_ragdoll_functions(HMODULE hModule) {
    init_engine = (InitEngineFunc)GetProcAddress(hModule, "coreria_init_engine");
    shutdown_engine = (ShutdownEngineFunc)GetProcAddress(hModule, "coreria_shutdown_engine");
    step_simulation = (StepSimulationFunc)GetProcAddress(hModule, "coreria_step_simulation");
    spawn_ragdoll = (SpawnRagdollFunc)GetProcAddress(hModule, "coreria_spawn_ragdoll");
    apply_limb_force = (ApplyLimbForceFunc)GetProcAddress(hModule, "coreria_apply_limb_force");
    get_ragdoll_position = (GetRagdollPositionFunc)GetProcAddress(hModule, "coreria_get_ragdoll_position");
    destroy_ragdoll = (DestroyRagdollFunc)GetProcAddress(hModule, "coreria_destroy_ragdoll");
    get_ragdoll_count = (GetRagdollCountFunc)GetProcAddress(hModule, "coreria_get_ragdoll_count");
    
    return (init_engine && shutdown_engine && step_simulation && spawn_ragdoll && 
            apply_limb_force && get_ragdoll_position && destroy_ragdoll && get_ragdoll_count);
}

void demo_fighting_style(int style_id, const char* style_name, void (*special_move)(RagdollHandle)) {
    printf("\n🥊 === %s COMBAT DEMO ===\n", style_name);
    
    // Spawn ragdoll with specific style
    RagdollHandle ragdoll = spawn_ragdoll(style_id + 100, style_id, 
                                         style_id * 2.0f, 2.0f, 0.0f);
    
    if (ragdoll.id == 0) {
        printf("❌ Failed to spawn %s ragdoll\n", style_name);
        return;
    }
    
    printf("✅ Spawned %s ragdoll (Handle ID: %u)\n", style_name, ragdoll.id);
    
    // Get initial position
    float x, y, z;
    if (get_ragdoll_position(ragdoll, &x, &y, &z) == 0) {
        printf("📍 Initial position: (%.2f, %.2f, %.2f)\n", x, y, z);
    }
    
    // Perform special move
    printf("⚡ Executing special ability:\n");
    special_move(ragdoll);
    
    // Get position after move
    if (get_ragdoll_position(ragdoll, &x, &y, &z) == 0) {
        printf("📍 Position after move: (%.2f, %.2f, %.2f)\n", x, y, z);
    }
    
    // Show ragdoll count
    int count = get_ragdoll_count();
    printf("🎯 Total ragdolls: %d\n", count);
    
    // Clean up
    destroy_ragdoll(ragdoll);
    printf("🗑️  Destroyed %s ragdoll\n", style_name);
}

int main() {
    printf("🚀 CHAOSFORGE MULTIPLAYER RAGDOLL COMBAT DEMO\n");
    printf("===============================================\n");
    
    // Load DLL
    HMODULE hModule = LoadLibrary("D:\\Coreria\\chaosforge-multiplayer\\target\\release\\chaosforge_multiplayer.dll");
    
    if (!hModule) {
        printf("❌ Failed to load DLL. Error: %lu\n", GetLastError());
        return 1;
    }
    
    printf("✅ DLL loaded successfully\n");
    
    // Load function pointers
    if (!load_ragdoll_functions(hModule)) {
        printf("❌ Failed to load required functions\n");
        FreeLibrary(hModule);
        return 1;
    }
    
    printf("✅ All ragdoll functions loaded\n");
    
    // Initialize engine
    printf("\n🔧 Initializing ChaosForge Multiplayer Engine...\n");
    int init_result = init_engine();
    if (init_result != 0) {
        printf("❌ Engine initialization failed: %d\n", init_result);
        FreeLibrary(hModule);
        return 1;
    }
    printf("✅ Engine initialized successfully\n");
    
    // Demo all fighting styles with their special moves
    demo_fighting_style(STYLE_BRAWLER, "BRAWLER", brawler_berserker_rage);
    demo_fighting_style(STYLE_STRIKER, "STRIKER", striker_tornado_kick);
    demo_fighting_style(STYLE_PHANTOM, "PHANTOM", phantom_phase_step);
    demo_fighting_style(STYLE_TITAN,   "TITAN",   titan_ground_slam);
    
    // Final stats
    printf("\n📊 === FINAL STATISTICS ===\n");
    int final_count = get_ragdoll_count();
    printf("Total ragdolls remaining: %d\n", final_count);
    
    // Shutdown
    printf("\n🔧 Shutting down engine...\n");
    shutdown_engine();
    
    FreeLibrary(hModule);
    
    printf("\n🎉 === DEMO COMPLETE ===\n");
    printf("Gang Beasts-style ragdoll physics integration successful!\n");
    
    return 0;
}