#!/bin/bash
# ~/.local/bin/record_screen.sh - Intel GPU optimized version

OUTDIR="$HOME/Videos/screen_recordings"
FILENAME="$OUTDIR/rec_$(date +%Y-%m-%d_%H-%M-%S).mp4"

# Create output directory if it doesn't exist
mkdir -p "$OUTDIR"

if pgrep -x "ffmpeg" > /dev/null; then
    # Stop recording
    pkill -INT ffmpeg
    notify-send "Stopped Recording" "Saved as '$(basename "$FILENAME")'"
else
    # Start recording with Intel Quick Sync Video acceleration
    # Much lower CPU usage than software encoding
    ffmpeg -video_size 1920x1080 -framerate 60 -f x11grab -i :0.0 \
           -c:v h264_vaapi -vaapi_device /dev/dri/renderD128 \
           -vf 'format=nv12,hwupload' \
           -preset fast -crf 23 \
           "$FILENAME" &
    
    notify-send "Started Recording" "Hardware-accelerated recording (60 fps)"
fi
