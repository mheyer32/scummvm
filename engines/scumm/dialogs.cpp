/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/events.h"
#include "common/localization.h"
#include "common/translation.h"
#include "common/ustr.h"

#include "graphics/scaler.h"

#include "gui/gui-manager.h"
#include "gui/widget.h"
#include "gui/ThemeEval.h"

#include "scumm/dialogs.h"
#include "scumm/sound.h"
#include "scumm/scumm.h"
#include "scumm/imuse/imuse.h"
#include "scumm/verbs.h"

#ifndef DISABLE_HELP
#include "scumm/help.h"
#endif

using Graphics::kTextAlignCenter;
using Graphics::kTextAlignLeft;
using GUI::WIDGET_ENABLED;

namespace Scumm {

struct ResString {
	int num;
	char string[80];
};

static const ResString string_map_table_v8[] = {
	{0, "/BT_100/Please insert disk %d. Press ENTER"},
	{0, "/BT__003/Unable to Find %s, (%s %d) Press Button."},
	{0, "/BT__004/Error reading disk %c, (%c%d) Press Button."},
	{0, "/BT__002/Game Paused.  Press SPACE to Continue."},
	{0, "/BT__005/Are you sure you want to restart?  (Y/N)"}, //BOOT.004
	{0, "/BT__006/Are you sure you want to quit?  (Y/N)"}, //BOOT.005
	{0, "/BT__008/Save"},
	{0, "/BT__009/Load"},
	{0, "/BT__010/Play"},
	{0, "/BT__011/Cancel"},
	{0, "/BT__012/Quit"},
	{0, "/BT__013/OK"},
	{0, ""},
	{0, "/BT__014/You must enter a name"},
	{0, "/BT__015/The game was NOT saved (disk full?)"},
	{0, "/BT__016/The game was NOT loaded"},
	{0, "/BT__017/Saving '%s'"},
	{0, "/BT__018/Loading '%s'"},
	{0, "/BT__019/Name your SAVE game"},
	{0, "/BT__020/Select a game to LOAD"}
};

static const ResString string_map_table_v7[] = {
	{96, "game name and version"}, //that's how it's supposed to be
	{83, "Unable to Find %s, (%c%d) Press Button."},
	{84, "Error reading disk %c, (%c%d) Press Button."},
	{85, "/BOOT.003/Game Paused.  Press SPACE to Continue."},
	{86, "/BOOT.004/Are you sure you want to restart?  (Y/N)"},
	{87, "/BOOT.005/Are you sure you want to quit?  (Y/N)"},
	{70, "/BOOT.008/Save"},
	{71, "/BOOT.009/Load"},
	{72, "/BOOT.010/Play"},
	{73, "/BOOT.011/Cancel"},
	{74, "/BOOT.012/Quit"},
	{75, "/BOOT.013/OK"},
	{0, ""},
	{78, "/BOOT.014/You must enter a name"},
	{81, "/BOOT.015/The game was NOT saved (disk full?)"},
	{82, "/BOOT.016/The game was NOT loaded"},
	{79, "/BOOT.017/Saving '%s'"},
	{80, "/BOOT.018/Loading '%s'"},
	{76, "/BOOT.019/Name your SAVE game"},
	{77, "/BOOT.020/Select a game to LOAD"}

	/* This is the complete string map for v7
	{68, "/BOOT.007/c:\\dig"},
	{69, "/BOOT.021/The Dig"},
	{70, "/BOOT.008/Save"},
	{71, "/BOOT.009/Load"},
	{72, "/BOOT.010/Play"},
	{73, "/BOOT.011/Cancel"},
	{74, "/BOOT.012/Quit"},
	{75, "/BOOT.013/OK"},
	{76, "/BOOT.019/Name your SAVE game"},
	{77, "/BOOT.020/Select a game to LOAD"},
	{78, "/BOOT.014/You must enter a name"},
	{79, "/BOOT.017/Saving '%s'"},
	{80, "/BOOT.018/Loading '%s'"},
	{81, "/BOOT.015/The game was NOT saved (disk full?)"},
	{82, "/BOOT.016/The game was NOT loaded"},
	{83, "Unable to Find %s, (%c%d) Press Button."},
	{84, "Error reading disk %c, (%c%d) Press Button."},
	{85, "/BOOT.003/Game Paused.  Press SPACE to Continue."},
	{86, "/BOOT.004/Are you sure you want to restart?  (Y/N)"},
	{87, "/BOOT.005/Are you sure you want to quit?  (Y/N)"},
	{90, "/BOOT.022/Music"},
	{91, "/BOOT.023/Voice"},
	{92, "/BOOT.024/Sfx"},
	{93, "/BOOT.025/disabled"},
	{94, "/BOOT.026/Text speed"},
	{95, "/BOOT.027/Display Text"},
	{96, "The Dig v1.0"},
	{138, "/BOOT.028/Spooled Music"),
	{139, "/BOOT.029/Do you want to replace this saved game?  (Y/N)"},
	{141, "Voice Only"},
	{142, "Voice and Text"},
	{143, "Text Display Only"}, */

};

static const ResString string_map_table_v6[] = {
	{90, "Insert Disk %c and Press Button to Continue."},
	{91, "Unable to Find %s, (%c%d) Press Button."},
	{92, "Error reading disk %c, (%c%d) Press Button."},
	{93, "Game Paused.  Press SPACE to Continue."},
	{94, "Are you sure you want to restart?  (Y/N)Y"},
	{95, "Are you sure you want to quit?  (Y/N)Y"},
	{96, "Save"},
	{97, "Load"},
	{98, "Play"},
	{99, "Cancel"},
	{100, "Quit"},
	{101, "OK"},
	{102, "Insert save/load game disk"},
	{103, "You must enter a name"},
	{104, "The game was NOT saved (disk full?)"},
	{105, "The game was NOT loaded"},
	{106, "Saving '%s'"},
	{107, "Loading '%s'"},
	{108, "Name your SAVE game"},
	{109, "Select a game to LOAD"},
	{117, "How may I serve you?"}
};

static const ResString string_map_table_v345[] = {
	{1, _s("Insert Disk %c and Press Button to Continue.")},
	{2, _s("Unable to Find %s, (%c%d) Press Button.")},
	{3, _s("Error reading disk %c, (%c%d) Press Button.")},
	{4, _s("Game Paused.  Press SPACE to Continue.")},
	// I18N: You may specify 'Yes' symbol at the end of the line, like this:
	// "Moechten Sie wirklich neu starten?  (J/N)J"
	// Will react to J as 'Yes'
	{5, _s("Are you sure you want to restart?  (Y/N)Y")},
	// I18N: you may specify 'Yes' symbol at the end of the line. See previous comment
	{6, _s("Are you sure you want to quit?  (Y/N)Y")},

	// Added in SCUMM4
	{7, _s("Save")},
	{8, _s("Load")},
	{9, _s("Play")},
	{10, _s("Cancel")},
	{11, _s("Quit")},
	{12, _s("OK")},
	{13, _s("Insert save/load game disk")},
	{14, _s("You must enter a name")},
	{15, _s("The game was NOT saved (disk full?)")},
	{16, _s("The game was NOT loaded")},
	{17, _s("Saving '%s'")},
	{18, _s("Loading '%s'")},
	{19, _s("Name your SAVE game")},
	{20, _s("Select a game to LOAD")},
	{28, _s("Game title)")}
};

#pragma mark -

#ifndef DISABLE_HELP

class HelpDialog : public ScummDialog {
public:
	HelpDialog(const GameSettings &game);
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	void reflowLayout() override;

protected:
	typedef Common::String String;

