#!/bin/bash -e

v4l2-ctl --device=/dev/video0 --set-edid=file=./1080p30 --fix-edid-checksums
