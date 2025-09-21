#!/bin/bash

# Screenshot script with notifications
# Usage: ./screenshot.sh [full|select]

send_notification() {
    local title="$1"
    local body="$2"
    notify-send -t 3000 "$title" "$body"
}

case "$1" in
    "full")
        # Full screenshot - immediate notification
        temp_file=$(mktemp --suffix=.png)
        if maim "$temp_file"; then
            if [[ -s "$temp_file" ]]; then
                xclip -selection clipboard -t image/png -i "$temp_file"
                send_notification "Screenshot" "Full screen copied to clipboard"
            else
                send_notification "Screenshot Error" "Captured image was empty"
            fi
        else
            send_notification "Screenshot Error" "Failed to capture full screen"
        fi
        rm -f "$temp_file"
        ;;

    "select")
        # Selection screenshot - notification AFTER selection is made
        send_notification "Screenshot" "Select area to capture..."

        temp_file=$(mktemp --suffix=.png)
        if maim -s "$temp_file"; then
            if [[ -s "$temp_file" ]]; then
                xclip -selection clipboard -t image/png -i "$temp_file"
                send_notification "Screenshot" "Selection copied to clipboard"
            else
                send_notification "Screenshot" "Selection cancelled"
            fi
        else
            send_notification "Screenshot" "Selection cancelled"
        fi
        rm -f "$temp_file"
        ;;

    *)
        echo "Usage: $0 [full|select]"
        exit 1
        ;;
esac
