# DeviceMod

Cyber-HUD style Android modification guide and Windows ADB/Fastboot companion for Lenovo Tab M10 HD TB-X505X.

## Projects

- `android/` — Kotlin + Jetpack Compose guide app.
- `windows/` — C++17 Win32 GUI companion, designed for MSYS2 UCRT64.
- `.github/workflows/build.yml` — GitHub Actions builds Android APK and Windows EXE.
- `dist/` — local build output target (created by builds).

Android package: `com.devicemod.android`  
Windows title: `DeviceMod (Android)`

## UI direction

Inspired by the supplied Calculator project: dark navy background, cyan/amber HUD palette, monospace typography, thin neon borders, grid/radar accents, compact operation panels.

## Android build

Requires JDK 17, Android SDK 35 and Gradle 8.9.

```bash
gradle :android:assembleDebug
```

APK output:
`android/build/outputs/apk/debug/`

## Windows build (MSYS2 UCRT64)

Open the **MSYS2 UCRT64** terminal:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make
cd windows
make
```

Output:
`dist/DeviceMod.exe`

Add Android SDK Platform Tools to Windows `PATH` so `adb.exe` and `fastboot.exe` can be found by the companion app.

## Safety / scope

- Bootloader unlock is destructive and normally factory-resets the tablet.
- The X505X `fastboot oem unlock-go` command is shown as community-reported, not Lenovo-confirmed. Verify the exact firmware/build and backup first.
- Recovery and misc flashing require images specifically built for TB-X505X and the relevant release.
- The root workflow deliberately does not guess a boot partition/slot name.
- Generic ARM64 NetHunter userspace does not automatically provide a NetHunter kernel or hardware features.
- X505X splash/logo partition format and offsets are not verified. Splash flashing is intentionally locked rather than guessing a raw partition command.
- Android app is a guided checklist; it cannot run host ADB/Fastboot commands itself.
- Windows companion runs commands only after user clicks a button and asks for confirmation for destructive operations.

This is an initial scaffold, not a certified device service tool. Validate all firmware and partition details before use.
