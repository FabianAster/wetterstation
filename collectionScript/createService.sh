#!/bin/bash

# Usage: ./create_python_service.sh /path/to/script.py /path/to/venv

SCRIPT_PATH="$1"
VENV_PATH="$2"
SERVICE_NAME=$(basename "$SCRIPT_PATH" .py)

if [[ -z "$SCRIPT_PATH" || -z "$VENV_PATH" ]]; then
  echo "Usage: $0 /path/to/script.py /path/to/venv"
  exit 1
fi

if [[ ! -f "$SCRIPT_PATH" ]]; then
  echo "Python script not found: $SCRIPT_PATH"
  exit 2
fi

if [[ ! -d "$VENV_PATH" ]]; then
  echo "Virtualenv not found: $VENV_PATH"
  exit 3
fi

SERVICE_FILE="/etc/systemd/system/${SERVICE_NAME}.service"

cat <<EOF | sudo tee "$SERVICE_FILE"
[Unit]
Description=Python Service for $SCRIPT_PATH
After=network.target

[Service]
Type=simple
User=$(whoami)
WorkingDirectory=$(dirname "$SCRIPT_PATH")
ExecStart=/bin/bash -c 'source "$VENV_PATH/bin/activate" && exec python "$SCRIPT_PATH"'
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable --now "$SERVICE_NAME"

echo "Service $SERVICE_NAME enabled and started."
