# Flir CES 2017 Contest Entry
Non-Contact Fire and Flood Detection - FLIR CES 2017 Contest Entry
This is the code base for the fire & flood detection demonstration presented at the FLIR booth at CES 2017.

### The Code
Code written in C. The following libraries were installed on the Raspberry Pi:

  - GTK+ 3.0 (Used for the UI)
    <https://developer.gnome.org/gtk3/stable/>
    ```sh
    sudo apt-get install libgtk-3-dev
    ```
  - [Wiring Pi] (Used for GPIO)
    <http://wiringpi.com/download-and-install/>
    ```sh
    sudo apt-get purge wiringpi
    ```
  - Magick Wand (Used to convert thermal data input video)
    <https://www.imagemagick.org/script/magick-wand.php>
    ```sh
    sudo apt-get install libmagickwand-dev
    ```

### The Build
Used NetBeans IDE to develop on local machine and compile directly on the Pi. The netbeans project is included in the repo. You can see how to setup that environment [here].

Compiled on the Raspberry Pi using **gcc**.  Addtional flags:
> \`pkg-config --cflags gtk+-3.0\` \`pkg-config --libs gtk+-3.0\` \`pkg-config --cflags --libs MagickWand\` -lwiringPi

[here]: <http://www.raspberry-projects.com/pi/programming-in-c/compilers-and-ides/netbeans-windows/installing-netbeans-for-c-remote-development-on-a-raspberry-pi>
