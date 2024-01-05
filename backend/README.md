# Chcount - Backend

This is chcount server which provides API for sending the texts for counting.
Server uses HTTP and WebSocket protocols for communicating.

## Install dependencies

To install dependencies look [here](../README.md#install-dependencies)

## How to build

```bash
cd path/to/chcount_project/backend
mkdir build
cd build
cmake ..
make -j
```

## Usage

```
chcount_server -D path/to/frontend/folder --chcount-executable path/to/chcount/cli/executable
```

All options

```bash
Options:
  --help                         Help message
  -H [ --host ] arg (=127.0.0.1) Host on which server listens
  -P [ --port ] arg (=3000)      Port on which server listens
  -D [ --docs ] arg              Served documents location directory
  -T [ --tmp-storage ] arg (=.)  Temporary storage directory
  --chcount-executable arg       Chcount executable path
```

## API

### HTTP

- `GET` `/` <br>

  Servers the static files.

- `POST` `/api/count` <br>

  Only accepts `application/json` content type.

  Body must consist of `id` and `data`.<br>
  `id` - Session/User ID which the user will get from the WebSocket connection<br>
  `data` - Text for counting the occurencies of character 'I'. Data is limited to 20Kb.<br>

  Response:

  ```json
  {
    "request_id": "..." // Unique requst identifier
  }
  ```

### WebSocket

Server is listening for connection on '/'.<br>

All messages sent by the server have format:

```json
{
  "type": "...",
  "data": "..."
}
```

Available types are:

- `id` - ID message type
- `result` - Result of the requested counting

Possible data formats:

- For `id` type `data` field is a string which contains session id which user must provide on each
  counting request
- For `result` type `data` field is a object with format<br>
  ```json
  {
    "request_id": "...", // Request ID
    "result": "..." // Result of counting
  }
  ```

### How to use API

- Connect to the WebSocket
- After connection is established you will get message with `id` type which contains your session id
- Make calls to `/api/count` to request counting
