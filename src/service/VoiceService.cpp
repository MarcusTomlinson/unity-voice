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

#include <VoiceService.h>
#include <VoiceAdaptor.h>
#include <config.h>

#include <stdexcept>

using namespace std;

arg_t VoiceService::sphinx_cmd_ln[] = { POCKETSPHINX_OPTIONS, { "-adcdev",
		ARG_STRING, NULL, "Name of audio device to use for input." },
		CMDLN_EMPTY_OPTION };

VoiceService::VoiceService(const QDBusConnection &connection,
		const char *deviceName, QObject *parent) :
		QObject(parent), m_adaptor(new VoiceAdaptor(this)), m_connection(
				connection) {

	if (deviceName != nullptr) {
		config = cmd_ln_init(nullptr, sphinx_cmd_ln, TRUE, "-hmm", HMM_PATH, "-dict",
				DICT_PATH, "-adcdev", deviceName, nullptr);
	} else {
		config = cmd_ln_init(nullptr, sphinx_cmd_ln, TRUE, "-hmm", HMM_PATH, "-dict",
				DICT_PATH, nullptr);
	}

	if (config == NULL) {
		throw logic_error("Sphinx command line arguments failed to initialize");
	}

	ps = ps_init(config);
	if (ps == NULL) {
		throw logic_error("Unable to initialize Sphinx decoder");
	}

	if (!m_dict.loadDictionary(DICT_PATH)) {
		throw logic_error("Unable to initialize pronunciations dictionary");
	}

	m_connection.registerObject("/com/canonical/Unity/Voice", this);
}

VoiceService::~VoiceService() {
	m_connection.unregisterObject("/com/canonical/unity/Voice");

	if (ps) {
		ps_free(ps);
	}
}

QString VoiceService::utteranceLoop() {
	ad_rec_t *ad;
	int16 adbuf[4096];
	int32 k, ts, rem;
	char const *hyp;
	char const *uttid;
	cont_ad_t *cont;

	if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),
			(int) cmd_ln_float32_r(config, "-samprate"))) == NULL) {
		sendErrorReply(QDBusError::Failed, "Failed to open audio device");
		return QString();
	}

	// Initialize continuous listening module
	if ((cont = cont_ad_init(ad, ad_read)) == NULL) {
		sendErrorReply(QDBusError::Failed,
				"Failed to initialize voice activity detection");
		return QString();
	}
	if (ad_start_rec(ad) < 0) {
		sendErrorReply(QDBusError::Failed, "Failed to start recording");
		return QString();
	}

	// Indicate listening for next utterance
	qDebug() << "Voice query is listening";
	m_adaptor->Listening();

	int attempts = 0;
	/* Wait data for next utterance */
	while ((k = cont_ad_read(cont, adbuf, 4096)) == 0) {
		++attempts;
		if (attempts == 100) {
			break;
		}
		QThread::msleep(100);
	}

	if (k == 0) {
		sendErrorReply(QDBusError::Failed, "Nothing was heard");
		return QString();
	}

	if (k < 0) {
		sendErrorReply(QDBusError::Failed, "Failed to read audio");
		return QString();
	}

	/*
	 * Non-zero amount of data received; start recognition of new utterance.
	 * NULL argument to uttproc_begin_utt => automatic generation of utterance-id.
	 */
	if (ps_start_utt(ps, NULL) < 0) {
		sendErrorReply(QDBusError::Failed, "Failed to start utterance");
		return QString();
	}

	qDebug() << "Voice query has heard something";
	m_adaptor->HeardSomething();

	ps_process_raw(ps, adbuf, k, FALSE, FALSE);

	/* Note timestamp for this first block of data */
	ts = cont->read_ts;

	/* Decode utterance until end (marked by a "long" silence, >1sec) */
	for (;;) {
		/* Read non-silence audio data, if any, from continuous listening module */
		if ((k = cont_ad_read(cont, adbuf, 4096)) < 0) {
			sendErrorReply(QDBusError::Failed, "Failed to read audio");
			return QString();
		}
		if (k == 0) {
			/*
			 * No speech data available; check current timestamp with most recent
			 * speech to see if more than 1 sec elapsed.  If so, end of utterance.
			 */
			if ((cont->read_ts - ts) > DEFAULT_SAMPLES_PER_SEC)
				break;
		} else {
			/* New speech data received; note current timestamp */
			ts = cont->read_ts;

			/* Check for timeout */
			if ((cont->read_ts - ts) > DEFAULT_SAMPLES_PER_SEC * 30) {
				sendErrorReply(QDBusError::Failed, "Nothing was heard");
				return QString();
			}
		}

		/*
		 * Decode whatever data was read above.
		 */
		rem = ps_process_raw(ps, adbuf, k, FALSE, FALSE);

		/* If no work to be done, sleep a bit */
		if ((rem == 0) && (k == 0))
			QThread::msleep(20);
	}

	/*
	 * Utterance ended; flush any accumulated, unprocessed A/D data and stop
	 * listening until current utterance completely decoded
	 */
	ad_stop_rec(ad);
	while (ad_read(ad, adbuf, 4096) >= 0)
		;
	cont_ad_reset(cont);

	qDebug() << "Voice query has stopped listening, processing...";
	fflush(stdout);
	/* Finish decoding, obtain and print result */
	ps_end_utt(ps);
	hyp = ps_get_hyp(ps, NULL, &uttid);
	fflush(stdout);

	cont_ad_close(cont);
	ad_close(ad);

	if (hyp) {
		return QString::fromUtf8(hyp);
	} else {
		return QString();
	}
}

