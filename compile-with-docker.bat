@echo on
make clean
docker build -t uvk5 .
docker run --rm -v %CD%\compiled-firmware:/app/compiled-firmware uvk5 /bin/bash -c "cd /app &&rm -rf compiled&& make clean && make full&& cp *.bin compiled-firmware/"
pause
