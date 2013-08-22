/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <VoiceInterface.h>

#include <pulse/pulseaudio.h>
#include <libqtdbustest/QProcessDBusService.h>
#include <libqtdbustest/DBusTestRunner.h>
#include <QDebug>
#include <QtConcurrentRun>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace com::canonical::Unity;

namespace {

class TestSphinx: public Test {
protected:
	TestSphinx() :
			mainloop(0), api(0), context(0), operation_success(false), module_index(
					PA_INVALID_INDEX ) {

		{
			QTemporaryFile file(
					QDir(QDir::tempPath()).filePath(
							"test-voice-XXXXXX.source"));
			file.setAutoRemove(true);
			file.open();
			file.close();
			devicePath = file.fileName();
			deviceName = QFileInfo(file).baseName();
		}

		loadPipeModule();

		dbusTestRunner.registerService(
				DBusServicePtr(
						new QProcessDBusService("com.canonical.Unity.Voice",
								QDBusConnection::SessionBus,
								UNITY_VOICE_SERVICE_BIN,
								QStringList() << deviceName)));
		dbusTestRunner.startServices();
	}

	virtual ~TestSphinx() {
		unloadPipeModule();
	}

	void quit_pulse(int ret) {
		ASSERT_TRUE(api != 0);
		api->quit(api, ret);
	}

	static void context_drain_complete(pa_context *c, void *userdata) {
		Q_UNUSED(userdata);
		pa_context_disconnect(c);
	}

	static void drain(pa_context *context) {
		pa_operation *o;

		if (!(o = pa_context_drain(context, context_drain_complete, NULL))) {
			pa_context_disconnect(context);
		} else {
			pa_operation_unref(o);
		}
	}

	static void index_callback(pa_context *c, uint32_t idx, void *userdata) {
		if (idx == PA_INVALID_INDEX ) {
			qWarning() << "Pulse error: " << pa_strerror(pa_context_errno(c));
		}

		TestSphinx *self = static_cast<TestSphinx *>(userdata);
		self->module_index = idx;

		drain(c);
	}

	static void success_callback(pa_context *c, int success, void *userdata) {
		TestSphinx *self = static_cast<TestSphinx *>(userdata);
		self->successCallback(c, success);

	}

	void successCallback(pa_context *c, int success) {
		operation_success = success;
		module_index = PA_INVALID_INDEX;

		drain(c);
	}

	static void context_state_callback_load_module(pa_context *context,
			void *userdata) {
		TestSphinx *self = static_cast<TestSphinx *>(userdata);
		self->contextStateCallbackLoadModule(context);
	}

	void contextStateCallbackLoadModule(pa_context *context) {
		QString formatString(
				"source_name=%1 file=%2 channels=1 format=s16le rate=16000");
		QString arguments;

		switch (pa_context_get_state(context)) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;

		case PA_CONTEXT_READY:
			arguments = formatString.arg(deviceName, devicePath);
			pa_operation_unref(
					pa_context_load_module(context, "module-pipe-source",
							arguments.toUtf8().data(), index_callback, this));
			break;

		case PA_CONTEXT_TERMINATED:
			quit_pulse(0);
			break;

		case PA_CONTEXT_FAILED:
		default:
			qWarning() << "PA_CONTEXT_FAILED: "
					<< pa_strerror(pa_context_errno(context));
			quit_pulse(1);
			break;
		}
	}

	static void context_state_callback_unload_module(pa_context *context,
			void *userdata) {
		TestSphinx *self = static_cast<TestSphinx *>(userdata);
		self->contextStateCallbackUnloadModule(context);
	}

