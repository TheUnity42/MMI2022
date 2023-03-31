mkdir -p build
cd build
cmake .. -DPICO_SDK_PATH=../../pico-sdk -DPICO_BOARD=pico_w -DWIFI_SSID=Unity -DWIFI_PASSWORD=42Unit24 
make -j12

# flag to connect to board -w

if [ "$1" = "-w" ]; then
    if [ -d "/run/media/talonh/RPI-RP2" ]; then
        echo "Uploading to Board..."
        cp main.uf2 /run/media/talonh/RPI-RP2
        # echo "Routing stdout to terminal..."
        # clear
        # sleep 1
        # cat /dev/ttyACM0
    fi
fi
