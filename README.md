# sscvisual-gtk
simple steam user fetching tool using `curl` and `GTK 3.0` <br>
[!Note] this is purely for educational purpose and therefore unexpected
issues may be encountered within the experience. I personally will
continue to maintain on this repository as long as I have 
enough spare time.

## dependencies
following packages shall be in presence in the desktop environments,
in order to build the executable from source without issues: <br>

- json-c
- curl
- gtk3

I have only tested the build process on Arch Linux, although most of
the Unix-based OS shall compile flawlessly, I'm not sure whether any
weird problems might come up in other environments. besides, here 
very possible that this is not nicely compatible with Windows 10/11,
where i had the multi-thread components run a little weird and 
eventually crashing the process. <br>

here's a possible approach for compiling sscvisual-gtk:

`$ sudo pacman -Syu json-c curl gtk3` <br>
`$ cd ./src`                          <br>
`$ make`                              <br>

