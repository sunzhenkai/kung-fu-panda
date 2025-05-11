# Description

Kung Fu Panda is a general request playback tool.

# Services

## HttpService

### Echo

```shell
curl 127.0.0.1:9820/api/echo -d "hello world"
```

# API

## `/api/replay`

Request Body

```json
{
  "requestId": "7a49c678-205e-4c83-884b-796d865c2964",
  "service": "KungFuPandaServer",
  "option": {
    "count": 1,
    "timeoutMs": 1000
  },
  "target": {
    "host": "127.0.0.1",
    "port": 9820
  }
}
```

Response Example

```shell
{
    "data": {
        "successCount": 1,
        "responses": [
            {
                "message": "CgtoZWxsbyB3b3JsZA==",
                "typeStr": "RECORD_TYPE_GRPC"
            }
        ]
    },
    "success": true,
    "code": 0,
    "message": ""
}
```

## `/api/debug/stat`

```shell
curl 127.0.0.1:9820/api/debug/stat
```

Response Example

```shell
{
    "data": {
        "KungFuPandaServer": "16"
    },
    "success": true,
    "code": 0,
    "message": ""
}
```

## `/api/debug/sample`

```shell
curl 127.0.0.1:9820/api/debug/sample -d '{"service":"KungFuPandaServer"}'
```

Response Example

```shell
{
    "data": {
        "1746965279497": {
            "uri": {
                "path": "/kfpanda.KfPandaDebugService/Echo"
            },
            "service": "KungFuPandaServer",
            "type": "RECORD_TYPE_GRPC",
            "data": "CgtoZWxsbyB3b3JsZA=="
        }
    },
    "success": true,
    "code": 0,
    "message": ""
}
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
- [C++ SDK](https://github.com/sunzhenkai/kfpanda-cpp-sdk)

# References

- [rocksdb](https://github.com/facebook/rocksdb)
- [brpc](https://github.com/apache/brpc)
