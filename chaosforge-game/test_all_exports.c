#include <windows.h>
#include <stdio.h>
#include <imagehlp.h>

#pragma comment(lib, "imagehlp.lib")

// Function to enumerate exports
BOOL CALLBACK EnumExportsCallback(PVOID pSymbolInfo, ULONG SymbolSize, PVOID UserContext) {
    PIMAGE_SYMBOL pSym = (PIMAGE_SYMBOL)pSymbolInfo;
    printf("Export: %s\n", (char*)UserContext + pSym->N.Name.Long);
    return TRUE;
}

int main() {
    HMODULE hModule = LoadLibrary("D:\\Coreria\\chaosforge-multiplayer\\target\\release\\chaosforge_multiplayer.dll");
    
    if (!hModule) {
        printf("Failed to load DLL. Error: %lu\n", GetLastError());
        return 1;
    }
    
    printf("DLL loaded successfully!\n");
    
    // Get module info
    MODULEINFO modInfo;
    if (GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo))) {
        printf("Module base: %p\n", modInfo.lpBaseOfDll);
        printf("Module size: %lu bytes\n", modInfo.SizeOfImage);
    }
    
    // Try a different approach - look for any coreria functions
    const char* test_names[] = {
        "coreria_initialize_engine",
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
        printf("%-30s: %s\n", test_names[i], addr ? "FOUND" : "NOT FOUND");
    }
    
    FreeLibrary(hModule);
    return 0;
}