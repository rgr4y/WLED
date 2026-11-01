# PCA9557 thumb wheel

Reads a 3-way thumb wheel (up / push / down) connected to a PCA9557 I2C I/O expander, for example on the
[Seengreat RGB Matrix HUB75 S3](https://seengreat.com/wiki/214) (K1/K2/K3 on PCA9557 IO1/IO3/IO2, address 0x19).

Requires the global I2C pins to be set (Usermod settings, "Global I2C GPIOs"; SDA 1 / SCL 2 on the Seengreat board).
Only the expander's input register is read; its configuration is left untouched.

## Default actions

| Key  | Short press      | Hold                        |
|------|------------------|-----------------------------|
| up   | brightness up    | brightness up, repeating    |
| down | brightness down  | brightness down, repeating  |
| push | toggle on/off    | next effect                 |

## Settings

- `i2cAddress`: PCA9557 address (default 25 = 0x19)
- per key (`up`, `push`, `down`):
  - `io`: PCA9557 pin number (0-7) the key is wired to
  - `preset`: preset to apply on short press (0 = default action)
  - `presetLong`: preset to apply on hold (0 = default action)

If up and push are swapped on your board, swap the `io` values. With `-D WLED_DEBUG` the raw input register is printed on every change.

## Build

```ini
custom_usermods = pca9557_wheel
```
