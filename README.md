# ESP32 Projects

A small collection of little projects to fiddle with ESP32s and learn how they
work.

## Setup

`sudo usermod -a -G dialout $USER`
`sudo usermod -a -G tty $USER`

## On First Clone


These projects have only been tested using VSCode with the PlatformIO 
extension.

Install the PIO CLI using a method described in the [official documentation](https://docs.platformio.org/en/stable/core/installation/methods/installer-script.html#super-quick-macos-linux).

Aliases are then created;
```
alias platformio='~/.platformio/penv/bin/platformio'
alias pio='platformio'
```

In the latest Ubuntu you have to disable brltty otherwise the ESP32 devices are 
immediately disconnected when plugged in.  It's unclear to me what you're 
supposed to do it you are a user who requires a Braile terminal.  See 
instructions (here)[https://www.reddit.com/r/pop_os/comments/uf54bi/how_to_remove_or_disable_brltty/] 
and reproduced below;

```
systemctl stop brltty-udev.service
sudo systemctl mask brltty-udev.service
systemctl stop brltty.service
systemctl disable brltty.service
```

After cloning the repo and installing the appropriate plugin you need to add the
following libraries in PlatformIO;

- Heltec ESP32 Dev-Boards by Heltec Automation