QString VoiceService::sphinxListen(fsg_model_t* fsg) {
	// Get the fsg set or create one if none
	fsg_set_t *fsgs = ps_get_fsgset(ps);
	if (fsgs == NULL)
		fsgs = ps_update_fsgset(ps);

	// Remove the old fsg
	fsg_model_t * old_fsg = fsg_set_get_fsg(fsgs, fsg_model_name(fsg));
	if (old_fsg) {
		fsg_set_remove(fsgs, old_fsg);
		fsg_model_free(old_fsg);
	}

	// Add the new fsg
	fsg_set_add(fsgs, fsg_model_name(fsg), fsg);
	fsg_set_select(fsgs, fsg_model_name(fsg));

	ps_update_fsgset(ps);

	return utteranceLoop();
}

int VoiceService::calculateNumberOfStates(const QList<QStringList> &commands) {
	int number_of_states = 0;

	for (const QStringList &command : commands) {
		number_of_states += command.size();
	}

	// the number of states calculated above doesn't include the start and end
	return number_of_states + 2;
}

int VoiceService::writeCommand(fsg_model_t *fsg, const QStringList &command,
		int stateNum, float commandProbability) {
	// the first transition goes from the state 0
	// it's probability depends on how many commands there are
	if (!command.isEmpty()) {
		const QString &word = command.first();
		QString lower = word.toLower();

		if (m_dict.contains(lower)) {
			int wid = fsg_model_word_add(fsg, lower.toUtf8().data());
			fsg_model_trans_add(fsg, 0, stateNum + 1, commandProbability, wid);
		}
		++stateNum;
	}

	// the rest of the transitions are certain (straight path)
	// so have probability 1.0
	auto word(command.constBegin());
	++word;
	for (; word != command.constEnd(); ++word) {
		QString lower = word->toLower();

		if (m_dict.contains(lower)) {
			int wid = fsg_model_word_add(fsg, lower.toUtf8().data());
			fsg_model_trans_add(fsg, stateNum, stateNum + 1, 1.0, wid);
		}
		++stateNum;
	}

	// null transition to exit state
	fsg_model_null_trans_add(fsg, stateNum, 1, 0);

	return stateNum;
}

fsg_model_t* VoiceService::buildGrammar(const QList<QStringList> &commands) {
	int numberOfStates = calculateNumberOfStates(commands);
	float commandProbability = 1.0f / commands.size();

	qDebug() << "Number of states:" << numberOfStates;

	fsg_model_t *fsg = fsg_model_init("<unity-voice.GRAM>", ps_get_logmath(ps),
			cmd_ln_float32_r(config, "-lw"), numberOfStates);

	if (fsg == NULL) {
		sendErrorReply(QDBusError::Failed,
				"Could not build Sphinx grammar. Is sphinx-voxforge installed?");
		return NULL;
	}

	fsg->start_state = 0;
	fsg->final_state = 1;

	// starting at state 2 (0 is start and 1 is exit)
	int stateNum = 1;
	for (const QStringList &command : commands) {
		// keep a record of the number of states so far
		stateNum = writeCommand(fsg, command, stateNum, commandProbability);
	}

	glist_t nulls = fsg_model_null_trans_closure(fsg, 0);
	glist_free(nulls);

	return fsg;
}

QString VoiceService::listen(const QList<QStringList> &commands) {
	if (commands.isEmpty()) {
		return QString();
	}

	m_adaptor->Loading();
	fsg_model_t *fsg = buildGrammar(commands);

	if (fsg == NULL) {
		return QString();
	}

	return sphinxListen(fsg);
}
