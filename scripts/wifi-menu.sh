#!/usr/bin/env bash

DMENU="rofi -dmenu -i -theme ~/.config/rofi/styles/wifi.rasi"
DMENU_PASSWORD="rofi -dmenu -password -theme ~/.config/rofi/styles/wifi-password.rasi"

while true; do
    connected=$(nmcli -fields WIFI g)
    
    if [[ "$connected" =~ "enabled" ]]; then
        # Get a list of available wifi connections and morph it into a nice-looking list
        wifi_list=$(nmcli --fields "SECURITY,SSID" device wifi list | sed 1d | sed 's/  */ /g' | sed -E "s/WPA*.?\S/ /g" | sed "s/^--/ /g" | sed "s/  //g" | sed "/--/d")
        toggle="󰖪  Disable Wi-Fi"
        menu_options="$toggle\n$wifi_list"
    elif [[ "$connected" =~ "disabled" ]]; then
        toggle="󰖩  Enable Wi-Fi"
        menu_options="$toggle"
    fi

    # Use rofi to select wifi network
    chosen_network=$(echo -e "$menu_options" | uniq | $DMENU -selected-row 1 -p "Wi-Fi Menu")
    
    if [ "$chosen_network" = "" ]; then
        break
    elif [ "$chosen_network" = "󰖩  Enable Wi-Fi" ]; then
        nmcli radio wifi on
	sleep 5
	continue
    elif [ "$chosen_network" = "󰖪  Disable Wi-Fi" ]; then
        nmcli radio wifi off
        continue
    else
        # Get name of connection
        chosen_id=$(echo "$chosen_network" | sed 's/^[[:space:]]*[^[:alnum:]]*[[:space:]]*//' | sed 's/[[:space:]]*$//')
        
        # Get saved connections
        saved_connections=$(nmcli -g NAME connection)
        if [[ $(echo "$saved_connections" | grep -w "$chosen_id") = "$chosen_id" ]]; then
            nmcli connection up id "$chosen_id" | grep "successfully" && break
        else
            if [[ "$chosen_network" =~ "" ]]; then
                wifi_password=$($DMENU_PASSWORD -p "Enter Password")
                if [ "$wifi_password" = "" ]; then
                    # User pressed escape or clicked back button, continue to main menu
                    continue
                fi
                nmcli device wifi connect "$chosen_id" password "$wifi_password" | grep "successfully" && break
            fi
        fi
    fi
done
