# Kubernetes Setup Verification Script
Write-Host "Checking Kubernetes setup..." -ForegroundColor Green

# Check if kubectl is working
Write-Host "`nTesting kubectl connection..." -ForegroundColor Yellow
try {
    kubectl cluster-info
    Write-Host "✅ kubectl is connected to cluster" -ForegroundColor Green
} catch {
    Write-Host "❌ kubectl connection failed" -ForegroundColor Red
    Write-Host "Make sure Kubernetes is enabled in Docker Desktop" -ForegroundColor Yellow
}

# Check nodes
Write-Host "`nChecking cluster nodes..." -ForegroundColor Yellow
kubectl get nodes

# Check system pods
Write-Host "`nChecking system pods..." -ForegroundColor Yellow
kubectl get pods -n kube-system

# Test with a simple deployment
Write-Host "`nTesting with a simple nginx deployment..." -ForegroundColor Yellow
kubectl create deployment nginx-test --image=nginx:latest
kubectl wait --for=condition=available --timeout=60s deployment/nginx-test
kubectl get deployments
kubectl get pods

# Cleanup test deployment
Write-Host "`nCleaning up test deployment..." -ForegroundColor Yellow
kubectl delete deployment nginx-test

Write-Host "`n✅ Kubernetes setup verification complete!" -ForegroundColor Green
