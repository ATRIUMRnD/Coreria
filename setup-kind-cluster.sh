#!/bin/bash

# Kind Cluster Setup and Verification Script
echo "🎯 Setting up Kind Kubernetes cluster..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if Docker is running
print_status "Checking Docker status..."
if ! docker info >/dev/null 2>&1; then
    print_error "Docker is not running. Starting Docker..."
    systemctl start docker
    sleep 5
    if ! docker info >/dev/null 2>&1; then
        print_error "Failed to start Docker. Please start Docker manually."
        exit 1
    fi
fi
print_status "✅ Docker is running"

# Check if kind is installed
if ! command -v kind &> /dev/null; then
    print_error "Kind is not installed. Please run install-kubernetes-linux.sh first."
    exit 1
fi

# Check if kubectl is installed
if ! command -v kubectl &> /dev/null; then
    print_error "kubectl is not installed. Please run install-kubernetes-linux.sh first."
    exit 1
fi

# Create Kind cluster
CLUSTER_NAME="dev-cluster"
print_status "Creating Kind cluster: $CLUSTER_NAME"

if kind get clusters | grep -q "$CLUSTER_NAME"; then
    print_warning "Cluster $CLUSTER_NAME already exists. Deleting and recreating..."
    kind delete cluster --name "$CLUSTER_NAME"
fi

# Create cluster with custom configuration
cat <<EOF > kind-config.yaml
kind: Cluster
apiVersion: kind.x-k8s.io/v1alpha4
nodes:
- role: control-plane
  kubeadmConfigPatches:
  - |
    kind: InitConfiguration
    nodeRegistration:
      kubeletExtraArgs:
        node-labels: "ingress-ready=true"
  extraPortMappings:
  - containerPort: 80
    hostPort: 80
    protocol: TCP
  - containerPort: 443
    hostPort: 443
    protocol: TCP
EOF

kind create cluster --name "$CLUSTER_NAME" --config kind-config.yaml

if [ $? -eq 0 ]; then
    print_status "✅ Kind cluster created successfully!"
    
    # Set kubectl context
    kubectl cluster-info --context "kind-$CLUSTER_NAME"
    
    print_status "Verifying cluster..."
    echo -e "${BLUE}Cluster Info:${NC}"
    kubectl cluster-info
    
    echo -e "\n${BLUE}Nodes:${NC}"
    kubectl get nodes -o wide
    
    echo -e "\n${BLUE}System Pods:${NC}"
    kubectl get pods -n kube-system
    
    # Test with a simple deployment
    print_status "Testing with nginx deployment..."
    kubectl create deployment nginx-test --image=nginx:latest
    kubectl wait --for=condition=available --timeout=60s deployment/nginx-test
    
    echo -e "\n${BLUE}Test Deployment:${NC}"
    kubectl get deployments
    kubectl get pods
    
    # Expose the deployment
    kubectl expose deployment nginx-test --port=80 --type=NodePort
    
    # Get the service details
    echo -e "\n${BLUE}Services:${NC}"
    kubectl get services
    
    # Cleanup test deployment
    print_status "Cleaning up test deployment..."
    kubectl delete deployment nginx-test
    kubectl delete service nginx-test
    
    print_status "🎉 Kubernetes cluster is ready!"
    echo -e "${BLUE}Cluster name:${NC} $CLUSTER_NAME"
    echo -e "${BLUE}Context:${NC} kind-$CLUSTER_NAME"
    echo -e "${BLUE}Kubeconfig:${NC} ~/.kube/config"
    
    echo -e "\n${YELLOW}Useful commands:${NC}"
    echo -e "${BLUE}•${NC} View cluster info: ${YELLOW}kubectl cluster-info${NC}"
    echo -e "${BLUE}•${NC} Get nodes: ${YELLOW}kubectl get nodes${NC}"
    echo -e "${BLUE}•${NC} Get pods: ${YELLOW}kubectl get pods -A${NC}"
    echo -e "${BLUE}•${NC} Delete cluster: ${YELLOW}kind delete cluster --name $CLUSTER_NAME${NC}"
    
else
    print_error "Failed to create Kind cluster"
    exit 1
fi

# Clean up config file
rm -f kind-config.yaml