	GUI::ButtonWidget *_nextButton;
	GUI::ButtonWidget *_prevButton;

	GUI::StaticTextWidget *_title;
	GUI::StaticTextWidget *_key[HELP_NUM_LINES];
	GUI::StaticTextWidget *_dsc[HELP_NUM_LINES];

	int _page;
	int _numPages;
	int _numLines;

	const GameSettings _game;

	void displayKeyBindings();
};

#endif

#pragma mark -

ScummDialog::ScummDialog(int x, int y, int w, int h) : GUI::Dialog(x, y, w, h) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;
}

ScummDialog::ScummDialog(String name) : GUI::Dialog(name) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;
}

#pragma mark -

#ifndef DISABLE_HELP

ScummMenuDialog::ScummMenuDialog(ScummEngine *scumm)
	: MainMenuDialog(scumm) {
	_helpDialog = new HelpDialog(scumm->_game);
	_helpButton->setEnabled(true);
}

ScummMenuDialog::~ScummMenuDialog() {
	delete _helpDialog;
}

void ScummMenuDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kHelpCmd:
		_helpDialog->runModal();
		break;
	default:
		MainMenuDialog::handleCommand(sender, cmd, data);
	}
}

#pragma mark -

enum {
	kNextCmd = 'NEXT',
	kPrevCmd = 'PREV'
};

