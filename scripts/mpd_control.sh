#!/bin/bash
if systemctl --user is-enabled --quiet mpd; then
    systemctl --user stop mpd
    systemctl --user disable mpd
    notify-send "Music Player Daemon Service" "Stopped and disabled"
else
    systemctl --user enable mpd
    systemctl --user start mpd
    notify-send "Music Player Daemon Service" "Started and enabled"
fi
