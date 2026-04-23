import subprocess

Import("env")  # noqa: F821  (PlatformIO SCons global)


def _dfu_already_present():
    result = subprocess.run(["dfu-util", "-l"], capture_output=True, text=True)
    return "df11" in result.stdout


def before_upload(source, target, env):
    # Firmware exposes a USB DFU runtime interface (see src/usb_descriptors.c).
    # dfu-util -D handles the DETACH + wait-for-reenumeration automatically,
    # so this hook only prints an informational line.
    if _dfu_already_present():
        print("reset_to_dfu: board already in DFU, skipping detach")
    else:
        print("reset_to_dfu: dfu-util will detach the DFU runtime interface")


env.AddPreAction("upload", before_upload)

# Wrap the dfu-util invocation so the cosmetic exit-74 from the ':leave'
# detach doesn't mark the upload as failed. See scripts/dfu_upload.py.
env.Replace(
    UPLOADCMD=(
        '"$PYTHONEXE" "$PROJECT_DIR/scripts/dfu_upload.py" '
        "$UPLOADER $UPLOADERFLAGS $SOURCE"
    )
)
