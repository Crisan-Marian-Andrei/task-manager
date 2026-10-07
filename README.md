# C Task Manager

A lightweight, interactive top/htop alternative written from scratch in C for Linux systems. It reads process data directly from the `/proc` filesystem and features a dynamic, flicker-free terminal interface.

## 🚀 Features
* **Live Sorting:** Sort by CPU (`c`) or Memory (`m`) usage.
* **Process Filtering:** Press `/` to enter Live Search mode and filter processes by name instantly.
* **Process Management:** Navigate with Arrow Keys and kill selected processes (`k`).
* **Visual States:** Color-coded process states (Running, Sleeping, Zombie).
* **Flicker-Free:** Uses ANSI escape codes and double-buffering logic for smooth 1-second interval updates.
* **Dynamic Allocation:** Safely handles heavily loaded systems without buffer overflows.

## 🛠️ Compilation & Usage

```bash
# Compile the source code
gcc -Wall -o task_manager proc.c procese.c

# Run the executable
./task_manager

⌨️ Keybindings
q - Quit application

c - Sort by CPU usage

m - Sort by Memory usage

r - Reverse sort order

/ - Toggle Live Search mode

k - Kill selected process (sends SIGKILL)

Up / Down Arrows - Navigate the process list