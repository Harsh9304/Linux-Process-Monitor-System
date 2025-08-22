

---

Linux Process Monitor

Overview

A Command-line process monitoring tool in C++ for Linux, similar to `top/htop`. It retrieves data from the `/proc` filesystem, displays real-time system stats, and allows process management (kill, suspend, resume)**.

Features

* Live process display (PID, name, CPU%, memory%)
* System stats (CPU, memory, uptime)
* Process control (kill/suspend/resume)
* Threaded design (UI + data collection)
* Interactive ncurses terminal UI

Technologies

* C++17, POSIX Threads
* Linux system calls
* `/proc` filesystem
* ncurses library
* CMake build system

OS Concepts Covered

* Process scheduling & management
* Memory & virtual memory
* Signals, IPC & sockets
* Multi-threading
* Linux filesystem architecture

Requirements

* Linux (Ubuntu/Debian recommended)
* g++ with C++17 support
* ncurses library
* CMake

Implementation Details

Project Structure
<img width="940" height="632" alt="image" src="https://github.com/user-attachments/assets/e2ccaa83-9e23-4908-8026-81b08d431fb7" />



