## Build Docker Image

```
docker build  -f Dockerfile.base -m 2G -t dotnet3.5-vs17:latest  .
docker build  -f Dockerfile.build -m 2G -t tinyphone  .
```

## Run Docker

```
./release.sh
```
