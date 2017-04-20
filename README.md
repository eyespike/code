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
Compiled on the Raspberry Pi using **gcc**. 
Addtional flags:
> \`pkg-config --cflags gtk+-3.0\` \`pkg-config --libs gtk+-3.0\` \`pkg-config --cflags --libs MagickWand\` -lwiringPi


