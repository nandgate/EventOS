#!/usr/bin/env bash
# Capture N seconds on channels 0-3 via Logic 2 MCP, save .sal + per-channel raw CSVs.
# Usage:  captures/capture.sh <name> <seconds>
set -euo pipefail

NAME="${1:?name required}"
SECONDS_ARG="${2:?seconds required}"
DIR="$(cd "$(dirname "$0")" && pwd)"
OUT_DIR="${DIR}/${NAME}"
MCP="http://127.0.0.1:10530/"

mkdir -p "$OUT_DIR"

call() {
  local resp
  resp=$(curl -sS -X POST "$MCP" -H "Content-Type: application/json" -d "$1")
  # Fail loudly if the tool call returned an error marker.
  if echo "$resp" | python3 -c "import sys,json; d=json.load(sys.stdin); c=d.get('result',{}).get('content',[]); sys.exit(0 if not d.get('result',{}).get('isError') else 1)" 2>/dev/null; then
    echo "$resp"
  else
    echo "MCP ERROR: $resp" >&2
    exit 1
  fi
}

START=$(call "$(cat <<JSON
{"jsonrpc":"2.0","id":1,"method":"tools/call","params":{
  "name":"start_capture","arguments":{
    "logicDeviceConfiguration":{
      "logicChannels":{"digitalChannels":[0,1,2,3,4]},
      "digitalSampleRate":10000000
    },
    "captureConfiguration":{
      "timedCaptureMode":{"durationSeconds":${SECONDS_ARG}}
    }
  }
}}
JSON
)")
CAPTURE_ID=$(echo "$START" | python3 -c 'import sys,json,re; d=json.load(sys.stdin); print(re.search(r"captureId\":(\d+)", d["result"]["content"][0]["text"]).group(1))')
echo "captureId=$CAPTURE_ID"

call "$(cat <<JSON
{"jsonrpc":"2.0","id":2,"method":"tools/call","params":{
  "name":"wait_capture","arguments":{"captureId":${CAPTURE_ID}}
}}
JSON
)" > /dev/null
echo "capture complete"

call "$(cat <<JSON
{"jsonrpc":"2.0","id":3,"method":"tools/call","params":{
  "name":"save_capture","arguments":{"captureId":${CAPTURE_ID},"filepath":"${OUT_DIR}/${NAME}.sal"}
}}
JSON
)" > /dev/null
echo "saved ${OUT_DIR}/${NAME}.sal"

call "$(cat <<JSON
{"jsonrpc":"2.0","id":4,"method":"tools/call","params":{
  "name":"export_raw_data_csv","arguments":{
    "captureId":${CAPTURE_ID},
    "directory":"${OUT_DIR}",
    "logicChannels":{"digitalChannels":[0,1,2,3,4]},
    "analogDownsampleRatio":1
  }
}}
JSON
)" > /dev/null
echo "exported CSVs to ${OUT_DIR}/"

call "$(cat <<JSON
{"jsonrpc":"2.0","id":5,"method":"tools/call","params":{
  "name":"close_capture","arguments":{"captureId":${CAPTURE_ID}}
}}
JSON
)" > /dev/null

ls -la "$OUT_DIR"
