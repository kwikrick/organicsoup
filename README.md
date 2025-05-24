# Organic Soup

An artifical chemisty toy. 

An articfical chemistry is a program that simulates or emulates chemical reactions. 
This particular chemistry is very simplfied and abstracted away from the real world. 
It's purpose is to allow exploration of interesting and potenially life-like systems.

This program was inspired by and is a partial re-creation of [Organic Builder](https://bertranddechoux.github.io/OrganicBuilder/). 

The web version of Organic Soup can be played with here: https://kwikrick.github.io/organicsoup/

## Building

Organic soup is coded in C++, using the SDL2 library and ImGui.

Build has been tested on Ubuntu (Noble), but should be easily portable to any Linux system. 
On Windows, Mac, etc., you're on your own. 

### Linux

Pre-requisites:
 - make
 - gcc (somewhat recent version supporting c++20)
 - libSDL2
 - libSDL2_ttf
 - libGL

Note: ImGui is included in the source tree. It's just a copy of the repository, but you can update it with git if you need to. 

From the command line run:
```
make CONFIG=linux
```
or simply
```
make
```

The result is an exectuble named `organicsoup` in directory `build_linux`.

### Web

To build the web version, Emscripten is used to create a webpage with Javascript and webAssembly. 

Pre-requisites:
 - make
 - Empscripten SDK (https://emscripten.org/)

Note: The Empscripten SDK includes SDL2 and an openGL/webGL implementation, so you don't need 
to install the libSDL2 or libGL development packages. 

From the command line run:
```
make CONFIG=web
```
The result are files `index.html`, `index.js` and `index.wasm` in directory `build_web`.

Note that the `web` directory contains a prebuild version. 

To locally host the page and run it in a webbrower, use:

```
emrun build_web/index.html
```