HelpDialog::HelpDialog(const GameSettings &game)
	: ScummDialog("ScummHelp"), _game(game) {
	_title = new GUI::StaticTextWidget(this, "ScummHelp.Title", Common::U32String());

	_page = 1;
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundDefault;

	_numPages = ScummHelp::numPages(_game.id);

	// I18N: Previous page button
	_prevButton = new GUI::ButtonWidget(this, "ScummHelp.Prev", _("~P~revious"), Common::U32String(), kPrevCmd);
	// I18N: Next page button
	_nextButton = new GUI::ButtonWidget(this, "ScummHelp.Next", _("~N~ext"), Common::U32String(), kNextCmd);
	new GUI::ButtonWidget(this, "ScummHelp.Close", _("~C~lose"), Common::U32String(), GUI::kCloseCmd);
	_prevButton->clearFlags(WIDGET_ENABLED);

	GUI::ContainerWidget *placeHolder = new GUI::ContainerWidget(this, "ScummHelp.HelpText");
	placeHolder->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);

	_numLines = HELP_NUM_LINES;

	// Dummy entries
	for (int i = 0; i < HELP_NUM_LINES; i++) {
		_key[i] = new GUI::StaticTextWidget(this, 0, 0, 10, 10, Common::U32String(), Graphics::kTextAlignRight);
		_dsc[i] = new GUI::StaticTextWidget(this, 0, 0, 10, 10, Common::U32String(), Graphics::kTextAlignLeft);
	}

}

void HelpDialog::reflowLayout() {
	ScummDialog::reflowLayout();

	int lineHeight = g_gui.getFontHeight();
	int16 x, y;
	int16 w, h;

	assert(lineHeight);

	g_gui.xmlEval()->getWidgetData("ScummHelp.HelpText", x, y, w, h);

	// Make sure than we don't have more lines than what we can fit
	// on the space that the layout reserves for text
	_numLines = MIN(HELP_NUM_LINES, (int)(h / lineHeight));

	int keyW = w * 20 / 100;
	int dscX = x + keyW + 32;
	int dscW = w * 80 / 100;

	int xoff = (_w >> 1) - (w >> 1);

	for (int i = 0; i < _numLines; i++) {
		_key[i]->resize(xoff + x, y + lineHeight * i, keyW, lineHeight, false);
		_dsc[i]->resize(xoff + dscX, y + lineHeight * i, dscW, lineHeight, false);
	}

	displayKeyBindings();
}

void HelpDialog::displayKeyBindings() {
	U32String titleStr, *keyStr, *dscStr;

	ScummHelp::updateStrings(_game.id, _game.version, _game.platform, _page, titleStr, keyStr, dscStr);

	_title->setLabel(titleStr);
	for (int i = 0; i < _numLines; i++) {
		_key[i]->setLabel(keyStr[i]);
		_dsc[i]->setLabel(dscStr[i]);
	}

	delete[] keyStr;
	delete[] dscStr;
}

void HelpDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {

	switch (cmd) {
	case kNextCmd:
		_page++;
		if (_page >= _numPages) {
			_nextButton->clearFlags(WIDGET_ENABLED);
		}
		if (_page >= 2) {
			_prevButton->setFlags(WIDGET_ENABLED);
		}
		displayKeyBindings();
		g_gui.scheduleTopDialogRedraw();
		break;
	case kPrevCmd:
		_page--;
		if (_page <= _numPages) {
			_nextButton->setFlags(WIDGET_ENABLED);
		}
		if (_page <= 1) {
			_prevButton->clearFlags(WIDGET_ENABLED);
		}
		displayKeyBindings();
		g_gui.scheduleTopDialogRedraw();
		break;
	default:
		ScummDialog::handleCommand(sender, cmd, data);
	}
}

#endif

#pragma mark -

static bool isCJKLanguage(Common::Language lang) {
	switch (lang) {
	case Common::KO_KOR:
	case Common::JA_JPN:
	case Common::ZH_TWN:
	case Common::ZH_CHN:
		return true;
	default:
		return false;
	}
}

