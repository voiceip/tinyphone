#!/bin/bash

docker build -m 2G -f Dockerfile.build  -t tinyphone  .
docker run -m 2G tinyphone:latest "C:\Build\release.ps1"

docker cp $(docker ps -a --last 1 -q):

docker cp "bb25b1996aed:C:\Code\tinyphone\tinyphone-installer\bin\Debug\tinyphone_installer.msi" .
