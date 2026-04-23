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


def _wait_for_dfu(timeout=8.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        result = subprocess.run(
            ["dfu-util", "-l"], capture_output=True, text=True
        )
        if "df11" in result.stdout:
            return True
        time.sleep(0.1)
    return False


def _touch_1200(port):
    # Firmware detects DFU entry on the DTR falling edge while line coding
    # is 1200 baud (classic Arduino reset-to-bootloader). pyserial opens
    # with DTR asserted by default, so opening at 1200 gives us the
    # "asserted" half of the edge. Sleep so the host USB stack actually
    # gets SET_LINE_CODING onto the wire, then drop DTR explicitly for
    # the falling edge. Don't set dtr/rts pre-open — on macOS that turns
    # into a TIOCMSET during open() and can fail with ENXIO on CDC ttys.
    s = serial.Serial(port, 1200)
    time.sleep(0.1)
    s.dtr = False
    time.sleep(0.05)
    s.close()


def before_upload(source, target, env):
    # If the board is already sitting in DFU (e.g. from a prior failed
    # flash), skip the touch entirely — opening the stale CDC tty node
    # would just fail.
    probe = subprocess.run(["dfu-util", "-l"], capture_output=True, text=True)
    if "df11" in probe.stdout:
        print("reset_to_dfu: board already in DFU mode, skipping touch")
        return

    port = _find_cdc_port(env)
    if port:
        print(f"reset_to_dfu: touching {port} at 1200 baud (DTR drop)")
        try:
            _touch_1200(port)
        except Exception as exc:
            print(f"reset_to_dfu: serial touch failed ({exc}), trying DFU directly")
    else:
        print("reset_to_dfu: no CDC port found, trying DFU directly")

    if not _wait_for_dfu():
        print("reset_to_dfu: DFU device not found after 8 s — is the board connected via USB?")


env.AddPreAction("upload", before_upload)

# Wrap the dfu-util invocation so the cosmetic exit-74 from the ':leave'
# detach doesn't mark the upload as failed. See scripts/dfu_upload.py.
env.Replace(
    UPLOADCMD=(
        '"$PYTHONEXE" "$PROJECT_DIR/scripts/dfu_upload.py" '
        "$UPLOADER $UPLOADERFLAGS $SOURCE"
    )
)
