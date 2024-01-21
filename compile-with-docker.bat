@echo on
docker build -t uvk5 .
docker run --rm -v %CD%/compiled-firmware:/app/compiled-firmware uvk5 /bin/bash -c "cd /app && make clean && make both && cp *.bin compiled-firmware/"
pause
