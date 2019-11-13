## Build Docker Image

```
docker build  -f Dockerfile.base -m 2G -t dotnet-vs15:latest  .
docker build  -f Dockerfile.build -m 2G -t tinyphone  .
```

## Run Docker

```
docker run -m 2G -it tinyphone:latest  "C:\Build\release.ps1"
```
