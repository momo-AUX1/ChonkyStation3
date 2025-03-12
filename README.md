

<h1 align="center">
  <strong>ChonkyStation3</strong>
</h1>

<h4 align="center">
  <strong>A PlayStation® 3 Emulator</strong>
</h4>


---



<p align="center">
ChonkyStation3 is a work-in-progress emulator for the PlayStation 3 system.<br>
It can currently boot a few simple commercial games.<br>
This is a hobby project I'm developing for fun and to learn.<br>
You should not use this to actually play games. At least not for now.<br>
</p>


<p align="center">
The Xbox support in this project assumes you're building with MSYS2 and that you have a UWP-capable SDL2 + OpenGL context.<br>
<a href="https://www.msys2.org">MSYS2</a> | <a href="https://github.com/momo-AUX1/sdl-cs-uwp">SDL C# UWP</a><br>
For info and general guidance please see this wiki: <a href="https://wiki.xboxdev.store/en/Chonkystation3Guide">Dev store wiki</a>
</p>




---

Media

<table align="center">
  <tr>
    <td><img src="Resources/TLOU.gif" alt="The Last of Us"></td>
    <td><img src="Resources/arkedo_series_pixel_menu.png" alt="Arkedo Series Pixel Menu"></td>
  </tr>
</table>

---

Building for Xbox (UWP)

To build ChonkyStation3 for Xbox, follow these steps:

1. Clone the Repository

```
git clone https://github.com/momo-AUX1/ChonkyStation3.git
cd ChonkyStation3
```
2. Install Dependencies

Make sure you have CMake, MSYS2, and an SDL2 UWP-compatible project.

For MSYS2:

```
pacman -Syu
pacman -S --needed base-devel mingw-w64-x86_64-SDL2 mingw-w64-x86_64-cmake
```

For SDL2 UWP:
Download and install SDL C# UWP or SDL C++ UWP.

3. Configure and Build

To compile for Xbox, enable the Xbox build flag using CMake:

```
mkdir buildxbox && cd buildxbox
cmake -DENABLE_XBOX_BUILD=ON ..
make
```

4. Running on Xbox

Once built, you should package the DLL inside SDL C#/C++ UWP. Make sure to call it's external main function while giving it the SDLWindow pointer and GLContext pointer.
