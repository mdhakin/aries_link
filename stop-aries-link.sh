#!/usr/bin/env bash

PID=$(pgrep -x aries_link)

if [ -z "$PID" ]; then
    echo "Aries-Link is not running."
    exit 0
fi

echo "Stopping Aries-Link (PID: $PID)..."

kill -TERM "$PID"

if [ $? -eq 0 ]; then
    echo "SIGTERM sent successfully."
else
    echo "Failed to stop Aries-Link."
    exit 1
fi
