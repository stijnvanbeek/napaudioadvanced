/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <inputevent.h>
#include <midievent.h>

namespace nap
{

	/**
	 * Helper class in order to emulate a midi keyboard using PC keyboard input.
	 * The object converts SDL keyboard input to midi note output.
	 * The keys "asdfghjkl" represent the white keys on a clavier.
	 * The keys "wetyuo" represent the black keys.
	 * "zx" are used to change octave, "cv" are used to change velocity up or down by 5.
	 */
	class KeyToMidiConverter
	{
	public:
		/**
		 * Call this function for each incoming KeyEvent.
		 * When the incoming key event results in a midi event being generated it is returned.
		 * @param keyEvent Incoming key event. Can be a KeyPressEvent or a KeyReleaseEvent.
		 * @return a MidiEvent id a note is generated, otherwise nullptr.
		 */
		std::unique_ptr<MidiEvent> processKeyEvent(KeyEvent& keyEvent);

	private:
		MidiValue mVelocity = 64;
		MidiValue mNoteOffset = 64;
		std::map<EKeyCode, MidiValue> mState;
	};

}