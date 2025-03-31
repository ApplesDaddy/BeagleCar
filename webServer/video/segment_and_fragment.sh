#!/bin/bash

# Split the input video into segments
ffmpeg -i SampleVideo_640x360_5mb.mp4 -c:v libx264 -g 50 -keyint_min 50 -sc_threshold 0 -f segment -segment_time 2 output%03d.mp4

# Temp directory to store the fragmented segments
mkdir -p output

for segment in output*.mp4; do
    # Run the second ffmpeg command to fragment the segment
    ffmpeg -i "$segment" -movflags frag_keyframe+empty_moov+default_base_moof "output/${segment%.mp4}.mp4"

    # Move the fragmented file to the output directory, and remove the original segment
    rm "$segment"
done

mv output/* . 
rm -r output

