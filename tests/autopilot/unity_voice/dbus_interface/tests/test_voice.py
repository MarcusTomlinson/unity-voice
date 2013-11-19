# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2013 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

from autopilot.testcase import AutopilotTestCase
from os.path import abspath, dirname, join, isdir
from autopilot.matchers import Eventually
from testtools.matchers import Equals
import dbus
import dbus.service
import dbus.mainloop.glib
import gobject
import glob
import subprocess
import sys

from unity_voice import DBusTestCase

DBUS_NAME = "com.canonical.Unity.Voice"
DBUS_PATH = "/com/canonical/Unity/Voice"

dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

class DBusInterfaceTests(AutopilotTestCase, DBusTestCase):

    @classmethod
    def setUpClass(klass):
        klass.start_session_bus()
        klass.dbus_con = klass.get_dbus()

    def setUp(self):
        super(DBusInterfaceTests, self).setUp()
        
        self.sound_device = "/tmp/unity-voice-test.source"
        args = ["pactl", "load-module",
                "module-pipe-source",
                "source_name=unity-voice-test",
                "file=/tmp/unity-voice-test.source",
                "channels=1",
                "format=s16le",
                "rate=16000"]
        self.module_id = subprocess.check_output(args).strip()

        self.mainloop = gobject.MainLoop()
        
        self.app = self.launch_application()
        self.wait_for_bus_object(DBUS_NAME, DBUS_PATH)
        self.interface = dbus.Interface(self.dbus_con
                                   .get_object(DBUS_NAME, DBUS_PATH), DBUS_NAME)

        self.result = ""
        self.sound_file = ""

        # First try the in-source path
        self.sound_dir = abspath(join(dirname(__file__), '..', '..', '..', '..', 'data', 'sounds'))

        # Now try the install path
        if not isdir(self.sound_dir):
            self.sound_dir = join(dirname(sys.modules['unity_voice'].__file__), 'sounds')

    def application_binary(self):
        cmds = glob.glob('/usr/lib/*/unity-voice-service')
        self.assertThat(len(cmds), Equals(1))
        return cmds[0] 

    def launch_application(self):
        return subprocess.Popen([self.application_binary(), "unity-voice-test"])

    def tearDown(self):
        super(DBusInterfaceTests, self).tearDown()

        args = ["pactl", "unload-module", self.module_id]
        subprocess.check_call(args)
        
        self.mainloop.quit()
        self.app.terminate()
        
    def get_result(self):
        return self.result

    def listen(self, commands, sound_file, expected_text):
        self.sound_file = sound_file
        self.interface.connect_to_signal("Listening", self.play_sound)
        self.interface.listen(commands, reply_handler=self.listen_result,
                              error_handler=self.listen_error)
        self.mainloop.run()
        self.assertThat(self.get_result, Eventually(Equals(expected_text)))

    def copy_file(self, src, dst):
        with open(src, 'r') as fp:
            content = fp.read()
        with open(dst, 'w') as fp:
            fp.write(content)

    def play_sound(self):
        # Note: Can't use shutil.copyfile, as that doesn't work on block devices
        self.copy_file(join(self.sound_dir, self.sound_file) + ".raw", self.sound_device)
        self.copy_file(join(self.sound_dir, "silence.raw"), self.sound_device)

    def listen_result(self, result):
        self.result = result
        self.mainloop.quit()
        
    def listen_error(self, result):
        self.mainloop.quit()
        self.fail()

    def test_sounds(self):
        commands = [["auto", "adjust"], ["color", "balance"], ["open", "tab"],
                    ["open", "terminal"], ["system", "settings"]]
        
        self.listen(commands, "auto-adjust", "auto adjust");
        self.listen(commands, "color-balance", "color balance");
        self.listen(commands, "open-tab", "open tab");
        self.listen(commands, "open-terminal", "open terminal");
        self.listen(commands, "system-settings", "system settings");
