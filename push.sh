#!/bin/bash

RED='\033[0;31m'
GREEN='033[0;32m'
YELLOW='033[1;33m'
NC='\033[0m' 

echo "Starting PlatformIO build..."
export PATH=$PATH:$HOME/.platformio/penv/bin
pio run
if [ $? -ne 0 ]; then
    echo "Build failed. Exiting."
    exit 1
fi
echo "Build finished successfully."

# Read version from platformio.ini
major=$(grep '^major' controler_root/platformio.ini | awk -F '=' '{print $2}' | xargs)
minor=$(grep '^minor' controler_root/platformio.ini | awk -F '=' '{print $2}' | xargs)
patch=$(grep '^patch' controler_root/platformio.ini | awk -F '=' '{print $2}' | xargs)
version="v${major}.${minor}.${patch}"

# Get current branch
current_branch=$(git rev-parse --abbrev-ref HEAD)
echo "Current branch: $current_branch"

# Get last git tag
last_tag=$(git describe --tags --abbrev=0 2>/dev/null)

if [ "$current_branch" != "main" ]; then
    read -p "Enter commit message: " commit_msg
    git add .
    git commit -m "$commit_msg"
    git push
    echo "Done."
    exit 0
fi

# On main branch
if [ -z "$last_tag" ]; then
    echo -e "${RED}No tags found in the repository."
    read -p "Do you want to commit and push without a tag? (y/n): " answer
    echo -e "${NC}" 
    if [ "$answer" != "y" ]; then
        echo "Exiting."
        exit 0
    fi
elif [ "$last_tag" == "$version" ]; then
    echo "Version $version is already the latest tag."
    read -p "${RED}Do you want to commit and push without a new tag? (y/n): ${NC}" answer
    if [ "$answer" != "y" ]; then
        echo "Nothing to do. Exiting."
        exit 0
    fi
else
    echo "New version detected: $version (last tag: $last_tag)."
fi

# Ask for commit message
read -p "Enter commit message: " commit_msg

# Add all changes and commit
echo "Adding changes..."
git add .
echo "Committing changes..."
git commit -m "$commit_msg"

# If version is new, create tag and push
if [ "$current_branch" == "main" ] && [ "$last_tag" != "$version" ]; then
    echo "Creating git tag $version..."
    git tag "$version"
    git push origin "$version"
fi

# Push commit
echo "Pushing commits..."
git push

echo "Done."
