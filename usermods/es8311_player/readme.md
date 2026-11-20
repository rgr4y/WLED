# ES8311 player

Speaker output through the ES8311 codec and NS4150 amp on the
[Seengreat RGB Matrix HUB75 S3](https://seengreat.com/wiki/214). Spike stage: plays a test tone.

The I2S port is shared with the mic (ES7210, `audioreactive`). While playing, this usermod pauses
audioreactive's input (`arPauseInput()`), takes the I2S pins, and hands them back (`arResumeInput()`)
when the tone ends. Without audioreactive in the build the hooks are weak no-ops.

Requires the global I2C pins (SDA 1 / SCL 2 on this board).

## Settings

| Field | Default | Meaning |
|---|---|---|
| `enabled` | true | |
| `i2cAddr` | 24 (0x18) | ES8311 address |
| `mclkPin` / `bclkPin` / `lrckPin` | 38 / 48 / 21 | I2S clocks |
| `doutPin` | 14 | I2S data to the codec (DSDIN) |
| `ampPin` | 3 | NS4150 enable, active high, -1 = none. GPIO3 is likely but unconfirmed |
| `volume` | 60 | 0-100 (0 = muted, 100 = 0 dB) |
| `sampleRate` | 44100 | 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000 |

## Test tone

```sh
curl -s -X POST http://<ip>/json/state -d '{"es8311":{"tone":1000,"ms":2000}}'
```

`tone` 50-8000 Hz (default 1000), `ms` 100-10000 (default 2000). A second request while playing
restarts the tone. The info page shows idle / playing and the last error.

## Build

```ini
custom_usermods = audioreactive es8311_player
```

The ES8311 register sequence is ported from Espressif `esp_codec_dev` (Apache-2.0).
