Import("env")

import glob
import os
import re
import sys

try:
    Import("pio_lib_builder")
    lib_root = pio_lib_builder.path
except Exception:
    lib_root = os.getcwd()

src = os.path.join(lib_root, "src")
board = env.BoardConfig()


def flag(prefix):
    for f in env.Flatten([env.get("CCFLAGS", []), env.get("LINKFLAGS", [])]):
        f = str(f)
        if f.startswith(prefix):
            return f[len(prefix):]
    return None


def archives(path):
    return sorted(glob.glob(os.path.join(path, "libmt_*.a")))


FLOAT_ABI = re.compile(r"(softfp|soft|hard)")
FPU = re.compile(r"fpv[0-9a-z-]*?d16|fpv[0-9a-z-]+")


def float_spec(name):
    """(fpu, abi) encoded in a folder name such as 'fpv4-sp-d16-hard' or
    '-mfpu=fpv5-sp-d16--mfloat-abi=hard' (arduino-cli uses the raw flags)."""
    abi = FLOAT_ABI.search(name)
    fpu = FPU.search(name)
    return (fpu.group(0) if fpu else None, abi.group(1) if abi else "soft")


# The core archive each PlatformIO platform links against (see the MT_NS
# selection in maxwelltrack.h). A CPU folder is shared by several cores, so
# the right folder is the one holding a real archive for this one.
CORE_ARCHIVES = {
    "espressif32": ("mt_esp32_v3", "mt_esp32_v2"),
    "espressif8266": ("mt_esp8266_v3",),
    "ststm32": ("mt_stm32",),
    "raspberrypi": ("mt_rp2040",),
    "atmelsam": ("mt_samd",),
    "atmelavr": ("mt_avr",),
}


def has_core_archive(path, wanted):
    for name in wanted:
        a = os.path.join(path, "lib%s.a" % name)
        if os.path.isfile(a) and os.path.getsize(a) > 8:  # 8 bytes = empty stub
            return True
    return False


def find_lib_dir():
    fpu = flag("-mfpu=")
    if fpu in (None, "", "auto", "none"):
        fpu = None
    abi = flag("-mfloat-abi=") or "soft"
    platform = env.PioPlatform().name
    wanted = CORE_ARCHIVES.get(platform)

    for base in (board.get("build.mcu", ""), board.get("build.cpu", "")):
        base = os.path.join(src, base.lower())
        if not os.path.isdir(base):
            continue
        candidates = [os.path.join(base, d) for d in sorted(os.listdir(base))
                      if os.path.isdir(os.path.join(base, d))]
        candidates.append(base)
        usable = [c for c in candidates if archives(c) and (wanted is None or has_core_archive(c, wanted))]
        if not usable:
            continue
        # Prefer the folder whose float ABI matches the board, as arduino-cli does.
        for c in usable:
            if c != base and float_spec(os.path.basename(c)) == (fpu, abi):
                return c
        return base if base in usable else usable[0]
    return None


lib_dir = find_lib_dir()
if lib_dir is None:
    sys.stderr.write(
        "MaxwellTrack: no precompiled build for board '%s' (mcu %s, cpu %s)\n"
        % (env.subst("$BOARD"), board.get("build.mcu", "?"), board.get("build.cpu", "?"))
    )
    env.Exit(1)

# One archive per core; the header picks the matching namespace, so linking
# all of them is safe and the unused ones are never pulled in.
names = [os.path.basename(a)[3:-2] for a in archives(lib_dir)]
env.Append(LIBPATH=[lib_dir], LIBS=names)
