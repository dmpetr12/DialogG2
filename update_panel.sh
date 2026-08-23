#!/usr/bin/env bash
set -euo pipefail

REPO_URL="https://github.com/dmpetr12/DialogG2.git"
BRANCH_NAME="${BRANCH_NAME:-master}"

USER_NAME="${USER_NAME:-$(id -un)}"
USER_HOME="$(getent passwd "$USER_NAME" | cut -d: -f6)"
if [ -z "$USER_HOME" ]; then
  USER_HOME="/home/$USER_NAME"
fi

PROJECT_DIR="${PROJECT_DIR:-$USER_HOME/DialogG2}"
BUILD_DIR="$PROJECT_DIR/build"

ENGINE_SERVICE="dialog-g2-engine.service"
HMI_AUTOSTART_DIR="$USER_HOME/.config/autostart"
HMI_AUTOSTART_FILE="$HMI_AUTOSTART_DIR/dialog-g2-hmi.desktop"

OLD_PROJECT_DIR="$USER_HOME/panelFull"
OLD_SERVICE_NAMES=(
  panelFull
  panel-full
  panel-guard
  panel
  dialog-g2-engine
  dialog-g2-hmi
)

echo "== Dialog G2 panel update =="
echo "repo:    $REPO_URL"
echo "branch:  $BRANCH_NAME"
echo "user:    $USER_NAME"
echo "project: $PROJECT_DIR"
cd "$USER_HOME"

if [ "$(id -u)" -eq 0 ]; then
  echo "Do not run this script as root. Run as $USER_NAME; sudo is used only where needed."
  exit 1
fi

if ! command -v sudo >/dev/null 2>&1; then
  echo "sudo not found"
  exit 1
fi

echo "== Stopping old services and processes =="
for service in "${OLD_SERVICE_NAMES[@]}"; do
  if systemctl list-unit-files "$service.service" >/dev/null 2>&1; then
    sudo systemctl disable --now "$service.service" >/dev/null 2>&1 || true
  fi
  systemctl --user disable --now "$service.service" >/dev/null 2>&1 || true
done

pkill -f "$OLD_PROJECT_DIR" >/dev/null 2>&1 || true
pkill -x dialog-g2-engine >/dev/null 2>&1 || true
pkill -x dialog-g2-hmi >/dev/null 2>&1 || true

echo "== Installing build dependencies =="
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  git \
  ninja-build \
  pkg-config \
  qt6-base-dev \
  qt6-declarative-dev \
  qt6-httpserver-dev \
  libcap2-bin \
  psmisc \
  qt6-serialbus-dev \
  qt6-serialport-dev \
  qml6-module-qtquick \
  qml6-module-qtquick-controls \
  qml6-module-qtquick-layouts

echo "== Removing previous DialogG2 source/build =="
rm -rf "$PROJECT_DIR"

echo "== Cloning DialogG2 =="
git clone --branch "$BRANCH_NAME" --single-branch "$REPO_URL" "$PROJECT_DIR"

echo "== Configuring CMake =="
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release

echo "== Building =="
cmake --build "$BUILD_DIR" -j"$(nproc)"

echo "== Preparing runtime directories =="
mkdir -p "$BUILD_DIR/logs" "$BUILD_DIR/state"
sudo usermod -aG dialout "$USER_NAME" || true
sudo fuser -k 502/tcp >/dev/null 2>&1 || true
sudo setcap cap_net_bind_service=+ep "$BUILD_DIR/dialog-g2-engine" || true

echo "== Installing engine systemd service =="
sudo tee "/etc/systemd/system/$ENGINE_SERVICE" >/dev/null <<SERVICE
[Unit]
Description=Dialog G2 engine
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
WorkingDirectory=$BUILD_DIR
ExecStart=$BUILD_DIR/dialog-g2-engine
Restart=always
RestartSec=2
User=$USER_NAME

[Install]
WantedBy=multi-user.target
SERVICE

sudo systemctl daemon-reload
sudo systemctl enable --now "$ENGINE_SERVICE"

echo "== Installing HMI desktop autostart =="
mkdir -p "$HMI_AUTOSTART_DIR"
cat > "$HMI_AUTOSTART_FILE" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Dialog G2 HMI
Exec=$BUILD_DIR/dialog-g2-hmi
Path=$BUILD_DIR
Terminal=false
AutostartEnabled=true
X-GNOME-Autostart-enabled=true
DESKTOP

chmod 755 "$HMI_AUTOSTART_FILE"

echo "== Cleaning old panelFull autostart files if present =="
rm -f "$USER_HOME/.config/autostart/panel.desktop" \
      "$USER_HOME/.config/autostart/panelFull.desktop" \
      "$USER_HOME/.config/autostart/apppanel.desktop"

echo "== Status =="
sudo systemctl --no-pager --full status "$ENGINE_SERVICE" || true

echo "== Done =="
echo "Engine service: $ENGINE_SERVICE"
echo "HMI autostart:  $HMI_AUTOSTART_FILE"
echo "Runtime dir:    $BUILD_DIR"
echo "Web UI:         http://$(hostname -I | awk '{print $1}'):8080"
echo
echo "If HMI is not visible yet, reboot the panel:"
echo "  sudo reboot"
