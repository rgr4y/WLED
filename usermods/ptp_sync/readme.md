# Wi-Fi PTP Sync usermod

This usermod is a Wi-Fi-first scaffold for precision time sync between WLED devices.

It is intended as the landing point for Wi-Fi time-sync backends such as FTM or TSF.
Current scaffold behavior:

- keeps the usermod buildable without the Ethernet-specific `esp_ptp` dependency
- exposes basic state in `/json/state` and `/json/info`
- persists the enabled flag and selected backend in `cfg.json`
- records the intended backend as `ftm` or `tsf`

Notes:

- the backend is a scaffold today; the actual Wi-Fi measurement/clock code still needs to be wired in
- the usermod is ready to host multiple Wi-Fi sync paths without tying the build to Ethernet

Enable it in your build environment with:

```ini
custom_usermods = ${env:your_env.custom_usermods} ptp_sync
```
