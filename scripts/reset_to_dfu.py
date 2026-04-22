import subprocess
import time

import serial
import serial.tools.list_ports

Import("env")  # noqa: F821  (PlatformIO SCons global)


def _find_cdc_port(env):
    port = env.get("UPLOAD_PORT")
    if port:
        return port
    for p in serial.tools.list_ports.comports():
        desc = (p.description or "").upper()
        if "STM32" in desc or "CDC" in desc:
            return p.device
    return None


def _wait_for_dfu(timeout=5.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        result = subprocess.run(
            ["dfu-util", "-l"], capture_output=True, text=True
        )
        if "df11" in result.stdout:
            return True
        time.sleep(0.1)
    return False


def before_upload(source, target, env):
    port = _find_cdc_port(env)
    if port:
        print(f"reset_to_dfu: touching {port} at 1200 baud")
        try:
            s = serial.Serial(port, 1200, timeout=1)
            s.close()
        except Exception as exc:
            print(f"reset_to_dfu: serial touch failed ({exc}), trying DFU directly")
    else:
        print("reset_to_dfu: no CDC port found, trying DFU directly")

    if not _wait_for_dfu():
        print("reset_to_dfu: DFU device not found after 5 s — is the board connected via USB?")


env.AddPreAction("upload", before_upload)
