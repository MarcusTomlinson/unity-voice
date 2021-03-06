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

#include <libunityvoice/VoiceInterface.h>
#include <libunityvoice/UnityVoice.h>

#include <pulse/pulseaudio.h>
#include <libqtdbustest/QProcessDBusService.h>
#include <libqtdbustest/DBusTestRunner.h>
#include <QDebug>
#include <QSignalSpy>
#include <QtConcurrentRun>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace LibUnityVoice;

namespace {

class TestVoiceService: public Test {
protected:
	TestVoiceService() :
			mainloop(0), api(0), context(0), operation_success(false), module_index(
					PA_INVALID_INDEX ) {

		{
			QTemporaryFile temporaryFile(
					QDir(QDir::tempPath()).filePath(
							"test-voice-XXXXXX.source"));
			temporaryFile.setAutoRemove(true);
			temporaryFile.open();
			temporaryFile.close();
			devicePath = temporaryFile.fileName();
			deviceName = QFileInfo(temporaryFile).baseName();
		}

		loadPipeModule();

		dbusTestRunner.registerService(
				DBusServicePtr(
						new QProcessDBusService("com.canonical.Unity.Voice",
								QDBusConnection::SessionBus,
								UNITY_VOICE_SERVICE_BIN,
								QStringList() << deviceName)));
		dbusTestRunner.startServices();

		voice.reset(UnityVoice::getInstance());

	}

	virtual ~TestVoiceService() {
		unloadPipeModule();
	}

	void quitPulse(int ret) {
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
		TestVoiceService *self = static_cast<TestVoiceService *>(userdata);
		self->indexCallback(c, idx);
	}

	void indexCallback(pa_context *c, uint32_t idx) {
		if (idx == PA_INVALID_INDEX ) {
			qWarning() << "Pulse error: " << pa_strerror(pa_context_errno(c));
			FAIL();
		}

		module_index = idx;

		drain(c);
	}

	static void success_callback(pa_context *c, int success, void *userdata) {
		TestVoiceService *self = static_cast<TestVoiceService *>(userdata);
		self->successCallback(c, success);
	}

	void successCallback(pa_context *c, int success) {
		operation_success = success;
		module_index = PA_INVALID_INDEX;

		drain(c);
	}

	static void context_state_callback_load_module(pa_context *context,
			void *userdata) {
		TestVoiceService *self = static_cast<TestVoiceService *>(userdata);
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
			quitPulse(0);
			break;

		case PA_CONTEXT_FAILED:
		default:
			qWarning() << "PA_CONTEXT_FAILED: "
					<< pa_strerror(pa_context_errno(context));
			quitPulse(1);
			FAIL();
			break;
		}
	}

	static void context_state_callback_unload_module(pa_context *context,
			void *userdata) {
		TestVoiceService *self = static_cast<TestVoiceService *>(userdata);
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
			quitPulse(0);
			break;

		case PA_CONTEXT_FAILED:
		default:
			qWarning() << "PA_CONTEXT_FAILED:"
					<< pa_strerror(pa_context_errno(context));
			quitPulse(1);
			break;
		}
	}

	void unloadPipeModule() {
		if (module_index == PA_INVALID_INDEX ) {
			return;
		}

		mainloop = pa_mainloop_new();
		ASSERT_TRUE(mainloop != 0);

		api = pa_mainloop_get_api(mainloop);

		context = pa_context_new(api, "voice.test");
		ASSERT_TRUE(context != 0);

		pa_context_set_state_callback(context,
				context_state_callback_unload_module, this);

		ASSERT_GE(pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL),
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

		ASSERT_GE(pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL),
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

	void testSound(QSharedPointer<ComCanonicalUnityVoiceInterface> voice,
			const QList<QStringList> &commands, const QString &soundName,
			const QString &expected) {

		QSignalSpy loadingSpy(voice.data(), SIGNAL(Loading()));
		QSignalSpy listeningSpy(voice.data(), SIGNAL(Listening()));
		QSignalSpy heardSomethingSpy(voice.data(), SIGNAL(HeardSomething()));

		// Start listening
		QDBusPendingReply<QString> reply(voice->listen(commands));

		// Waiting until the service is listening
		listeningSpy.wait();
		ASSERT_FALSE(listeningSpy.empty());
		ASSERT_FALSE(loadingSpy.empty());

		// Only play the sounds when we know the service is listening
		QtConcurrent::run(TestVoiceService::playSound, soundName, devicePath);

		// We should be able to hear something before the result is available
		heardSomethingSpy.wait();
		ASSERT_FALSE(heardSomethingSpy.empty());

		QString result(reply);
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

	QSharedPointer<ComCanonicalUnityVoiceInterface> voice;
};

TEST_F(TestVoiceService, Listens) {
	QList<QStringList> commands;
	commands << (QStringList() << "auto" << "adjust");
	commands << (QStringList() << "color" << "balance");
	commands << (QStringList() << "open" << "tab");
	commands << (QStringList() << "open" << "terminal");
	commands << (QStringList() << "system" << "settings");

	testSound(voice, commands, "auto-adjust", "auto adjust");
	testSound(voice, commands, "color-balance", "color balance");
	testSound(voice, commands, "open-tab", "open tab");
	testSound(voice, commands, "open-terminal", "open terminal");
	testSound(voice, commands, "system-settings", "system settings");
}

} // namespace