InfoDialog::InfoDialog(ScummEngine *scumm, int res)
: ScummDialog(0, 0, 0, 0), _vm(scumm), _style(GUI::ThemeEngine::kFontStyleBold) { // dummy x and w

	_message = queryResString(res);

	Common::Language lang = isCJKLanguage(_vm->_language) ? _vm->_language : Common::UNK_LANG;

	// Width and height are dummy
	_text = new GUI::StaticTextWidget(this, 0, 0, 10, 10, _message, kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleBold, lang);

	// Store this for the calls to getStringWidth() and getStringHeight() in reflowLayout().
	if (lang != Common::UNK_LANG)
		_style = GUI::ThemeEngine::kFontStyleLangExtra;
}

InfoDialog::InfoDialog(ScummEngine *scumm, const U32String &message)
: ScummDialog(0, 0, 0, 0), _vm(scumm), _style(GUI::ThemeEngine::kFontStyleBold) { // dummy x and w

	_message = message;

	// Width and height are dummy
	_text = new GUI::StaticTextWidget(this, 0, 0, 10, 10, _message, kTextAlignCenter);
}

void InfoDialog::setInfoText(const U32String &message) {
	_message = message;
	_text->setLabel(_message);
	//reflowLayout(); // FIXME: Should we call this here? Depends on the usage patterns, I guess...
}

void InfoDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int width = g_gui.getStringWidth(_message, _style) + 16;
	int height = g_gui.getFontHeight(_style) + 8;

	_w = width;
	_h = height;
	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;

	_text->setSize(_w, _h);
}

const Common::U32String InfoDialog::queryResString(int stringno) {
	byte buf[256];
	byte reverseBuf[256];
	const byte *result;

	if (stringno == 0)
		return String();

	if (_vm->_game.heversion >= 80)
		return _(string_map_table_v6[stringno - 1].string);
	else if (_vm->_game.version == 8)
		result = (const byte *)string_map_table_v8[stringno - 1].string;
	else if (_vm->_game.version == 7)
		result = _vm->getStringAddressVar(string_map_table_v7[stringno - 1].num);
	else if (_vm->_game.version == 6)
		result = _vm->getStringAddressVar(string_map_table_v6[stringno - 1].num);
	else if (_vm->_game.version >= 3)
		result = _vm->getStringAddress(string_map_table_v345[stringno - 1].num);
	else
		return _(string_map_table_v345[stringno - 1].string);

	if (result && *result == '/') {
		_vm->translateText(result, buf);
		result = buf;
	}

	if (!result || *result == '\0') {	// Gracelessly degrade to english :)
		return _(string_map_table_v345[stringno - 1].string);
	}

	if (_vm->reverseIfNeeded(result, reverseBuf))
		result = reverseBuf;
	// Convert to a proper string (take care of FF codes)
	byte chr;
	String tmp;
	while ((chr = *result++)) {
		if (chr == 0xFF) {
			result += 3;
		} else if (chr != '@') {
			tmp += chr;
		}
	}

	const Common::CodePage convertFromCodePage = _vm->getDialogCodePage();
	return convertFromCodePage == Common::kCodePageInvalid ? _(tmp) : U32String(tmp, convertFromCodePage);
}

#pragma mark -

PauseDialog::PauseDialog(ScummEngine *scumm, int res)
	: InfoDialog(scumm, res) {
}

void PauseDialog::handleKeyDown(Common::KeyState state) {
	if (state.ascii == ' ')  // Close pause dialog if space key is pressed
		close();
	else
		ScummDialog::handleKeyDown(state);
}

ConfirmDialog::ConfirmDialog(ScummEngine *scumm, int res)
	: InfoDialog(scumm, res), _yesKey('y'), _noKey('n') {

	if (_message.empty())
		return;

	if (_message[_message.size() - 1] != ')') {
		_yesKey = _message[_message.size() - 1];
		_message.deleteLastChar();

		if (_yesKey >= 'A' && _yesKey <= 'Z')
			_yesKey += 'a' - 'A';

		_text->setLabel(_message);
		reflowLayout();
	}
}

void ConfirmDialog::handleKeyDown(Common::KeyState state) {
	Common::KeyCode keyYes, keyNo;

	Common::getLanguageYesNo(keyYes, keyNo);

	if (state.keycode == Common::KEYCODE_n || state.ascii == _noKey || state.ascii == keyNo) {
		setResult(0);
		close();
	} else if (state.keycode == Common::KEYCODE_y || state.ascii == _yesKey || state.ascii == keyYes) {
		setResult(1);
		close();
	} else
		ScummDialog::handleKeyDown(state);
}

#pragma mark -

