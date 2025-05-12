# Description

Kung Fu Panda is a general-purpose request recording and playback tool.

# Services

## KfPandaService

### Record

### Replay

## Debugervice

### Echo

## ApiService

```shell
curl 127.0.0.1:9820/api/echo -d "hello world"
```

# RPC Protocols

## Request Recording

```protobuf
message RecordRequest {
  string request_id = 1;
  // reqeust timestamp in millisecond
  // default: current timestamp if not set
  int64 timestamp_ms = 2;
  URI uri = 3;

  string service = 100;
  RecordType type = 101;
  bytes data = 102;
}

message RecordResponse {
  // code: indicte the processing result
  // 0: ok, failed otherwise
  int32 code = 1;
  string message = 2;
}

service KfPandaService {
  rpc Record(RecordRequest) returns (RecordResponse) {}
  ...
}
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
- [Web UI](https://github.com/Sunflowerjing/kfpanda-admin)

# References

- [rocksdb](https://github.com/facebook/rocksdb)
- [brpc](https://github.com/apache/brpc)
