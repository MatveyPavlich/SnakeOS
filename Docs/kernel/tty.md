# tty layer
## General notes
- tty layer would be the one responsible for managing the cursor state,
implementing backspace and arrow movement.
- console layer is there to abstract hardware calls (e.g., VGA, serial, frame
buffer) and manage multiple console instances (windows);
- keyboard IRQ -> keyboard -> tty -> console -> vga

## TODOs:
- Adjust keyboard_handle_scancode to convert scancodes into key events and send
to the tty layer.

```
enum keycode {
    KEY_NONE = 0,
    KEY_CHAR,        // printable character
    KEY_ENTER,
    KEY_BACKSPACE,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
};

struct key_event {
    enum keycode code;
    char ch;         // valid only if code == KEY_CHAR
};

// Before:
static char translate_scancode(uint8_t sc);
// After
static struct key_event translate_scancode(uint8_t sc);
```

- Create tty's ingestion API to pass key events into tty.c (e.g., `tty_input`)
- Connect tty to the console (no idea how i'd do it).
