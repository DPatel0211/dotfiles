#!/bin/bash

# SoundCloud Likes Downloader with Incremental Support
# Usage: ./download_likes.sh

# Configuration
DOWNLOAD_DIR="$HOME/Music/SoundCloud/likes"
ARCHIVE_FILE="$DOWNLOAD_DIR/.yt-dlp-archive"
YOUR_SOUNDCLOUD_URL="https://soundcloud.com/username/likes"

# Create download directories if they don't exist
mkdir -p "$DOWNLOAD_DIR/audio"
mkdir -p "$DOWNLOAD_DIR/metadata"

# Download with archive to avoid re-downloading
yt-dlp \
    --download-archive "$ARCHIVE_FILE" \
    --playlist-end 10 \
    --output "$DOWNLOAD_DIR/audio/%(uploader)s - %(title)s.%(ext)s" \
    --format "bestaudio" \
    --extract-audio \
    --audio-format mp3 \
    --embed-metadata \
    --embed-thumbnail \
    --write-info-json \
    --ignore-errors \
    "$YOUR_SOUNDCLOUD_URL"

# Move .info.json files to metadata folder
find "$DOWNLOAD_DIR/audio" -name "*.info.json" -exec mv {} "$DOWNLOAD_DIR/metadata/" \;

echo "Download complete! Check $DOWNLOAD_DIR"
echo "Audio files: $DOWNLOAD_DIR/audio/"
echo "Metadata: $DOWNLOAD_DIR/metadata/"  
