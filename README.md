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

# Future Development
There are several things that I have yet to implement; eg: dynamic window resizing, color options, a notification or audible bell to allow users to know when their session is over (I will probably use SDL for this), etc.

I would also like to include this in the AUR (Arch User Repository) eventually as well as possible making an install script that makes installation easy on other operating systems (Windows, Mac, Debian based systems, Ubuntu based systems).

I have plans to continue developing this project in the future, but as for now, things will likely stay as they are. If you would like to contribute, please submit a pull request or fork own version! If you would like to get in contact with me personally, you can contact me at *mkmcgrath.dev@gmail.com*

Thank you!
