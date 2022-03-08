# ECS

![GitHub Workflow Status](https://img.shields.io/github/workflow/status/MedoviyKeksik/ECS/Check%20build%20on%20Windows?label=Windows)
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/MedoviyKeksik/ECS/Check%20build%20on%20linux?label=linux)
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/MedoviyKeksik/ECS/Check%20build%20on%20macos?label=macOS)
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/MedoviyKeksik/ECS/Clang%20format%20lint?label=clang-format)

Entity Component System library.

## Table of Contents

- [About](#about)
- [Install](#install)
- [Usage](#usage)
- [Third party](#third-party)
- [Team](#team)

## About

Our implementation of the Entity Component System design pattern. This library is a rewritten library of [@tobias-stein](https://github.com/tobias-stein).
[Here](https://tsprojectsblog.wordpress.com/portfolio/entity-component-system/) you can see his post referred to ECS and his library.

## Install

### Download

Clone this repository and update submodules.
```shell
git clone https://github.com/MedoviyKeksik/ECS.git
cd ECS
git submodule update --init --recursive
```

### Build

Run CMake configure:
```shell
cmake -B build
```

Build with CMake:
```shell
cmake --build build
```

## Usage

Sample of game based on this library you can see [here](https://github.com/MedoviyKeksik/3tankista)

## Third party

- [log4cplus](https://github.com/log4cplus/log4cplus)

## Team

- [<img src="https://github.com/Sjarozha.png" title="Sjarozha" width="50" align="center"/>](https://github.com/Sjarozha) - Sergey Titov
- [<img src="https://github.com/MaXMoPKa.png" title="MaXMoPKa" width="50" align="center"/>](https://github.com/MaXMoPKa) - Vladislav Aleynikov
- [<img src="https://github.com/MedoviyKeksik.png" title="MedoviyKeksik" width="50" align="center"/>](https://github.com/MedoviyKeksik) - Klim Severin

Photo of our team when we discuss who committed this bug :)

![7373342587a464f37f1fe93490914339](https://user-images.githubusercontent.com/39442984/155014716-e425f434-ea06-40fc-9fa0-117fa5eeb2c5.png)
