#!/bin/bash
set -e

docker build -m 8G -f distribution/docker/Dockerfile.base  -t tinyphone_base  ./distribution/docker
#docker run -m 2G tinyphone:latest "C:\Build\release.ps1"
cat release.ps1 | docker run -v `pwd`:"C:\Code\tinyphone" -i tinyphone_base

# docker cp $(docker ps -a --last 1 -q):"C:\Code\tinyphone\tinyphone-installer\bin\Release\tinyphone_installer.msi" "tinyphone_installer.$(date +%s).msi"
# docker rmi $(docker ps -a --last 1 -q)