ValueDisplayDialog::ValueDisplayDialog(const Common::U32String &label, int minVal, int maxVal,
		int val, uint16 incKey, uint16 decKey)
	: GUI::Dialog(0, 0, 0, 0),
	_label(label), _min(minVal), _max(maxVal),
	_value(val), _incKey(incKey), _decKey(decKey), _timer(0) {
	assert(_min <= _value && _value <= _max);
}

void ValueDisplayDialog::drawDialog(GUI::DrawLayer layerToDraw) {
	Dialog::drawDialog(layerToDraw);

	const int labelWidth = _w - 8 - _percentBarWidth;
	g_gui.theme()->drawText(Common::Rect(_x+4, _y+4, _x+labelWidth+4,
				_y+g_gui.theme()->getFontHeight()+4), _label);
	g_gui.theme()->drawSlider(Common::Rect(_x+4+labelWidth, _y+4, _x+_w-4, _y+_h-4),
				_percentBarWidth * (_value - _min) / (_max - _min));
}

void ValueDisplayDialog::handleTickle() {
	if (g_system->getMillis() > _timer) {
		close();
	}
}

void ValueDisplayDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_percentBarWidth = screenW * 100 / 640;

	int width = g_gui.getStringWidth(_label) + 16 + _percentBarWidth;
	int height = g_gui.getFontHeight() + 4 * 2;

	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;
	_w = width;
	_h = height;
}

void ValueDisplayDialog::handleKeyDown(Common::KeyState state) {
	if (state.ascii == _incKey || state.ascii == _decKey) {
		if (state.ascii == _incKey && _value < _max)
			_value++;
		else if (state.ascii == _decKey && _value > _min)
			_value--;

		setResult(_value);
		_timer = g_system->getMillis() + kDisplayDelay;
		g_gui.scheduleTopDialogRedraw();
	} else {
		close();
	}
}

void ValueDisplayDialog::open() {
	GUI::Dialog::open();
	setResult(_value);
	_timer = g_system->getMillis() + kDisplayDelay;
}

SubtitleSettingsDialog::SubtitleSettingsDialog(ScummEngine *scumm, int value)
	: InfoDialog(scumm, U32String()), _value(value), _timer(0) {

}

void SubtitleSettingsDialog::handleTickle() {
	InfoDialog::handleTickle();
	if (g_system->getMillis() > _timer)
		close();
}

void SubtitleSettingsDialog::handleKeyDown(Common::KeyState state) {
	if (state.keycode == Common::KEYCODE_t && state.hasFlags(Common::KBD_CTRL)) {
		cycleValue();

		reflowLayout();
		g_gui.scheduleTopDialogRedraw();
	} else {
		close();
	}
}

void SubtitleSettingsDialog::open() {
	cycleValue();
	InfoDialog::open();
	setResult(_value);
}

void SubtitleSettingsDialog::cycleValue() {
	static const char *const subtitleDesc[] = {
		_s("Speech Only"),
		_s("Speech and Subtitles"),
		_s("Subtitles Only")
	};

	_value += 1;
	if (_value > 2)
		_value = 0;

	if (_value == 1 && g_system->getOverlayWidth() <= 320)
		setInfoText(_c("Speech & Subs", "lowres"));
	else
		setInfoText(_(subtitleDesc[_value]));

	_timer = g_system->getMillis() + 1500;
}

Indy3IQPointsDialog::Indy3IQPointsDialog(ScummEngine *scumm, char* text)
	: InfoDialog(scumm, Common::U32String(text)) {
}

void Indy3IQPointsDialog::handleKeyDown(Common::KeyState state) {
	if (state.ascii == 'i')
		close();
	else
		ScummDialog::handleKeyDown(state);
}

DebugInputDialog::DebugInputDialog(ScummEngine *scumm, char* text)
	: InfoDialog(scumm, U32String(text)) {
	mainText = text;
	done = 0;
}

void DebugInputDialog::handleKeyDown(Common::KeyState state) {
	if (state.keycode == Common::KEYCODE_BACKSPACE && buffer.size() > 0) {
		buffer.deleteLastChar();
		Common::String total = mainText + ' ' + buffer;
		setInfoText(total);
		g_gui.scheduleTopDialogRedraw();
		reflowLayout();
	} else if (state.keycode == Common::KEYCODE_RETURN) {
		done = 1;
		close();
		return;
	} else if ((state.ascii >= '0' && state.ascii <= '9') || (state.ascii >= 'A' && state.ascii <= 'Z') || (state.ascii >= 'a' && state.ascii <= 'z') || state.ascii == '.' || state.ascii == ' ') {
		buffer += state.ascii;
		Common::String total = mainText + ' ' + buffer;
		g_gui.scheduleTopDialogRedraw();
		reflowLayout();
		setInfoText(total);
	}
}

