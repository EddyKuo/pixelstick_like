#!/bin/bash
# Read the parameter and resize the picture with any format file.
# Notice the delay of the shell to convert the image.

if [ "$#" -lt 1 ]; then
    echo "Please use the shell likt this:"
    echo "$0 your_image.jpg"
    exit 0
fi

echo "Convert image \"$1\" into "\"$1.bmp\"""
convert -resize x144 $1 $1.bmp

echo "Start running!!"

if [ "$#" -eq 1 ]; then
    echo "Set Brightness: 1.0"
    sudo ./strip.out $1.bmp 1.0
else
    echo "Set Brightness: $2"
    sudo ./strip.out $1.bmp $2
fi

echo "Delete template file... $1.bmp"
rm $1.bmp

exit 0