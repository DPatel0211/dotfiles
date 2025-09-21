#!/bin/bash
#wifi() { nmcli radio wifi | grep -q disabled && echo OFF || ssid=$(nmcli -t -f active,ssid dev wifi | grep '^yes:' | cut -d: -f2); [ -z "$ssid" ] && echo N/A || echo "$ssid"; }
wifi() {
    # Check if WiFi is off (Airplane mode)
    if ! nmcli radio wifi | grep -q enabled; then
        echo "󰤮  Off"
        return
    fi

    # Get SSID and signal strength
    ssid=$(nmcli -t -f active,ssid dev wifi | grep '^yes' | cut -d: -f2)
    strength=$(nmcli -t -f active,signal dev wifi | grep '^yes' | cut -d: -f2)

    if [ -z "$ssid" ]; then
        echo "󰤯  Disconnected"
        return
    fi

    # Choose icon based on signal strength
    if [ "$strength" -ge 80 ]; then
        icon="󰤨"  # Full bars
    elif [ "$strength" -ge 60 ]; then
        icon="󰤥"  # 3 bars
    elif [ "$strength" -ge 40 ]; then
        icon="󰤢"  # 2 bars
    elif [ "$strength" -ge 20 ]; then
        icon="󰤟"  # 1 bar
    else
        icon="󰤯"  # Empty / weak
    fi

    echo "$icon  $ssid"
}

battery() {
    # Try upower first (most reliable)
    if command -v upower >/dev/null 2>&1; then
        battery_info=$(upower -i $(upower -e | grep 'BAT') 2>/dev/null)
        if [ -n "$battery_info" ]; then
            status=$(echo "$battery_info" | grep -E "state" | awk '{print $2}')
            capacity=$(echo "$battery_info" | grep -E "percentage" | awk '{print $2}' | sed 's/%//')
            
            # Handle different upower status names
            case "$status" in
                "charging")
                    charging=true ;;
                "fully-charged"|"full")
                    # Battery protection may stop charging at 95-98%
                    # but still report as "fully-charged" - this is normal
                    charging=false ;;
                "discharging"|"not-charging"|"unknown")
                    charging=false ;;
                *)
                    charging=false ;;
            esac
        else
            # UPower failed, fallback to other methods
            battery_fallback
            return
        fi
    else
        # UPower not available, try alternatives
        battery_fallback
        return
    fi

    # Choose icon based on charging status and capacity
    if [ "$charging" = true ]; then
        icon="󰂄"  # Single charging icon for all charging states
    else
        # Not charging - show battery level
        if [ "$capacity" -ge 90 ]; then icon="󰁹"
        elif [ "$capacity" -ge 70 ]; then icon="󰂁"
        elif [ "$capacity" -ge 50 ]; then icon="󰁾"
        elif [ "$capacity" -ge 21 ]; then icon="󰁻"
        else icon="󰂃"  # Low battery
        fi
    fi

    echo "$icon ${capacity}%"
}

battery_fallback() {
    # Try acpi first
    if command -v acpi >/dev/null 2>&1; then
        acpi_output=$(acpi -b 2>/dev/null | head -1)
        if [ -n "$acpi_output" ]; then
            capacity=$(echo "$acpi_output" | grep -o '[0-9]\+%' | tr -d '%')
            
            if echo "$acpi_output" | grep -qi "charging"; then
                icon="󰂄"  # Single charging icon
            elif echo "$acpi_output" | grep -qi "full"; then
                icon="󰁹"  # Full
            else
                # Set icon based on capacity
                if [ "$capacity" -ge 75 ]; then icon="󰂁"
                elif [ "$capacity" -ge 50 ]; then icon="󰁾"
                elif [ "$capacity" -ge 25 ]; then icon="󰁻"
                else icon="󰂃"
                fi
            fi
            echo "$icon ${capacity}%"
            return
        fi
    fi
    
    # Final fallback to /sys (your original method)
    battery_path="/sys/class/power_supply/BAT0"
    if [ -f "$battery_path/status" ] && [ -f "$battery_path/capacity" ]; then
        status=$(cat "$battery_path/status")
        capacity=$(cat "$battery_path/capacity")
        
        case "$status" in
            "Charging")
                icon="󰂄" ;;  # Single charging icon
            "Full")
                icon="󰁹" ;;
            *)
                if [ "$capacity" -ge 75 ]; then icon="󰂁"
                elif [ "$capacity" -ge 50 ]; then icon="󰁾"
                elif [ "$capacity" -ge 25 ]; then icon="󰁻"
                else icon="󰂃"
                fi ;;
        esac
        echo "$icon ${capacity}%"
    else
        echo "󰂃  ${capacity}%"
    fi
}

while true; do
	if ! xset q &>/dev/null; then
        	exit 0
	fi
	BAT=$(battery)
	# BAT=$(cat /sys/class/power_supply/BAT0/capacity 2>/dev/null || echo "0")
    	WIFI=$(wifi)
    	MEM=$(free | awk 'NR==2{printf "%.0f", $3*100/$2}')
    	CPU=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{printf "%.0f", 100 - $1}')
    	DISK=$(df -h / | awk 'NR==2{gsub("%",""); print $5}')
    	day=$(date '+%A')
    	month=$(date '+%B')
    	date_num=$(date '+%d')
 	year=$(date '+%Y')
    	time=$(date '+%I:%M %P')
    
    	# Add ordinal suffix (st/nd/rd/th)
    	case $date_num in
        	01|21|31) suffix="st" ;;
        	02|22) suffix="nd" ;;
        	03|23) suffix="rd" ;;
        	*) suffix="th" ;;
    	esac
    
    	# Remove leading zero from date if present
    	date_num=$(echo $date_num | sed 's/^0//')
    
   	 DATE="$day, $month ${date_num}${suffix} $year - $time"
    
    	# Fixed width formatting
	CENTER=$(printf "%s" "$DATE")
    	RIGHT=$(printf "   %1s%% |   %2s%% |   %2s%% | %s | %s " \
		"$DISK" "$MEM" "$CPU" "$BAT" "$WIFI")
	STATUS="$CENTER;$RIGHT"
    
    	xsetroot -name "$STATUS"
    	sleep 1
done
