## Build Docker Image

```
docker build  -f Dockerfile.base -m 2G -t tinyphone_base  .
```

## Run Docker Build

```
sed -i 's/$env:CodeDir/C:\\Code\\tinyphone/g' ./distribution/docker/release.ps1
cat ./distribution/docker/release.ps1 | docker run -v `pwd`:"C:\Code\tinyphone" -i tinyphone_base
```
