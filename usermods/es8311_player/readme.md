# ES8311 player

Speaker output through the ES8311 codec and NS4150 amp on the
[Seengreat RGB Matrix HUB75 S3](https://seengreat.com/wiki/214). Plays a test tone and
network audio (raw PCM over UDP).

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
| `volume` | 100 | 0-100 (0 = muted, 1-100 = -47.5 to 0 dB in 0.5 dB steps) |
| `sampleRate` | 44100 | 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000 |
| `udpPort` | 4049 | UDP port for network audio (WLED already uses 21324, 4048, 5568, 6454, 65506, 11988) |
| `bufferMs` | 150 | audio buffered before playback starts (20-500); this is also the delay against the picture |
| `timeoutMs` | 1000 | no packets for this long = stream over, speaker off, mic back (100-10000) |

## Test tone

```sh
curl -s -X POST http://<ip>/json/state -d '{"es8311":{"tone":1000,"ms":2000}}'
```

`tone` 50-8000 Hz (default 1000), `ms` 100-10000 (default 2000). A second request while playing
restarts the tone. The info page shows idle / playing and the last error.

## Volume

```sh
curl -s -X POST http://<ip>/json/state -d '{"es8311":{"vol":80}}'
```

Takes effect immediately, even mid-stream, and is saved to the config. `/json/state` reports the
current value as `es8311.vol`.

## Network audio

Packet format, little-endian, one UDP datagram each:

| Bytes | Content |
|---|---|
| 0-3 | magic `"WAU1"` |
| 4-7 | sequence number (u32, +1 per packet) |
| 8-11 | sample rate in Hz (u32), must equal `sampleRate` |
| 12 | channels, 1 or 2 (stereo is downmixed to mono) |
| 13-15 | reserved, 0 |
| 16- | PCM, signed 16-bit |

Packets with the wrong magic, a different sample rate, bad channel count or an odd payload are
dropped (a rate mismatch is logged once). Late and duplicate sequence numbers are dropped; a gap
shorter than 50 ms is filled with silence. The first valid packet pauses audioreactive and starts
the output; it plays once `bufferMs` is buffered (about 1 s of buffer in PSRAM). On underrun it
goes silent and re-buffers. After `timeoutMs` without packets the rest is played out and the I2S
port goes back to audioreactive. The test tone is ignored while a stream plays. Keep packets
around 1000 bytes of PCM (500 samples) so they fit one frame.

Test sender (`send.py`, any rate in the table, needs ffmpeg):

```python
import socket, struct, subprocess, sys, time
host, path, rate = sys.argv[1], sys.argv[2], int(sys.argv[3]) if len(sys.argv) > 3 else 44100
ff = subprocess.Popen(["ffmpeg", "-re", "-i", path, "-vn", "-ac", "1", "-ar", str(rate),
                       "-f", "s16le", "-"], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
sock, seq = socket.socket(socket.AF_INET, socket.SOCK_DGRAM), 0
while True:
    pcm = ff.stdout.read(1000)          # 500 samples, ~11 ms at 44.1k; -re paces ffmpeg
    if len(pcm) < 2: break
    pcm = pcm[:len(pcm) & ~1]
    sock.sendto(b"WAU1" + struct.pack("<IIB3x", seq, rate, 1) + pcm, (host, 4049))
    seq += 1
```

`python3 send.py <ip> in.mp4 44100`. ffmpeg's `-re` reads at playback speed, so no timing code is
needed. Audio lags the picture by about `bufferMs`; delay the video by that much on the host.

## Build

```ini
custom_usermods = audioreactive es8311_player
```

Pin defaults are for the Seengreat board. Other boards override them with build flags
(`ES8311_MCLK_PIN`, `ES8311_BCLK_PIN`, `ES8311_LRCK_PIN`, `ES8311_DOUT_PIN`, `ES8311_AMP_PIN`).
Waveshare ESP32-S3-RGB-Matrix, from its schematic:

```ini
  -D ES8311_MCLK_PIN=12 -D ES8311_BCLK_PIN=43 -D ES8311_LRCK_PIN=38 -D ES8311_DOUT_PIN=21 -D ES8311_AMP_PIN=11
```

Use the in-tree `audioreactive` with it; other forks lack the `arPauseInput()` hook, so the shared
I2S pins stay taken by the mic and playback fails with "I2S/amp pin allocation failed".

The ES8311 register sequence is ported from Espressif `esp_codec_dev` (Apache-2.0).