	void contextStateCallbackUnloadModule(pa_context *context) {
		switch (pa_context_get_state(context)) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;

		case PA_CONTEXT_READY:
			pa_operation_unref(
					pa_context_unload_module(context, module_index,
							success_callback, this));
			break;

		case PA_CONTEXT_TERMINATED:
			quit_pulse(0);
			break;

		case PA_CONTEXT_FAILED:
		default:
			qWarning() << "PA_CONTEXT_FAILED:"
					<< pa_strerror(pa_context_errno(context));
			quit_pulse(1);
			break;
		}
	}

	void unloadPipeModule() {
		mainloop = pa_mainloop_new();
		ASSERT_TRUE(mainloop != 0);

		api = pa_mainloop_get_api(mainloop);

		context = pa_context_new(api, "voice.test");
		ASSERT_TRUE(context != 0);

		pa_context_set_state_callback(context,
				context_state_callback_unload_module, this);

		ASSERT_GE(pa_context_connect (context, NULL, PA_CONTEXT_NOFLAGS, NULL ),
				0);

		int ret = 0;
		ASSERT_GE(pa_mainloop_run(mainloop, &ret), 0);
		ASSERT_EQ(PA_INVALID_INDEX, module_index);
		ASSERT_TRUE(operation_success);

		if (context) {
			pa_context_unref(context);
		}

		if (mainloop) {
			pa_signal_done();
			pa_mainloop_free(mainloop);
		}
	}

	void loadPipeModule() {
		mainloop = pa_mainloop_new();
		ASSERT_TRUE(mainloop != 0);

		api = pa_mainloop_get_api(mainloop);

		context = pa_context_new(api, "voice.test");
		ASSERT_TRUE(context != 0);

		pa_context_set_state_callback(context,
				context_state_callback_load_module, this);

		ASSERT_GE(pa_context_connect (context, NULL, PA_CONTEXT_NOFLAGS, NULL ),
				0);

		int ret = 0;
		ASSERT_GE(pa_mainloop_run(mainloop, &ret), 0);
		ASSERT_NE(PA_INVALID_INDEX, module_index);

		if (context) {
			pa_context_unref(context);
		}

		if (mainloop) {
			pa_signal_done();
			pa_mainloop_free(mainloop);
		}
	}

	static void playSound(const QString &soundName, const QString &devicePath) {
		// FIXME wait until voice service is listening
		QThread::msleep(300);

		QDir soundsDir(SOUNDS_DIR);

		QFile soundFile(soundsDir.filePath(soundName).append(".raw"));
		QFile silenceFile(soundsDir.filePath("silence.raw"));
		QFile deviceFile(devicePath);

		ASSERT_TRUE(soundFile.open(QIODevice::ReadOnly));
		ASSERT_TRUE(silenceFile.open(QIODevice::ReadOnly));
		ASSERT_TRUE(deviceFile.open(QIODevice::WriteOnly));

		deviceFile.write(soundFile.readAll());
		deviceFile.write(silenceFile.readAll());

		soundFile.close();
		silenceFile.close();
		deviceFile.close();
	}

	void testSound(Voice &voice, const QList<QStringList> &commands,
			const QString &soundName, const QString &expected) {

		QtConcurrent::run(TestSphinx::playSound, soundName, devicePath);
		QString result(voice.listen(commands));
		EXPECT_EQ(expected.toStdString(), result.toStdString());
	}

	DBusTestRunner dbusTestRunner;

	pa_mainloop *mainloop;
	pa_mainloop_api *api;
	pa_context *context;
	bool operation_success;
	int module_index;

	QString deviceName;
	QString devicePath;
};

TEST_F(TestSphinx, Listens) {
	QList<QStringList> commands;

	commands << (QStringList() << "auto" << "adjust");
	commands << (QStringList() << "color" << "balance");
	commands << (QStringList() << "open" << "tab");
	commands << (QStringList() << "open" << "terminal");
	commands << (QStringList() << "system" << "settings");

	Voice voice("com.canonical.Unity.Voice", "/com/canonical/Unity/Voice",
			dbusTestRunner.sessionConnection());

	testSound(voice, commands, "auto-adjust", "auto adjust");
	testSound(voice, commands, "color-balance", "color balance");
	testSound(voice, commands, "open-tab", "open tab");
	testSound(voice, commands, "open-terminal", "open terminal");
	testSound(voice, commands, "system-settings", "system settings");
}

} // namespace
