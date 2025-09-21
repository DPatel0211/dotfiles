#!/bin/bash

# Volume control script for PulseAudio with dunst notifications
# Usage: ./volume.sh [up|down|mute|micmute]

STEP=5  # Volume step percentage
NOTIFICATION_ID=1234  # Unique ID for replacing notifications

get_volume() {
    pactl get-sink-volume @DEFAULT_SINK@ | grep -Po '\d+(?=%)' | head -1
}

get_mute_status() {
    pactl get-sink-mute @DEFAULT_SINK@ | grep -q "yes" && echo "MUTED" || echo "UNMUTED"
}

get_mic_mute_status() {
    pactl get-source-mute @DEFAULT_SOURCE@ | grep -q "yes" && echo "MUTED" || echo "UNMUTED"
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
        pactl set-sink-volume @DEFAULT_SINK@ +${STEP}%
        current_vol=$(get_volume)
        if [ $current_vol -gt 100 ]; then
            pactl set-sink-volume @DEFAULT_SINK@ 100%
            current_vol=100
        fi
        
        # Check if muted and show appropriate notification
        status=$(get_mute_status)
        if [ "$status" = "MUTED" ]; then
            send_notification "Volume Control" "MUTED (Volume: ${current_vol}%)"
        else
            progress=$(create_progress_bar $current_vol)
            send_notification "Volume Control" "$progress"
        fi
        ;;
    "down")
        pactl set-sink-volume @DEFAULT_SINK@ -${STEP}%
        current_vol=$(get_volume)
        
        # Check if muted and show appropriate notification
        status=$(get_mute_status)
        if [ "$status" = "MUTED" ]; then
            send_notification "Volume Control" "MUTED (Volume: ${current_vol}%)"
        else
            progress=$(create_progress_bar $current_vol)
            send_notification "Volume Control" "$progress"
        fi
        ;;
    "mute")
        pactl set-sink-mute @DEFAULT_SINK@ toggle
        status=$(get_mute_status)
        if [ "$status" = "MUTED" ]; then
            send_notification "Volume Control" "MUTED"
        else
            current_vol=$(get_volume)
            progress=$(create_progress_bar $current_vol)
            send_notification "Volume Control" "$progress"
        fi
        ;;
    "micmute")
        pactl set-source-mute @DEFAULT_SOURCE@ toggle
        status=$(get_mic_mute_status)
        send_notification "Microphone Control" "$status"
        ;;
    *)
        echo "Usage: $0 [up|down|mute|micmute]"
        exit 1
        ;;
esac
