#!/bin/bash

echo "🚀 Quick Kubernetes Setup for Linux/WSL"
echo "========================================"

# Check what we have
echo "Checking current tools..."
echo "Docker: $(which docker 2>/dev/null || echo 'Not found')"
echo "kubectl: $(which kubectl 2>/dev/null || echo 'Not found')"
echo "kind: $(which kind 2>/dev/null || echo 'Not found')"
echo ""

# Install kubectl if missing
if ! command -v kubectl &> /dev/null; then
    echo "Installing kubectl..."
    curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl"
    chmod +x kubectl
    sudo mv kubectl /usr/local/bin/ 2>/dev/null || mv kubectl ~/bin/ 2>/dev/null || echo "Please move kubectl to your PATH manually"
fi

# Install kind if missing
if ! command -v kind &> /dev/null; then
    echo "Installing kind..."
    curl -Lo ./kind https://kind.sigs.k8s.io/dl/v0.20.0/kind-linux-amd64
    chmod +x ./kind
    sudo mv ./kind /usr/local/bin/ 2>/dev/null || mv ./kind ~/bin/ 2>/dev/null || echo "Please move kind to your PATH manually"
fi

# Check Docker
if ! command -v docker &> /dev/null; then
    echo "Docker not found. Installing Docker..."
    curl -fsSL https://get.docker.com -o get-docker.sh
    sudo sh get-docker.sh
    sudo usermod -aG docker $USER
    echo "Docker installed. You may need to restart your session."
else
    echo "Docker found: $(docker --version)"
fi

# Test Docker
if docker info >/dev/null 2>&1; then
    echo "✅ Docker is running"
else
    echo "⚠️  Docker is not running. Starting Docker..."
    sudo systemctl start docker 2>/dev/null || sudo service docker start 2>/dev/null || echo "Please start Docker manually"
fi

echo ""
echo "🎯 Creating Kubernetes cluster with kind..."
kind create cluster --name dev-cluster

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Kubernetes cluster created successfully!"
    echo ""
    echo "Testing cluster..."
    kubectl cluster-info
    echo ""
    kubectl get nodes
    echo ""
    echo "🎉 Kubernetes is ready!"
    echo ""
    echo "Try these commands:"
    echo "  kubectl get nodes"
    echo "  kubectl get pods -A"
    echo "  kubectl create deployment nginx --image=nginx"
else
    echo "❌ Failed to create cluster. Check Docker is running."
fi
