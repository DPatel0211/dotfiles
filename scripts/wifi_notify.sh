#!/bin/bash

export DISPLAY=:0
export DBUS_SESSION_BUS_ADDRESS="unix:path=/run/user/$(id -u)/bus"

status_file="/tmp/.wifi_status_last"

# Determine WiFi state
if ! nmcli radio wifi | grep -q enabled; then
    wifi_status="Interface Off"
else
    ssid=$(nmcli -t -f active,ssid dev wifi | grep '^yes' | cut -d: -f2)

    if [ -n "$ssid" ]; then
        wifi_status="Connected to $ssid"
    else
        wifi_status="Disconnected"
    fi
fi

# Only notify if status has changed
if [ -f "$status_file" ]; then
    prev_status=$(<"$status_file")
else
    prev_status=""
fi

if [ "$wifi_status" != "$prev_status" ]; then
    echo "$wifi_status" > "$status_file"
    notify-send "WiFi Connection Status" "$wifi_status"
fi
