#!/bin/bash

# Kubernetes Installation Script for Linux
echo "🚀 Installing Kubernetes tools on Linux..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root
if [[ $EUID -eq 0 ]]; then
    print_warning "Running as root. This is fine for installation."
fi

# Detect Linux distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
    print_status "Detected OS: $OS $VER"
else
    print_error "Cannot detect Linux distribution"
    exit 1
fi

# Update package manager
print_status "Updating package manager..."
if command -v apt-get &> /dev/null; then
    apt-get update
elif command -v yum &> /dev/null; then
    yum update -y
elif command -v dnf &> /dev/null; then
    dnf update -y
fi

# Install required packages
print_status "Installing required packages..."
if command -v apt-get &> /dev/null; then
    apt-get install -y curl wget apt-transport-https ca-certificates gnupg lsb-release
elif command -v yum &> /dev/null; then
    yum install -y curl wget
elif command -v dnf &> /dev/null; then
    dnf install -y curl wget
fi

# Install Docker if not present
if ! command -v docker &> /dev/null; then
    print_status "Installing Docker..."
    curl -fsSL https://get.docker.com -o get-docker.sh
    sh get-docker.sh
    systemctl enable docker
    systemctl start docker
    usermod -aG docker $USER
    print_status "Docker installed. You may need to log out and back in for group changes to take effect."
else
    print_status "Docker is already installed"
    docker --version
fi

# Install kubectl if not present
if ! command -v kubectl &> /dev/null; then
    print_status "Installing kubectl..."
    curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl"
    chmod +x kubectl
    mv kubectl /usr/local/bin/
    print_status "kubectl installed"
else
    print_status "kubectl is already installed"
    kubectl version --client
fi

# Install Kind (Kubernetes in Docker)
if ! command -v kind &> /dev/null; then
    print_status "Installing Kind..."
    curl -Lo ./kind https://kind.sigs.k8s.io/dl/v0.20.0/kind-linux-amd64
    chmod +x ./kind
    mv ./kind /usr/local/bin/kind
    print_status "Kind installed"
else
    print_status "Kind is already installed"
    kind version
fi

print_status "✅ Installation complete!"
print_status "Next steps:"
echo -e "${BLUE}1.${NC} Create a Kind cluster: ${YELLOW}kind create cluster --name dev-cluster${NC}"
echo -e "${BLUE}2.${NC} Verify installation: ${YELLOW}kubectl cluster-info${NC}"
echo -e "${BLUE}3.${NC} Get nodes: ${YELLOW}kubectl get nodes${NC}"

if [[ $EUID -ne 0 ]]; then
    print_warning "If you installed Docker, you may need to log out and back in for group changes to take effect."
fi
