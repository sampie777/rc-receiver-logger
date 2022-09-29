Car Computer
====================

This is project uses [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).

# Workspace setup

1. Install [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/v4.4.1/esp32/get-started/index.html#get-started).
2. Clone this project using `git clone` command.
3. Open project with CLion.
4. Tell CLion to open the project as CMake project.
5. Setup CMake profile enviroment varables as in [the video](https://www.youtube.com/watch?v=M6fa7tzZdLw&t=162s).
6. Use 'Reset Cache and Reload Project' from the settings icon in the CMake tab in CLion.

Now you can **build** (Ctrl+F9) the different configurations (app, flash, monitor) (even though not executor is defined)

### Terminal usage

If you want to use the terminal, make sure you've fired up EDS-IF using the `get_idf` command (if you've set that alias during ESP-IDF install).

Commands to use:
- `idf.py build`
- `idf.py [-p /dev/ttyUSB0] flash` (build included)
- `idf.py [-p /dev/ttyUSB0] monitor`


# Project setup

1. Copy and rename the `secrets.h.template` file to `secrets.h` and edit the values to your need.

#### New files

Remember to add newly created files to the `CMakeLists.txt` in `main/`.
