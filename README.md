# VoidClip – Soft Clipper / Saturator / Compressor (VST3, Windows)

Signal-Kette: Input -> Low Cut -> Kompressor -> [Oversampling: Saturation -> Soft Clipper (optional Low-End-Split)] -> High Cut -> Width -> Dry/Wet -> Output

## Features
- **Compressor**: Peak/RMS, Threshold, Ratio, Attack, Release, Soft-Knee, Makeup, Parallel-Mix, Sidechain-HPF, GR-Meter
- **Saturation**: 6 Typen (Tube, Tape, Diode, Fold, Crunch, Crush), Drive, Bias, Tone-Tilt, Mix, Auto-Gain
- **Soft Clipper**: 6 Kurven (Tanh, Arctan, Cubic, Sine, Exp, Hard), Drive, Ceiling, Knee, Asymmetrie, Safe-Ceiling
- **Low End Clip**: eigener Clip-Drive fuer alles unter der Split-Frequenz (808-Punch)
- Low/High Cut, Stereo Width, Dry/Wet, Output Gain
- Oversampling Off/2x/4x/8x (latenzkompensiert), Transfer-Kurven-Anzeige, In/Out/GR-Meter
- 9 Presets (Startpunkte fuer Rage/Underground-Sounds)

## Build ueber GitHub
1. Ordner als Repo pushen (Branch `main`).
2. Actions -> "Build VST3 + Installer (Windows)" laeuft automatisch.
3. Artifact `VoidClip-Windows` herunterladen: `VoidClip-Setup-1.0.0.exe` (Installer) + `VoidClip.vst3`.
4. Fuer ein Release: `git tag v1.0.0 && git push --tags`.

Installationsziel: `C:\Program Files\Common Files\VST3\VoidClip.vst3`
