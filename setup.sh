#!/bin/sh

echo "Downloading image-net-2012.sqlite3"
curl -L -o bin/data/image-net-2012.sqlite3 --progress-bar https://raw.githubusercontent.com/liuliu/ccv/unstable/samples/image-net-2012.sqlite3

echo "Done"
