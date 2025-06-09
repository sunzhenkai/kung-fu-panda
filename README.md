# Description

Kung Fu Panda is a general-purpose request recording and playback tool.

# Services

## KfPandaService

```protobuf
service KfPandaService {
  rpc Record(RecordRequest) returns (RecordResponse) {}
  rpc Replay(ReplayRequest) returns (ReplayResponse) {}
  rpc Sample(SampleRequest) returns (SampleResponse) {}
  rpc Log(LogRequest) returns (LogResponse) {}
}
```

## KfPandaDebugService

```protobuf
service KfPandaDebugService {
  rpc Echo(EchoMessage) returns (EchoMessage) {}
  rpc Replay(HttpRequest) returns (HttpResponse) {}
}
```

## KfPandaApiService

```protobuf
service KfPandaApiService {
  rpc Api(HttpRequest) returns (HttpResponse) {}
}
```

```shell
curl 127.0.0.1:9820/api/echo -d "hello world"
```

# RPC Protocols

## KfPandaService

### Record

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

### Playback

```protobuf
message ReplayRequest {
  message Option {
    int32 count = 1;
    int32 timeout_ms = 2;
  }

  string request_id = 1;
  string service = 2;
  Option option = 3;
  URI target = 100;
}

message ReplayResponse {
  message ServiceResponse {
    bytes body = 1;
    string message = 2;
    RecordType type = 3;
    string type_str = 4;
  }

  int32 code = 1;
  string message = 2;

  int32 success_count = 100;
  int32 failed_count = 101;
  repeated ServiceResponse responses = 102;
}

service KfPandaService {
  ...
  rpc Replay(ReplayRequest) returns (ReplayResponse) {}
}

```

### Log

```protobuf
message LogResponse {
  int32 code = 1;
  string message = 2;
}

enum LogLevel {
  LOG_LEVEL_UNSPECIFIED = 0;
  LOG_LEVEL_INFO = 1;
  LOG_LEVEL_WARN = 2;
  LOG_LEVEL_ERROR = 3;
  LOG_LEVEL_FATAL = 4;
  LOG_LEVEL_DEBUG = 5;
}

message LogRequest {
  string request_id = 1;
  int64 timestamp_ms = 2;
  string service = 3;
  string log_name = 4;
  LogLevel log_level = 5;
  string message = 6;

  string file = 50;
  int32 line = 51;
  string function = 52;

  string host = 100;
  string environment = 101;
  string region = 102;
  string cluster = 103;
  string version = 104;

  map<string, string> metadata = 200;
  string extra = 201;
}
```

# API

## Request Api

### `/api/replay`

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

### `/api/debug/stat`

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

### `/api/debug/sample`

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

## Log Api

### `/api/log/record`

Request Body

```json
{
  "request_id": "abcdef123456",
  "timestamp_ms": 1684000000000,
  "service": "user_service",
  "log_name": "user_login",
  "log_level": 1,
  "message": "User 'john.doe' logged in successfully.",
  "file": "auth.cc",
  "line": 123,
  "function": "HandleLogin",
  "host": "user-api-01",
  "environment": "production",
  "region": "us-central1",
  "cluster": "primary",
  "version": "1.2.3",
  "metadata": {
    "user_id": "789",
    "session_id": "xyz",
    "latency_ms": "50"
  },
  "extra": "Detailed debug information here."
}
```

Response

```json
{
  "success": true,
  "code": 0,
  "message": ""
}
```

### `/api/log/sample`

Api With Query

```shell
/api/log/sample?service=user_service&log_name=user_login&count=3
```

Response

```json
{
  "count": 1,
  "data": {
    "1684000000000": {
      "requestId": "abcdef123456",
      "timestampMs": "1684000000000",
      "service": "user_service",
      "logName": "user_login",
      "logLevel": "LOG_LEVEL_INFO",
      "message": "User 'john.doe' logged in successfully.",
      "file": "auth.cc",
      "line": 123,
      "function": "HandleLogin",
      "host": "user-api-01",
      "environment": "production",
      "region": "us-central1",
      "cluster": "primary",
      "version": "1.2.3",
      "metadata": {
        "user_id": "789",
        "session_id": "xyz",
        "latency_ms": "50"
      },
      "extra": "Detailed debug information here."
    }
  },
  "success": true,
  "code": 0,
  "message": ""
}
```

### `/api/log/stat`

```json
{
  "data": {
    "user_service::user_login": "1"
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
docker run --rm --name kfpanda -p 9820:9820 -it sunzhenkai/kfpanda:0.0.4
```

# Submodules

- [Protocols](https://github.com/sunzhenkai/kung-fu-panda-protocols)
- [Go SDK](https://github.com/sunzhenkai/kfpanda-go-sdk)
- [C++ SDK](https://github.com/sunzhenkai/kfpanda-cpp-sdk)
- [Tools](https://github.com/sunzhenkai/kfpanda-tools)
- [Web UI](https://github.com/Sunflowerjing/kfpanda-admin)

# References

- [rocksdb](https://github.com/facebook/rocksdb)
- [brpc](https://github.com/apache/brpc)
