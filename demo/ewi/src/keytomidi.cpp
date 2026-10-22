/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "keytomidi.h"

namespace nap
{

	std::unique_ptr<MidiEvent> KeyToMidiConverter::processKeyEvent(KeyEvent& keyEvent)
	{
		if (rtti::isTypeMatch(keyEvent.get_type(), RTTI_OF(KeyReleaseEvent), rtti::ETypeCheck::EXACT_MATCH))
		{
			// If the key is not in pressed state we can ignore this release
			auto it = mState.find(keyEvent.mKey);
			if (it == mState.end())
				return nullptr;

			// Otherwise retrieve the note number.
			auto noteNr = it->second;
			// Erase the keypress state
			mState.erase(it);
			// If the released key is a note, return a note off.
			if (noteNr > 0)
			{
				auto result = std::make_unique<MidiEvent>(MidiEvent::Type::noteOff, noteNr, 0);
				return std::move(result);
			}
			else
				return nullptr;
		}

		if (rtti::isTypeMatch(keyEvent.get_type(), RTTI_OF(KeyPressEvent), rtti::ETypeCheck::EXACT_MATCH))
		{
            // If the key is already pressed, return
			auto it = mState.find(keyEvent.mKey);
			if (it != mState.end())
				return nullptr;

			bool isNoteKey = true;
			MidiValue noteNr;

			switch (keyEvent.mKey)
			{
				case EKeyCode::KEY_a:
					noteNr = mNoteOffset + 0;
					break;
				case EKeyCode::KEY_w:
					noteNr = mNoteOffset + 1;
					break;
				case EKeyCode::KEY_s:
					noteNr = mNoteOffset + 2;
					break;
				case EKeyCode::KEY_e:
					noteNr = mNoteOffset + 3;
					break;
				case EKeyCode::KEY_d:
					noteNr = mNoteOffset + 4;
					break;
				case EKeyCode::KEY_f:
					noteNr = mNoteOffset + 5;
					break;
				case EKeyCode::KEY_t:
					noteNr = mNoteOffset + 6;
					break;
				case EKeyCode::KEY_g:
					noteNr = mNoteOffset + 7;
					break;
				case EKeyCode::KEY_y:
					noteNr = mNoteOffset + 8;
					break;
				case EKeyCode::KEY_h:
					noteNr = mNoteOffset + 9;
					break;
				case EKeyCode::KEY_u:
					noteNr = mNoteOffset + 10;
					break;
				case EKeyCode::KEY_j:
					noteNr = mNoteOffset + 11;
					break;
				case EKeyCode::KEY_k:
					noteNr = mNoteOffset + 12;
					break;
				case EKeyCode::KEY_o:
					noteNr = mNoteOffset + 13;
					break;
				case EKeyCode::KEY_l:
					noteNr = mNoteOffset + 14;
					break;
				case EKeyCode::KEY_p:
					noteNr = mNoteOffset + 15;
					break;
				default:
					isNoteKey = false;
			}

			if (isNoteKey)
			{
				mState[keyEvent.mKey] = noteNr;
				auto result = std::make_unique<MidiEvent>(MidiEvent::Type::noteOn, noteNr, mVelocity);
				return std::move(result);
			}
			else {
				switch (keyEvent.mKey)
				{
				case EKeyCode::KEY_z:
					if (mNoteOffset > 12)
						mNoteOffset = mNoteOffset - 12;
					break;
				case EKeyCode::KEY_x:
					if (mNoteOffset < 115)
						mNoteOffset = mNoteOffset + 12;
					break;
				case EKeyCode::KEY_c:
					if (mVelocity > 5)
						mVelocity = mVelocity - 5;
					break;
				case EKeyCode::KEY_v:
					if (mVelocity < 122)
						mVelocity = mVelocity + 12;
					break;
				}
				mState[keyEvent.mKey] = 0;
			}
		}

		return nullptr;
	}

}
