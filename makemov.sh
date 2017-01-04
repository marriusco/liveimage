#!/bin/bash
if [[ -d /data/snaps && -d /data/videos ]];then
pushd /data/snaps
	cat *.jpg | ffmpeg -f image2pipe -i - /data/videos/output.mkv
popd
