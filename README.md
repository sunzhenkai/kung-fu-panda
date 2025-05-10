# Description

Kung Fu Panda is a general request playback tool.

# Services

## HttpService

### Echo

```shell
curl 127.0.0.1:9820/api/echo -d "hello world"
```

# Docker

## Build image

```shell
make image
```

## Create Container & Run

```shell
docker run --rm --name kfpanda -p 9820:9820 -it sunzhenkai/kfpanda:0.0.1
```

# Submodules

- [Protocols](https://github.com/sunzhenkai/kung-fu-panda-protocols)
- [Go SDK](https://github.com/sunzhenkai/kfpanda-go-sdk)

# References

- [rocksdb](https://github.com/facebook/rocksdb)
- [brpc](https://github.com/apache/brpc)
