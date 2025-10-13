# PowerShell script to check DLL exports
$dll = "D:\Coreria\chaosforge-multiplayer\target\release\chaosforge_multiplayer.dll"

if (Test-Path $dll) {
    Write-Host "DLL exists: $dll"
    
    # Try to load the DLL and get export information
    Add-Type -TypeDefinition @"
        using System;
        using System.Runtime.InteropServices;
        
        public class DllExports {
            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern IntPtr LoadLibrary(string lpFileName);
            
            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);
            
            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern bool FreeLibrary(IntPtr hModule);
        }
"@
    
    $hModule = [DllExports]::LoadLibrary($dll)
    if ($hModule -ne [IntPtr]::Zero) {
        Write-Host "Successfully loaded DLL"
        
        # Test for our expected functions
        $functions = @(
            "coreria_initialize_engine",
            "coreria_initialize_engine_full",
            "_coreria_initialize_engine", 
            "coreria_shutdown_engine",
            "coreria_step_simulation"
        )
        
        foreach ($func in $functions) {
            $addr = [DllExports]::GetProcAddress($hModule, $func)
            if ($addr -ne [IntPtr]::Zero) {
                Write-Host "Found function: $func at address $addr"
            } else {
                Write-Host "Function not found: $func"
            }
        }
        
        [DllExports]::FreeLibrary($hModule) | Out-Null
    } else {
        Write-Host "Failed to load DLL"
    }
} else {
    Write-Host "DLL not found: $dll"
}