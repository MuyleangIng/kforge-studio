# kforge-studio

> **A native Qt6/C++ GUI for [kforge](https://github.com/MuyleangIng/kforge) — the personal multi-platform Docker image builder powered by BuildKit.**
> 
> Built by [KhmerStack](https://github.com/MuyleangIng) · Made by Ing Muyleang

---

## What is this?

`kforge-studio` is a desktop application that wraps the `kforge` CLI in a polished, Docker Desktop–style GUI. It lets you:

- **Launch multiple parallel builds** and monitor each one in a live job card (log output, progress bar, cancel button)
- **Manage BuildKit builders** — create, list, switch, and remove
- **Run declarative bake builds** from `kforge.hcl` / `kforge.json` with live log streaming
- **Run the interactive setup wizard** for QEMU / native multi-node configuration
- **See a session dashboard** — running, succeeded, and failed build counts at a glance

---

## Screenshots (planned)

| Dashboard | Build Page | Builder Manager |
|-----------|-----------|-----------------|
| _(screenshot)_ | _(screenshot)_ | _(screenshot)_ |

---

## Architecture

```
kforge-studio/
├── CMakeLists.txt              # CMake build (Qt5 & Qt6 compatible)
├── src/
│   ├── main.cpp                # QApplication entry point
│   ├── MainWindow.h/.cpp       # Root window: dark theme, sidebar, page stack
│   │                           # Owns all QProcess jobs, emits jobStatusChanged
│   ├── SidebarWidget.h/.cpp    # Left nav: logo, nav buttons, live badges
│   ├── BuildJobWidget.h/.cpp   # "Build" page: form + live job cards (JobCardWidget)
│   └── OtherWidgets.h/.cpp     # BuilderManagerWidget | BakeWidget
│                               # SettingsWidget | DashboardWidget
└── resources/
    └── resources.qrc           # Icons, fonts (optional)
```

### Data flow

```
User fills form → BuildJobWidget::onBuildClicked()
  → MainWindow::launchBuild(BuildJob)          // spawns QProcess("kforge", args)
  → QProcess::readyRead → MainWindow::onProcessReadyRead()
  → emit jobLogAppended(id, line)
  → JobCardWidget::appendLog()                 // appends to terminal view

QProcess::finished → MainWindow::onProcessFinished()
  → emit jobStatusChanged(id, "success"|"failed")
  → JobCardWidget::setStatus()                 // updates badge + border color
  → QSystemTrayIcon::showMessage()             // desktop notification
```

---

## Requirements

| Dependency | Version |
|------------|---------|
| Qt         | 5.15+ or 6.x |
| CMake      | 3.20+ |
| C++        | 17 |
| kforge     | [v1.0.0+](https://github.com/MuyleangIng/kforge/releases) in `PATH` |

---

## Build & Run

```bash
# Clone
git clone https://github.com/MuyleangIng/kforge-studio
cd kforge-studio

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Run
./build/kforge-studio
```

### macOS
```bash
cmake -B build -GXcode
open build/kforge-studio.xcodeproj
# or command-line:
cmake --build build --config Release
open build/kforge-studio.app
```

### Windows (MSVC)
```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 `
      -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\msvc2022_64"
cmake --build build --config Release
.\build\Release\kforge-studio.exe
```

---

## Roadmap

### ✅ v1.0 — Core MVP (current)
- [x] Dark-themed Qt main window with sidebar navigation
- [x] **Build page** — form with tag, context, platforms, flags, progress style
- [x] **Multiple parallel builds** — each in its own `QProcess`, live log card
- [x] **Job card** — status badge (running/success/failed/cancelled), log toggle, cancel button
- [x] **Builder Manager** — table view, create/use/remove builders
- [x] **Bake page** — run `kforge bake` with file picker, --set, env vars
- [x] **Settings** — kforge binary path, default progress/platform, setup wizard launcher
- [x] **Dashboard** — stat cards (total/running/success/failed), recent jobs tree
- [x] **System tray** — minimise to tray, desktop notification on build complete/fail
- [x] **Dark GitHub-style theme** — full QSS stylesheet

---

### 🚧 v1.1 — Polish & UX
- [ ] Persist settings to `QSettings` (INI / registry)
- [ ] Save build history to SQLite via `QtSql`
- [ ] Real-time build log ANSI color rendering (custom `QTextEdit` renderer)
- [ ] Estimated build time from previous runs
- [ ] Context-menu on job cards (copy log, open Dockerfile, re-run)
- [ ] Drag-and-drop Dockerfile / directory onto the Build page
- [ ] Multi-language i18n skeleton (Khmer + English)

---

### 🚀 v1.2 — Builder Graphs & Registry
- [ ] Pull live builder list from `kforge builder ls --json` (parse JSON)
- [ ] Registry browser — login, list tags, pull/delete images
- [ ] Image layer inspector — sizes and command per layer
- [ ] Build timeline graph (QCustomPlot or QtCharts) — seconds per stage
- [ ] Cache hit/miss visualisation
- [ ] Export build report as PDF

---

### 🌐 v1.3 — Remote Builders & Teams
- [ ] SSH remote builder wizard (multi-node)
- [ ] Kubernetes builder node provisioning dialog
- [ ] Shared builder pool — store endpoints in team config file
- [ ] Webhook trigger — listen for `git push` → auto-build
- [ ] Slack / webhook notification on build events

---

### 🔒 v1.4 — Security & Secrets
- [ ] OS keychain integration for registry credentials (libsecret / macOS Keychain / Windows Credential Store)
- [ ] Secret manager — create, list, and revoke `--secret` sources
- [ ] SBOM (Software Bill of Materials) viewer from `kforge sbom` output
- [ ] Vulnerability scan results panel (Trivy integration)

---

### 📦 v2.0 — Full Docker Desktop Replacement Mode
- [ ] Containers tab — list, start, stop, exec into running containers
- [ ] Images tab — local image list, inspect, tag, push, delete
- [ ] Volumes & Networks management
- [ ] Docker Compose runner — parse `docker-compose.yml`, start/stop stacks
- [ ] System resource monitor (CPU, memory, disk via Docker API)
- [ ] Plugin system — load kforge-studio extensions from `.so`/`.dll`

---

## Contributing

PRs welcome! Please open an issue first for large features.

```bash
# Format code
clang-format -i src/*.cpp src/*.h

# Build in Debug mode
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

---

## License

MIT © 2025 Ing Muyleang / KhmerStack
