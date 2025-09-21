#!/bin/bash

# Nerd Font icons
ICON_LOGOUT="󰍃"
#ICON_LOGOUT="󰩈"
ICON_POWEROFF="󰐥"
ICON_REBOOT="󰜉"
ICON_LOCK=""
ICON_YES="󰄬"
ICON_NO="󰅖"
ICON_SLEEP="󰤄"

# DMENU="rofi -dmenu -i -matching fuzzy"
DMENU="rofi -dmenu -i -matching fuzzy -theme ~/.config/rofi/styles/power-8.rasi"

# Confirmation prompt
confirm() {
    printf "${ICON_YES}\n${ICON_NO}\n" | $DMENU -p "Confirm Action"
}

while true; do
    # Main menu
    option="$(printf "${ICON_LOCK}\n${ICON_SLEEP}\n${ICON_LOGOUT}\n${ICON_POWEROFF}\n${ICON_REBOOT}\n" | $DMENU -p "Power Menu")"
    
    case "$option" in
        "${ICON_LOCK}")
            slock
            break
            ;;
        "${ICON_SLEEP}")
            systemctl suspend && slock
            break
            ;;
        "${ICON_LOGOUT}")
            response=$(confirm)
            [ "$response" = "${ICON_YES}" ] && loginctl terminate-user "$USER" && break
            ;;
        "${ICON_POWEROFF}")
            response=$(confirm)
            [ "$response" = "${ICON_YES}" ] && systemctl poweroff && break
            ;;
        "${ICON_REBOOT}")
            response=$(confirm)
            [ "$response" = "${ICON_YES}" ] && systemctl reboot && break
            ;;
        *)
            # User exited or pressed Esc — exit silently
            break
            ;;
    esac
done
