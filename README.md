# CSOPESY - README

## Group Developers

- Austria, Ma. Alexandria
- De Leon, Sofia Ysabela
- Guererro, Laura Mae
- Patricio, Anne Beatriz

## Entry Class / Main Function

- **Entry file:** `csopesy.cpp`
- **Main function:** `main()` in `csopesy.cpp`

## Program Requirements

- C++ compiler with C++17 support
- Windows is recommended because the program uses Windows Console API features for the animated marquee.
- A terminal/console that supports ANSI escape sequences is recommended.

## How to Compile

Using g++/MinGW on Windows:

```bash
g++ -std=c++17 -O2 -o csopesy.exe csopesy.cpp -lpthread
```

This creates the executable:

```text
csopesy.exe
```

## How to Run

After compiling, run:

```bash
main.exe
```

Or from the terminal:

```bash
./main.exe
```

## Program Commands

Type `help` to display the available commands.

### `help`

Displays the commands and their descriptions.

### `start_marquee`

Starts the animated marquee.

### `stop_marquee`

Stops the animated marquee.

### `set_text <text>`

Sets the text that will be displayed by the marquee.

Example:

```text
set_text Hello CSOPESY!
```

### `set_speed <milliseconds>`

Sets the marquee animation refresh speed in milliseconds.

Example:

```text
set_speed 100
```

### `exit`

Terminates the program.

## Basic Usage Example

1. Compile the program using the command above.
2. Run `csopesy.exe`.
3. Type:

```text
help
```

4. Set the marquee text:

```text
set_text Welcome to CSOPESY!
```

5. Start the animation:

```text
start_marquee
```

6. To stop it:

```text
stop_marquee
```

7. To change the animation speed:

```text
set_speed 50
```

8. To close the program:

```text
exit
```
