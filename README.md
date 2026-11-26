# MCB1700 Media Center

**A bare-metal media center for the ARM Cortex-M3: a photo gallery, a USB audio player and two games, written in C on the NXP LPC1768.**

![C](https://img.shields.io/badge/C-bare--metal-00599C?logo=c&logoColor=white)
![ARM](https://img.shields.io/badge/ARM-Cortex--M3-0091BD?logo=arm&logoColor=white)
![MCU](https://img.shields.io/badge/MCU-NXP%20LPC1768-F7A800)
![IDE](https://img.shields.io/badge/IDE-Keil%20%C2%B5Vision-1E5BA8)
![USB](https://img.shields.io/badge/USB-Audio%20Class-555)
![License](https://img.shields.io/badge/license-MIT-green)

![Main menu, Tetris and Snake on the 320×240 LCD](docs/img/hero.png)

Everything runs on a 100 MHz microcontroller with 64 KB of RAM. There's no operating system, frame buffer or graphics library: just a 320×240 SPI LCD, a 5-way joystick, a potentiometer and a speaker. The project is a single firmware image with a joystick-driven menu and four apps.

| | |
|---|---|
| ![Photo gallery](docs/img/gallery.png) | ![MP3 player](docs/img/mp3.png) |
| **Photo gallery**: RGB565 images stored in flash, streamed to the LCD one row at a time | **USB audio player**: the board shows up on the PC as a USB speaker; the knob sets the volume |
| ![Tetris](docs/img/tetris.png) | ![Snake](docs/img/snake.png) |
| **Tetris**: rotation, wall collision, line clears, scoring and levels that speed up | **Snake**: grows on each apple and gets faster as the score climbs |

> Screens above are rendered from the UI code at the LCD's native 320×240 resolution.

## Highlights

- **Bare-metal C on a Cortex-M3.** A 1 ms SysTick time base drives every timer, debounce and game tick. No RTOS, no heap, no dynamic allocation.
- **Partial redraws instead of a frame buffer.** A full frame would need 150 KB of RAM; the chip has 64 KB. The games redraw only the cells that change (a moved piece, the snake's head and tail), so the slow SPI display keeps up.
- **Compact Tetris engine.** Each of the 7 tetrominoes stores its 4 rotations as 16-bit masks of a 4×4 grid; one `fits()` check handles walls, floor and stacked blocks for moves and rotations alike. Scoring follows the classic 40 / 100 / 300 / 1200 table times the level.
- **USB Audio Class streaming.** Audio comes from the PC over USB, a timer interrupt feeds samples to the 10-bit DAC, and the ADC reads the potentiometer for volume. Leaving the player disconnects USB and stops the interrupts so the audio stops cleanly and can be re-entered.
- **Image pipeline.** `tools/img2c.py` turns any JPG or PNG into an RGB565 C array, the same format as GIMP's "C source" export, so new photos drop straight into the gallery.

## Hardware

| Part | Used for |
|---|---|
| Keil MCB1700 board, NXP LPC1768 (ARM Cortex-M3, 100 MHz, 512 KB flash, 64 KB RAM) | Everything |
| 320×240 colour LCD (SPI) | All graphics |
| 5-way joystick (GPIO) | Navigation and game controls |
| Potentiometer (ADC) | Volume |
| Speaker (DAC) | Audio output |
| USB device port | Audio stream from the PC |
| 8 LEDs (GPIO) | Show the active menu item or app |

## How it works

```mermaid
flowchart LR
    A[Reset] --> B[Init SysTick, LEDs, joystick, LCD]
    B --> M[Main menu]
    M -- SELECT --> G[Photo gallery]
    M -- SELECT --> P[USB audio player]
    M -- SELECT --> T[Tetris]
    M -- SELECT --> S[Snake]
    G -- SELECT --> M
    P -- "SELECT: USB disconnect, IRQs off" --> M
    T -- "SELECT / game over" --> M
    S -- "SELECT / game over" --> M
```

The menu is a simple state machine. Each app takes over the main loop until the user presses SELECT, then hands control back and the menu redraws itself.

```mermaid
flowchart LR
    PC[PC audio] -- USB isochronous --> USB[USB device controller]
    USB -- interrupt --> BUF[Sample buffer]
    T0[Timer 0 interrupt] --> BUF
    BUF --> VOL[x volume]
    POT[Potentiometer] -- ADC --> VOL
    VOL --> DAC[10-bit DAC] --> SPK[Speaker]
```

## Controls

| | Menu | Gallery | Tetris | Snake |
|---|---|---|---|---|
| **Up** | Previous item | | Rotate | Turn up |
| **Down** | Next item | | Soft drop | Turn down |
| **Left / Right** | | Previous / next photo | Move (hold to repeat) | Turn |
| **Select** | Open | Back | Quit | Quit |

## Project layout

```
src/
  main.c        menu, SysTick time base, joystick edge detection, drawing helpers
  common.h      shared declarations
  gallery.c     photo carousel (row-by-row bitmap streaming, optional mirroring)
  audio.c       USB audio player screen, clean start and stop
  tetris.c      Tetris engine and renderer
  snake.c       Snake engine and renderer
  photo1-4.c    RGB565 images (160×120)
tools/
  img2c.py      JPG/PNG to RGB565 C array converter
docs/img/       screenshots and source images
```

## Building

1. Start from Keil's **USBAudio** example for the MCB1700, which provides the USB stack, startup code and clock setup.
2. Add the board's LCD, joystick and LED drivers (`GLCD_SPI_LPC1700.c`, `KBD.c`, `LED.c`) and their headers to the include path.
3. Rename the example's `main()` to `audio_start()` and remove its `SystemInit()` call and final `while (1);`.
4. Add everything in `src/` to the project **except** `photo*.c` (`gallery.c` includes those directly).
5. Build and flash with Keil µVision (F7, then F8).

The Keil/NXP example and driver code isn't included here; only the code written for this project is.

### Adding your own photos

```bash
pip install pillow
python tools/img2c.py my_picture.jpg photo1          # writes photo1.c (160×120, RGB565)
python tools/img2c.py my_picture.jpg photo1 --flip   # mirrored, if your LCD draws it backwards
```

## Built with

C (C89), Keil µVision 5 and the ARM compiler, CMSIS, Python and Pillow for the image tool.

---

Built for COE718 Embedded Systems Design at Toronto Metropolitan University by **Ridham Aggarwal**.
