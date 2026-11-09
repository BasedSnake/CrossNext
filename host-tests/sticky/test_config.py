"""Check the resolved Sticky environments against the hardware they run on."""
import configparser
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
INI = configparser.ConfigParser(interpolation=None, inline_comment_prefixes=(";",))
INI.read(ROOT / "platformio.ini")
ENVS = ("sticky", "gh_release_sticky")


def option(section, key):
    if key not in INI[section]:
        return option(INI[section]["extends"], key)
    return re.sub(r"\$\{([^}.]+)\.([^}]+)\}",
                  lambda m: option(m[1], m[2]), INI[section][key])


def flags(env):
    return option("env:" + env, "build_flags").split()


class StickyConfig(unittest.TestCase):
    def test_targets_the_sticky_s3_with_its_psram_and_flash_layout(self):
        for env in ENVS:
            with self.subTest(env=env):
                section = "env:" + env
                self.assertEqual(option(section, "board_build.mcu"), "esp32s3")
                self.assertEqual(option(section, "board"), "esp32-s3-devkitc1-n16r8")
                self.assertEqual(option(section, "board_upload.flash_size"), "16MB")
                self.assertIn("-DBOARD_HAS_PSRAM", flags(env))
                devices = [f for f in flags(env) if f.startswith("-DFREEINK_DEVICE_")]
                self.assertEqual(devices, ["-DFREEINK_DEVICE_STICKY=1"])

    def test_render_stack_matches_the_x4pro_screens_it_runs(self):
        x4pro = [f for f in flags("gh_release_x4pro") if f.startswith("-DCROSSPOINT_RENDER_TASK_STACK=")]
        for env in ENVS:
            with self.subTest(env=env):
                stack = [f for f in flags(env) if f.startswith("-DCROSSPOINT_RENDER_TASK_STACK=")]
                self.assertEqual(stack, ["-DCROSSPOINT_RENDER_TASK_STACK=16384"])
                self.assertEqual(stack, x4pro)
                self.assertIn("-DEINK_DISPLAY_SINGLE_BUFFER_MODE=1", flags(env))

    def test_storage_and_usb_match_the_sticky_wiring(self):
        # SD is on the display's SPI bus, not SDMMC, and USB-C reaches UART0
        # through a WCH bridge: there is no native USB to wait for or to expose
        # as a drive.
        for env in ENVS:
            with self.subTest(env=env):
                for flag in ("-DUSE_BLOCK_DEVICE_INTERFACE=1", "-DFREEINK_CAP_USB_MSC=1",
                             "-DCROSSPOINT_WAIT_FOR_USB_SERIAL"):
                    self.assertNotIn(flag, flags(env))

    def test_release_reports_the_fork_version_and_excludes_serial_control(self):
        versions = [f for f in flags("gh_release_sticky") if f.startswith("-DCROSSPOINT_VERSION=")]
        self.assertEqual(len(versions), 1)
        self.assertEqual(versions[0].split("=", 1)[1].replace(chr(92), "").strip('"'),
                         INI["crossplay"]["version"])
        self.assertNotIn("-DCROSSPOINT_DEV_SERIAL_BRIDGE=1", flags("gh_release_sticky"))
        self.assertIn("-DCROSSPOINT_DEV_SERIAL_BRIDGE=1", flags("sticky"))

    def test_only_the_sticky_checks_the_fork_for_updates(self):
        # The fork publishes Sticky images only, so another board pointed at it
        # would find nothing; the X4 Pro and PaperMono keep upstream's default.
        fork = "https://api.github.com/repos/BasedSnake/CrossNext/releases/latest"
        for env in ENVS:
            with self.subTest(env=env):
                sources = [f for f in flags(env) if f.startswith("-DCROSSNEXT_RELEASE_URL=")]
                self.assertEqual(len(sources), 1)
                self.assertEqual(sources[0].split("=", 1)[1].replace(chr(92), "").strip('"'), fork)
        for env in ("x4pro", "gh_release_x4pro", "papermono", "gh_release_papermono"):
            with self.subTest(env=env):
                self.assertFalse(any(f.startswith("-DCROSSNEXT_RELEASE_URL=") for f in flags(env)))


if __name__ == "__main__":
    unittest.main()
