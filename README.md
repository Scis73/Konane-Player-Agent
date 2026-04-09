# CMPT 355 - Project 2

**Group A:** Andre, Steven, Helen
**Course:** Introduction to Artificial Intelligence
**Date:** April 8, 2026

---

## Overview
This program implements a Konane game playing agent which uses an iterative deepening alpha beta search algorithm to attempt to find the best move for any Konane board state.


## Requirements
- **C99** **gcc-13**

## How to Compile (Build)

1. Open your terminal in the project directory.
2. Run the following command:
```bash
$ make
```

3. This will create an executable file named **`konaneplayer_grpA`**.

---

## How to Run
The program accepts a text file containing a string representation of a Konane board and a player colour (B or W) to indicate the current player.

**Syntax:**

./konaneplayer_grpA {board_file} {colour}

**Example:**
./konaneplayer_grpA Sample.txt B

---
## Cleaning Up
To remove the executable, run:
```bash
$ make clean
```
