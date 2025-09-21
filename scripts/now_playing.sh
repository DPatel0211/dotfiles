#!/bin/bash

# Create unique temporary files based on process ID
PID=$$
COVER_DIR="/tmp/mpd-covers"
COVER_FILE="$COVER_DIR/current-$PID.jpg"
LOCK_FILE="/tmp/mpd-notify.lock"
MUSIC_DIR="$HOME/Music/SoundCloud/likes/audio"

# Create cover directory
mkdir -p "$COVER_DIR"

# Simple lock mechanism to prevent multiple notifications
if [[ -f "$LOCK_FILE" ]]; then
    # If lock is older than 2 seconds, remove it (stale lock)
    if [[ $(($(date +%s) - $(stat -c %Y "$LOCK_FILE" 2>/dev/null || echo 0))) -gt 2 ]]; then
        rm -f "$LOCK_FILE"
    else
        # Lock exists and is recent, exit to avoid spam
        exit 0
    fi
fi

# Create lock file
echo $$ > "$LOCK_FILE"

# Cleanup function
cleanup() {
    rm -f "$COVER_FILE" "$LOCK_FILE"
}
trap cleanup EXIT

# Get current song info and playback state
SONG=$(mpc --format "%title%" current)
ARTIST=$(mpc --format "%artist%" current)
FILE=$(mpc --format "%file%" current)
STATE=$(mpc status | head -2 | tail -1 | grep -o "\[playing\]")

# If no song is playing OR if mpd is not in playing state, exit
if [[ -z "$SONG" ]] || [[ -z "$STATE" ]]; then
    exit 0
fi

# Full path to current file
FULL_PATH="$MUSIC_DIR/$FILE"

# Extract embedded thumbnail
if [[ -f "$FULL_PATH" ]]; then
    # Extract embedded artwork using ffmpeg
    ffmpeg -i "$FULL_PATH" -an -vcodec copy "$COVER_FILE" 2>/dev/null
fi

# Create notification text - just title and artist
NOTIFICATION_TEXT="$SONG"
if [[ -n "$ARTIST" ]]; then
    NOTIFICATION_TEXT="$ARTIST - $SONG"
fi

# Send notification with or without image
if [[ -f "$COVER_FILE" ]]; then
    dunstify -i "$COVER_FILE" "Now Playing" "$NOTIFICATION_TEXT" -t 5000 -r 9999
else
    dunstify "Now Playing" "$NOTIFICATION_TEXT" -t 5000 -r 9999
fi

# Small delay to let notification show
sleep 0.5
