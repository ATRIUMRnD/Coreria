# Install Kind (Kubernetes in Docker) - No admin privileges required
Write-Host "Installing Kind (Kubernetes in Docker)..." -ForegroundColor Green

# Create tools directory if it doesn't exist
$toolsDir = "$env:USERPROFILE\tools"
if (!(Test-Path $toolsDir)) {
    New-Item -ItemType Directory -Path $toolsDir -Force
    Write-Host "Created tools directory: $toolsDir" -ForegroundColor Yellow
}

# Download Kind
$kindUrl = "https://kind.sigs.k8s.io/dl/v0.20.0/kind-windows-amd64"
$kindPath = "$toolsDir\kind.exe"

Write-Host "Downloading Kind..." -ForegroundColor Yellow
try {
    Invoke-WebRequest -Uri $kindUrl -OutFile $kindPath
    Write-Host "✅ Kind downloaded successfully" -ForegroundColor Green
} catch {
    Write-Host "❌ Failed to download Kind: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Add tools directory to PATH for current session
$env:PATH = "$toolsDir;$env:PATH"

# Check if Docker is running
Write-Host "Checking if Docker is running..." -ForegroundColor Yellow
try {
    docker info | Out-Null
    Write-Host "✅ Docker is running" -ForegroundColor Green
} catch {
    Write-Host "❌ Docker is not running. Please start Docker Desktop first." -ForegroundColor Red
    Write-Host "After starting Docker Desktop, run this script again." -ForegroundColor Yellow
    exit 1
}

# Create Kind cluster
Write-Host "Creating Kind cluster..." -ForegroundColor Yellow
& $kindPath create cluster --name dev-cluster

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Kind cluster created successfully!" -ForegroundColor Green
    
    # Verify the setup
    Write-Host "Verifying cluster..." -ForegroundColor Yellow
    kubectl cluster-info --context kind-dev-cluster
    kubectl get nodes
    
    Write-Host "`n🎉 Kubernetes is now ready!" -ForegroundColor Green
    Write-Host "Cluster name: dev-cluster" -ForegroundColor Cyan
    Write-Host "Context: kind-dev-cluster" -ForegroundColor Cyan
    Write-Host "`nTo use this cluster, make sure to add $toolsDir to your PATH permanently." -ForegroundColor Yellow
} else {
    Write-Host "❌ Failed to create Kind cluster" -ForegroundColor Red
}
