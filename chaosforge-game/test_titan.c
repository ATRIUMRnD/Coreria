#include <stdio.h>
#include <string.h>

// Mock structs
typedef struct { float x, y, z; } Vector3;
typedef struct {
    float health;
    int lives;
    float meter;
    float charge_time;
    char* last_hit;
    void (*combo_extender1)(void* self, void* target);
    void (*guard_break)(void* self, void* target);
    void (*ultimate)(void* self, void* target);
    float mass;
    float gravity;
} FightingStyle;
typedef struct {
    FightingStyle style;
    Vector3 position;
    float stamina;
    int stun;
} Entity;

// Mock physics
void physics_manager_apply_impulse_aoe(Vector3 force, float radius) {
    printf("Physics: Apply impulse (%.0f, %.0f, %.0f) to entities within %.1fm\n",
           force.x, force.y, force.z, radius);
}

// Titan ultimate (from combat_system.c)
void titan_ultimate(void* self, void* target) {
    FightingStyle* s = (FightingStyle*)self;
    if (s->meter >= 100) {
        physics_manager_apply_impulse_aoe((Vector3){0, -300, 0}, 6.0f);
        ((Entity*)target)->health -= 100;
        s->meter = 0;
        printf("Target hit, Health: %.0f, Lives: %d\n", ((Entity*)target)->health, ((Entity*)target)->lives);
    }
}

FightingStyle create_titan_style() {
    FightingStyle s = {0};
    s.ultimate = titan_ultimate;
    s.health = 100; s.lives = 2; s.meter = 100; s.mass = 2.0; s.gravity = 1.0;
    return s;
}

// Test harness
int main() {
    Entity titan = { .style = create_titan_style(), .position = {0, 0, 0}, .stamina = 100 };
    Entity target1 = { .style = { .health = 100, .lives = 2 }, .position = {5, 0, 0} };
    Entity target2 = { .style = { .health = 100, .lives = 2 }, .position = {3, 0, 2} };
    Entity target3 = { .style = { .health = 100, .lives = 2 }, .position = {4, 0, 1} };
    printf("Test: Titan Meteor Drop\n");
    printf("Titan at (%.0f, %.0f, %.0f), Targets at (5,0,0), (3,0,2), (4,0,1)\n",
           titan.position.x, titan.position.y, titan.position.z);
    titan.style.ultimate(&titan, &target1);  // AOE affects all targets
    target2.style.health -= 100;  // Simulate AOE
    target3.style.health -= 100;  // Simulate AOE
    target1.position.y -= 3; target2.position.y -= 3; target3.position.y -= 3;  // Simulate downward force
    printf("Post-Meteor Drop: Titan at (%.0f, %.0f, %.0f), Mass: %.1f\n",
           titan.position.x, titan.position.y, titan.position.z, titan.style.mass);
    printf("Target1 at (%.0f, %.0f, %.0f), Health: %.0f, Lives: %d\n",
           target1.position.x, target1.position.y, target1.position.z, target1.style.health, target1.style.lives);
    printf("Target2 at (%.0f, %.0f, %.0f), Health: %.0f, Lives: %d\n",
           target2.position.x, target2.position.y, target2.position.z, target2.style.health, target2.style.lives);
    printf("Target3 at (%.0f, %.0f, %.0f), Health: %.0f, Lives: %d\n",
           target3.position.x, target3.position.y, target3.position.z, target3.style.health, target3.style.lives);
    return 0;
}
