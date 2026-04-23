"""Thin dfu-util wrapper that swallows the cosmetic 'leave' exit code.

PlatformIO's ststm32 DFU upload appends `:leave` to the dfu-util download
spec, asking the ROM bootloader to jump to user code once the flash is
done. The bootloader does that — and with it, detaches USB before
dfu-util can read back the final DFU status. dfu-util then reports
'Error during download get_status' and exits 74 (EX_IOERR), even though
the firmware is already running. We only swallow exit 74 when dfu-util
itself confirmed the download succeeded.
"""

import subprocess
import sys


def main():
    if len(sys.argv) < 2:
        print("usage: dfu_upload.py <dfu-util> [args...]", file=sys.stderr)
        return 2

    proc = subprocess.Popen(
        sys.argv[1:],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )
    saw_success = False
    for line in proc.stdout:
        sys.stdout.write(line)
        sys.stdout.flush()
        if "File downloaded successfully" in line:
            saw_success = True
    proc.wait()

    if proc.returncode == 74 and saw_success:
        print("dfu_upload: flash OK, ignoring expected post-leave status error")
        return 0
    return proc.returncode


if __name__ == "__main__":
    sys.exit(main())
