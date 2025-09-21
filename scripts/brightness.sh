#!/bin/bash

# Brightness control script with dunst notifications
# Usage: ./brightness.sh [up|down]

STEP=10  # Brightness step percentage
NOTIFICATION_ID=1235  # Unique ID for replacing notifications

get_brightness() {
    brightnessctl get
}

get_max_brightness() {
    brightnessctl max
}

get_brightness_percentage() {
    local current=$(get_brightness)
    local max=$(get_max_brightness)
    echo $((current * 100 / max))
}

create_progress_bar() {
    local current=$1
    local width=33
    local filled=$((current * width / 100))
    local empty=$((width - filled))
    
    local bar=""
    for ((i=0; i<filled; i++)); do bar+="█"; done
    for ((i=0; i<empty; i++)); do bar+=" "; done
    
    printf "[%s] %d%%" "$bar" "$current"
}

send_notification() {
    local title="$1"
    local body="$2"
    
    dunstify -r $NOTIFICATION_ID -t 2000 "$title" "$body"
}

case "$1" in
    "up")
        brightnessctl set +${STEP}%
        current_percent=$(get_brightness_percentage)
        progress=$(create_progress_bar $current_percent)
        send_notification "Brightness Control" "$progress"
        ;;
    "down")
        brightnessctl set ${STEP}%-
        current_percent=$(get_brightness_percentage)
        progress=$(create_progress_bar $current_percent)
        send_notification "Brightness Control" "$progress"
        ;;
    *)
        echo "Usage: $0 [up|down]"
        exit 1
        ;;
esac
