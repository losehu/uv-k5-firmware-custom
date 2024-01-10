#!/bin/sh
rm -rf compiled
docker build -t uvk5 .
docker run --rm -v ${PWD}/compiled-firmware:/app/compiled-firmware uvk5 /bin/bash -c "cd /app && make both && cp *.bin compiled-firmware/"