LoomTownsDifficultyDialog::LoomTownsDifficultyDialog()
	: Dialog("LoomTownsDifficultyDialog"), _difficulty(-1) {
	GUI::StaticTextWidget *text1 = new GUI::StaticTextWidget(this, "LoomTownsDifficultyDialog.Description1", _("Select a Proficiency Level."));
	text1->setAlign(Graphics::kTextAlignCenter);
	GUI::StaticTextWidget *text2 = new GUI::StaticTextWidget(this, "LoomTownsDifficultyDialog.Description2", _("Refer to your Loom(TM) manual for help."));
	text2->setAlign(Graphics::kTextAlignCenter);

	new GUI::ButtonWidget(this, "LoomTownsDifficultyDialog.Standard", _("Standard"), Common::U32String(), kStandardCmd);
	new GUI::ButtonWidget(this, "LoomTownsDifficultyDialog.Practice", _("Practice"), Common::U32String(), kPracticeCmd);
	new GUI::ButtonWidget(this, "LoomTownsDifficultyDialog.Expert", _("Expert"), Common::U32String(), kExpertCmd);
}

void LoomTownsDifficultyDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kStandardCmd:
		_difficulty = 1;
		close();
		break;

	case kPracticeCmd:
		_difficulty = 0;
		close();
		break;

	case kExpertCmd:
		_difficulty = 2;
		close();
		break;

	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

EgaLoomOptionsWidget::EgaLoomOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "EgaLoomOptionsDialog", false, domain) {
	GUI::StaticTextWidget *text = new GUI::StaticTextWidget(widgetsBoss(), "EgaLoomOptionsDialog.OvertureTicksHeader", _("Overture Timing:"));

	text->setAlign(Graphics::TextAlign::kTextAlignEnd);

	_overtureTicksSlider = new GUI::SliderWidget(widgetsBoss(), "EgaLoomOptionsDialog.OvertureTicks", _("When using replacement music, this adjusts the time when the Overture changes to the scene with the Lucasfilm and Loom logotypes."), kOvertureTicksChanged);

	// This allows the transition in the Loom Overture to be changed by
	// 20 seconds in each direction. I think it's nice if the interval is
	// small enough that you can set the slider back to default at any
	// reasonable screen resolution.

	_overtureTicksSlider->setMinValue(-200);
	_overtureTicksSlider->setMaxValue(200);

	_overtureTicksLabel = new GUI::StaticTextWidget(widgetsBoss(), "EgaLoomOptionsDialog.OvertureTicksLabel", Common::U32String());

	_overtureTicksLabel->setFlags(GUI::WIDGET_CLEARBG);
}

void EgaLoomOptionsWidget::load() {
	int loomOvertureTicks = 0;

	if (ConfMan.hasKey("loom_overture_ticks", _domain))
		loomOvertureTicks = ConfMan.getInt("loom_overture_ticks", _domain);

	_overtureTicksSlider->setValue(loomOvertureTicks);
	updateOvertureTicksLabel();
}

bool EgaLoomOptionsWidget::save() {
	ConfMan.setInt("loom_overture_ticks", _overtureTicksSlider->getValue(), _domain);
	return true;
}

void EgaLoomOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
			.addPadding(0, 0, 16, 0)
			.addWidget("OvertureTicksHeader", "OptionsLabel")
			.addWidget("OvertureTicks", "WideSlider")
			.addWidget("OvertureTicksLabel", "ShortOptionsLabel")
		.closeLayout()
	.closeDialog();
}

void EgaLoomOptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {

	switch (cmd) {
	case kOvertureTicksChanged:
		updateOvertureTicksLabel();
		break;
	default:
		GUI::OptionsContainerWidget::handleCommand(sender, cmd, data);
		break;
	}
}

void EgaLoomOptionsWidget::updateOvertureTicksLabel() {
	int ticks = DEFAULT_LOOM_OVERTURE_TRANSITION + _overtureTicksSlider->getValue();

	_overtureTicksLabel->setLabel(Common::String::format("%d:%02d.%d", ticks / 600, (ticks % 600) / 10, ticks % 10));
}

} // End of namespace Scumm
