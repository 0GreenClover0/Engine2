<h1 align="center"><b>Foilaĝe</b></h3>

<p align="center">
  <img src="readme_files/cover.png" width = "400"/>
</p>

## What is it?
This is a fork of a custom game engine and contains code of a "Komiks Game Jam 2k25" game (and engine) "Foilaĝe".
For more info about the engine and its origin, please check the [<b>original repository under this link</b>](https://github.com/0GreenClover0/Engine).

## Story
**You're a truther.** And you know that **aliens exist**. 👽 Everyone knows that. You just need to convince them a little. So you **make crazy symbols in wheat** yourself!

However, not everyone is happy with it. Some **suspicious cows** or **a farm owner**, for instance... Will you create the symbols against the odds?

<p align="center">
  <img src="readme_files/foilage.gif" width = "500"/>
</p>

## How do I build and run this?
Run the following command to generate the build system:
```
cmake -B build
```
or use the `build.bat` which does the same.

Install the required Python packages specified in the `requirements.txt` file in the EngineHeaderTool directory.
You can install them with `pip` using this command:

`py -m pip install -r .\requirements.txt`

You will also need to have Developer mode in Windows activated, since the build system needs to have permissions to create symlinks.

To compile and run, C++ compiler with C++23 support is required. MSVC 14.50 has been tested.

`.slnx` file is located in the generated `/build` directory.
You can open it in Visual Studio, choose the desired build configuration (`Debug` is the default), and simply run it.

## Game Creators
| Name | Link | Role |
|------|--------|--------|
| Mikołaj Przybylski | https://github.com/0GreenClover0| Programming Lead |
| Michał Galiński | https://github.com/MikeMG-PL| Programming, Production |
| Miłosz Kawczyński | https://github.com/MiloszKawczynski | Programming, Design |
| Karolina Sołtysiak | https://github.com/dramatiCatt | 2D & 3D Art |
| Michał Świstak | https://soundcloud.com/michal_swistak | Sound Design |

## Engine Programmers
| Name | Link |
|------|--------|
| Mikołaj Przybylski | https://github.com/0GreenClover0|
| Hubert Olejnik | https://github.com/umbc1ok |
| Miłosz Kawczyński  | https://github.com/MiloszKawczynski
| Michał Galiński | https://github.com/MikeMG-PL|