# PomoTask
PomoTask is a CLI pomodoro timer focused on task management and simplicity, written in C.

I created this software because I wanted to have a simple, non-resource-intensive way to manage my pomodoro sessions.

[Software Demo Video](https://youtu.be/vqa2F6gP1GU)


# Getting Started
To compile on Linux and other Unix-based systems, run this command (must have the ncurses and menu installed and included in your $PATH)

```bash
gcc main.c -o out -lncurses -lmenu
```

# Development Environment
This software was developed in the C programing language on Arch Linux using Visual Studio Code.


# Sources
A helpful introduction to those interested in the Pomodoro Technique:
https://todoist.com/productivity-methods/pomodoro-technique

# Features
- Accurate countdown timer (drift-free, anchored to the monotonic clock instead of accumulating `sleep()` error)
- Settings menu with four color schemes (Green & Red, Blue & Yellow, Green & Black, White & Green) and an invert-colors toggle
- Audible chime at the end of each work/break phase, played via `paplay`/`pw-play`/`aplay` if available (falls back to the terminal bell otherwise, so it never depends on a specific audio backend and can't break PulseAudio, ALSA, or PipeWire)
- Screen strobe alert at the end of each phase
- Cancel a running session with `q` to return to the main menu at any time

# Future Development
There are several things that I have yet to implement; eg: dynamic window resizing, etc.

In addition, I would like to add a section for a user to add tasks that can be checked off and completed (hence the name "task").

I would also like to include this in the AUR (Arch User Repository) eventually as well as possible making an install script that makes installation easy on other operating systems (Windows, Mac, Debian based systems, Ubuntu based systems).

I have plans to continue developing this project in the future, but as for now, things will likely stay as they are. If you would like to contribute, please submit a pull request or fork own version! If you would like to get in contact with me personally, you can contact me at *mkmcgrath.dev@gmail.com*

Thank you!
