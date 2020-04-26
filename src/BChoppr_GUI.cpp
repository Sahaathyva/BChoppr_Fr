/* B.Choppr
 * Step Sequencer Effect Plugin
 *
 * Copyright (C) 2018, 2019 by Sven Jähnichen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "BChoppr_GUI.hpp"
#include "Ports.hpp"
#include "screen.h"


BChoppr_GUI::BChoppr_GUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeWindow parentWindow) :
	Window (760, 560, "B.Choppr", parentWindow, true),
	controller (NULL), write_function (NULL),

	mContainer (0, 0, 760, 560, "main"),
	rContainer (260, 80, 480, 360, "rcontainer"),
	sContainer (3, 220, 474, 137, "scontainer"),
	monitorSwitch (600, 15, 40, 16, "switch", 0.0),
	monitorLabel (600, 35, 40, 20, "smlabel", "Monitor"),
	bypassButton (662, 15, 16, 16, "redbutton"),
	bypassLabel (650, 35, 40, 20, "smlabel", "Bypass"),
	drywetDial (703, 5, 33, 40, "dial", 1.0, 0.0, 1.0, 0.0, "%1.2f"),
	drywetLabel (700, 35, 40, 20, "smlabel", "Dry/wet"),
	helpButton (20, 80, 24, 24, "halobutton", "Help"),
	ytButton (50, 80, 24, 24, "halobutton", "Tutorial"),
	monitorDisplay (3, 3, 474, 217, "mmonitor"),
	rectButton (40, 250, 60, 40, "abutton"),
	sinButton (140, 250, 60, 40, "nbutton"),
	stepshapeDisplay (30, 300, 180, 140, "smonitor"),
	attackControl (40, 465, 50, 60, "dial", 0.2, 0.01, 1.0, 0.01, "%1.2f"),
	attackLabel (20, 520, 90, 20, "label", "Attack"),
	releaseControl (150, 465, 50, 60, "dial", 0.2, 0.01, 1.0, -0.01, "%1.2f"),
	releaseLabel (130, 520, 90, 20, "label", "Decay"),
	sequencesperbarControl (260, 442, 120, 28, "slider", 1.0, 1.0, 8.0, 1.0, "%1.0f"),
	sequencesperbarLabel (260, 470, 120, 20, "label", "Sequences per bar"),
	swingControl (460, 442, 120, 28, "slider", 1.0, 1.0 / 3.0, 3.0, 0.0),
	swingLabel (460, 470, 120, 20, "label", "Steps swing"),
	markersAutoButton (655, 450, 80, 20, "button", "Auto"),
	markersAutoLabel (655, 470, 80, 20, "label", "Markers"),
	nrStepsControl (260, 492, 480, 28, "slider", 1.0, 1.0, MAXSTEPS, 1.0, "%2.0f"),
	nrStepsLabel (260, 520, 480, 20, "label", "Number of steps"),
	stepshapeLabel (33, 303, 80, 20, "label", "Step shape"),
	sequencemonitorLabel (263, 83, 120, 20, "label", "Sequence monitor"),
	messageLabel (420, 83, 280, 20, "hilabel", ""),
	markerListBox (12, -68, 86, 66, "listbox", BItems::ItemList ({"Auto", "Manual"})),

	surface (NULL), cr1 (NULL), cr2 (NULL), cr3 (NULL), cr4 (NULL), pat1 (NULL), pat2 (NULL), pat3 (NULL), pat4 (NULL), pat5 (NULL),
	pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")),  sz (1.0), bgImageSurface (nullptr),
	scale (DB_CO(0.0)), bypass (0.0f), drywet (1.0f),
	blend (1), attack (0.2), release (0.2), nrSteps (16.0), sequencesperbar (4.0), swing (1.0),
	map (NULL)

{
	if (!init_mainMonitor () || !init_Stepshape ())
	{
		std::cerr << "BChoppr.lv2#GUI: Failed to init monitor." <<  std::endl;
		destroy_mainMonitor ();
		destroy_Stepshape ();
		throw std::bad_alloc ();
	}

	// Set callbacks
	monitorSwitch.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
	bypassButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
	drywetDial.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
	attackControl.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
	releaseControl.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
	sequencesperbarControl.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
	swingControl.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
	nrStepsControl.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
	monitorDisplay.setCallbackFunction (BEvents::EventType::WHEEL_SCROLL_EVENT, BChoppr_GUI::monitorScrolledCallback);
	monitorDisplay.setCallbackFunction (BEvents::EventType::POINTER_DRAG_EVENT, BChoppr_GUI::monitorDraggedCallback);
	markerListBox.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::listBoxChangedCallback);
	markersAutoButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::markersAutoClickedCallback);
	rectButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BChoppr_GUI::buttonClickedCallback);
	sinButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BChoppr_GUI::buttonClickedCallback);
	helpButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, helpButtonClickedCallback);
	ytButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, ytButtonClickedCallback);

	// Configure widgets
	bgImageSurface = cairo_image_surface_create_from_png ((pluginPath + BG_FILE).c_str());
	widgetBg.loadFillFromCairoSurface (bgImageSurface);
	drywetDial.setScrollable (true);
	drywetDial.setHardChangeable (false);
	attackControl.setScrollable (true);
	attackControl.setHardChangeable (false);
	releaseControl.setScrollable (true);
	releaseControl.setHardChangeable (false);
	sequencesperbarControl.setScrollable (true);
	swingControl.setHardChangeable (false);
	nrStepsControl.setScrollable (true);
	monitorDisplay.setScrollable (true);
	monitorDisplay.setDraggable (true);
	markerListBox.setOversize (true);
	applyTheme (theme);

	//Initialialize and configure stepControllers
	double sw = sContainer.getEffectiveWidth();
	double sx = sContainer.getXOffset();
	for (int i = 0; i < MAXSTEPS; ++i)
	{
		stepControl[i] = BWidgets::VSliderValue ((i + 0.5) * sw / MAXSTEPS + sx - 14, 40, 28, 100, "slider", 1.0, 0.0, 1.0, 0.01, "%1.2f");
		stepControl[i].setHardChangeable (false);
		stepControl[i].setScrollable (true);
		stepControl[i].setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
		stepControl[i].applyTheme (theme, "slider");
		stepControl[i].getDisplayLabel ()->setState (BColors::ACTIVE);
		sContainer.add (stepControl[i]);
	}

	//Initialialize and configure markers
	for (int i = 0; i < MAXSTEPS - 1; ++i)
	{
		markerWidgets[i] = Marker ((i + 1) * sw / MAXSTEPS + sx - 5, 10, 10, 16, "marker", (double(i) + 1.0) / MAXSTEPS, 0.0, 1.0, 0.0);
		markerWidgets[i].setHasValue (false);
		markerWidgets[i].setDraggable (true);
		markerWidgets[i].setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BChoppr_GUI::markerClickedCallback);
		markerWidgets[i].setCallbackFunction (BEvents::EventType::POINTER_DRAG_EVENT, BChoppr_GUI::markerDraggedCallback);
		markerWidgets[i].setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
		markerWidgets[i].applyTheme (theme, "slider");
		sContainer.add (markerWidgets[i]);
	}

	setAutoMarkers ();
	rearrange_controllers ();
	redrawMainMonitor ();
	redrawSContainer ();
	redrawButtons ();

	// Pack widgets
	mContainer.add (rContainer);
	rContainer.add (monitorDisplay);
	rContainer.add (sContainer);
	mContainer.add (monitorSwitch);
	mContainer.add (monitorLabel);
	mContainer.add (bypassButton);
	mContainer.add (bypassLabel);
	mContainer.add (drywetDial);
	mContainer.add (drywetLabel);
	mContainer.add (helpButton);
	mContainer.add (ytButton);
	mContainer.add (rectButton);
	mContainer.add (sinButton);
	mContainer.add (stepshapeDisplay);
	mContainer.add (attackControl);
	mContainer.add (attackLabel);
	mContainer.add (releaseControl);
	mContainer.add (releaseLabel);
	mContainer.add (sequencesperbarControl);
	mContainer.add (sequencesperbarLabel);
	mContainer.add (swingControl);
	mContainer.add (swingLabel);
	mContainer.add (markersAutoButton);
	mContainer.add (markersAutoLabel);
	mContainer.add (nrStepsControl);
	mContainer.add (nrStepsLabel);
	mContainer.add (stepshapeLabel);
	mContainer.add (sequencemonitorLabel);
	mContainer.add (messageLabel);
	add (mContainer);

	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0) m = (LV2_URID_Map*) features[i]->data;
	}
	if (!m) throw std::invalid_argument ("Host does not support urid:map");

	//Map URIS
	map = m;
	getURIs (map, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge,map);
}

BChoppr_GUI::~BChoppr_GUI()
{
	send_record_off ();
	destroy_mainMonitor ();
	destroy_Stepshape ();
}

void BChoppr_GUI::portEvent(uint32_t port_index, uint32_t buffer_size, uint32_t format, const void* buffer)
{
	// Notify port
	if ((format == uris.atom_eventTransfer) && (port_index == Notify))
	{
		const LV2_Atom* atom = (const LV2_Atom*) buffer;
		if ((atom->type == uris.atom_Blank) || (atom->type == uris.atom_Object))
		{
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*) atom;

			// Monitor notification
			if (obj->body.otype == uris.notify_event)
			{
				const LV2_Atom* data = NULL;
				lv2_atom_object_get(obj, uris.notify_key, &data, 0);
				if (data && (data->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) data;
					if (vec->body.child_type == uris.atom_Float)
					{
						uint32_t notificationsCount = (uint32_t) ((data->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (BChopprNotifications));
						BChopprNotifications* notifications = (BChopprNotifications*) (&vec->body + 1);
						if (notificationsCount > 0)
						{
							add_monitor_data (notifications, notificationsCount, mainMonitor.horizonPos);
							redrawMainMonitor ();
						}
					}
				}
				else std::cerr << "BChoppr.lv2#GUI: Corrupt audio message." << std::endl;
			}

			// Message notification
			else if (obj->body.otype == uris.notify_messageEvent)
			{
				const LV2_Atom* data = NULL;
				lv2_atom_object_get(obj, uris.notify_message, &data, 0);
				if (data && (data->type == uris.atom_Int))
				{
					const int messageNr = ((LV2_Atom_Int*)data)->body;
					std::string msg = ((messageNr >= NO_MSG) && (messageNr <= MAX_MSG) ? messageStrings[messageNr] : "");
					messageLabel.setText (msg);
				}
			}
		}
	}

	// Scan remaining ports
	else if ((format == 0) && (port_index >= Controllers) && (port_index < Controllers + NrControllers))
	{
	float* pval = (float*) buffer;
	switch (port_index) {
		case Bypass:
			bypass = LIM (*pval, 0.0f, 1.0f);
			bypassButton.setValue (*pval);
			break;
		case DryWet:
			drywet = LIM (*pval, 0.0f, 1.0f);
			drywetDial.setValue (*pval);
			break;
		case Blend:
			{
				float* pval = (float*) buffer;
				blend = LIM (*pval, 1.0f, 2.0f);
				if (blend == 1) {rectButton.rename ("abutton"); sinButton.rename ("nbutton");}
				if (blend == 2) {sinButton.rename ("abutton"); rectButton.rename ("nbutton");}
				rectButton.applyTheme (theme);
				sinButton.applyTheme (theme);
				redrawButtons ();
				redrawStepshape ();
			}
			break;
		case Attack:
			attack = *pval;
			attackControl.setValue (*pval);
			break;
		case Release:
			release = *pval;
			releaseControl.setValue (*pval);
			break;
		case SequencesPerBar:
			sequencesperbar = *pval;
			sequencesperbarControl.setValue (*pval);
			break;
		case Swing:
			swing = *pval;
			swingControl.setValue (*pval);
			break;
		case NrSteps:
			if (nrSteps != *pval)
			{
				nrSteps = *pval;
				rearrange_controllers ();
			}
			redrawMainMonitor ();
			nrStepsControl.setValue (*pval);
			break;
		default:
			if ((port_index >= StepPositions) and (port_index < StepPositions + MAXSTEPS - 1))
			{
				setMarker (port_index-StepPositions, *pval);
				setAutoMarkers ();
				rearrange_controllers ();
				redrawSContainer ();
				redrawMainMonitor ();
			}

			else if ((port_index >= StepLevels) and (port_index < StepLevels + MAXSTEPS))
			{
				stepControl[port_index-StepLevels].setValue (*pval);
			}
		}
	}

}

void BChoppr_GUI::resizeGUI()
{
	hide ();

	// Resize Fonts
	defaultFont.setFontSize (12 * sz);
	smFont.setFontSize (8 * sz);

	// Resize Background
	cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 760 * sz, 560 * sz);
	cairo_t* cr = cairo_create (surface);
	cairo_scale (cr, sz, sz);
	cairo_set_source_surface(cr, bgImageSurface, 0, 0);
	cairo_paint(cr);
	widgetBg.loadFillFromCairoSurface(surface);
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	// Resize widgets
	RESIZE (mContainer, 0, 0, 760, 560, sz);
	RESIZE (rContainer, 260, 80, 480, 360, sz);
	RESIZE (monitorSwitch, 600, 15, 40, 16, sz);
	RESIZE (monitorLabel, 600, 35, 40, 20, sz);
	RESIZE (bypassButton, 662, 15, 16, 16, sz);
	RESIZE (bypassLabel, 650, 35, 40, 20, sz);
	RESIZE (drywetDial, 703, 5, 33, 40, sz);
	RESIZE (drywetLabel, 700, 35, 40, 20, sz);
	RESIZE (helpButton, 20, 80, 24, 24, sz);
	RESIZE (ytButton, 50, 80, 24, 24, sz);
	RESIZE (monitorDisplay, 3, 3, 474, 217, sz);
	RESIZE (rectButton, 40, 250, 60, 40, sz);
	RESIZE (sinButton, 140, 250, 60, 40, sz);
	RESIZE (stepshapeDisplay, 30, 300, 180, 140, sz);
	RESIZE (attackControl, 40, 465, 50, 60, sz);
	RESIZE (attackLabel, 20, 520, 90, 20, sz);
	RESIZE (releaseControl, 150, 465, 50, 60, sz);
	RESIZE (releaseLabel, 130, 520, 90, 20, sz);
	RESIZE (sequencesperbarControl, 260, 442, 120, 28, sz);
	RESIZE (sequencesperbarLabel, 260, 470, 120, 20, sz);
	RESIZE (swingControl, 460, 442, 120, 28, sz);
	RESIZE (swingLabel, 460, 470, 120, 20, sz);
	RESIZE (markersAutoButton, 655, 450, 80, 20, sz);
	RESIZE (markersAutoLabel, 655, 470, 80, 20, sz);
	RESIZE (nrStepsControl, 260, 492, 480, 28, sz);
	RESIZE (nrStepsLabel, 260, 520, 480, 20, sz);
	RESIZE (stepshapeLabel, 33, 303, 80, 20, sz);
	RESIZE (sequencemonitorLabel, 263, 83, 120, 20, sz);
	RESIZE (messageLabel, 420, 83, 280, 20,sz);
	RESIZE (sContainer, 3, 220, 474, 137, sz);
	RESIZE (markerListBox, 12, -68, 86, 66, sz);
	markerListBox.resizeItems (BUtilities::Point (80 * sz, 20 * sz));

	double sw = sContainer.getEffectiveWidth();
	double sx = sContainer.getXOffset();
	for (int i = 0; i < MAXSTEPS; ++i) {RESIZE (stepControl[i], (i + 0.5) * sw / sz / nrSteps + sx / sz - 14, 40, 28, 100, sz);}
	for (int i = 0; i < MAXSTEPS - 1; ++i) {RESIZE (markerWidgets[i], markerWidgets[i].getValue() * sw / sz + sx / sz - 5, 10, 10, 16, sz);}

	// Update monitors
	destroy_Stepshape ();
	init_Stepshape ();
	redrawStepshape ();
	destroy_mainMonitor ();
	init_mainMonitor ();
	redrawMainMonitor ();
	redrawSContainer ();
	redrawButtons ();

	// Apply changes
	applyTheme (theme);
	show ();
}

void BChoppr_GUI::applyTheme (BStyles::Theme& theme)
{
	mContainer.applyTheme (theme);
	rContainer.applyTheme (theme);
	monitorSwitch.applyTheme (theme);
	monitorLabel.applyTheme (theme);
	bypassButton.applyTheme (theme);
	bypassLabel.applyTheme (theme);
	drywetDial.applyTheme (theme);
	drywetLabel.applyTheme (theme);
	helpButton.applyTheme (theme);
	ytButton.applyTheme (theme);
	monitorDisplay.applyTheme (theme);
	rectButton.applyTheme (theme);
	sinButton.applyTheme (theme);
	stepshapeDisplay.applyTheme (theme);
	attackControl.applyTheme (theme);
	attackLabel.applyTheme (theme);
	releaseControl.applyTheme (theme);
	releaseLabel.applyTheme (theme);
	sequencesperbarControl.applyTheme (theme);
	sequencesperbarLabel.applyTheme (theme);
	swingControl.applyTheme (theme);
	swingLabel.applyTheme (theme);
	markersAutoButton.applyTheme (theme);
	markersAutoLabel.applyTheme (theme);
	nrStepsControl.applyTheme (theme);
	nrStepsLabel.applyTheme (theme);
	stepshapeLabel.applyTheme (theme);
	sequencemonitorLabel.applyTheme (theme);
	messageLabel.applyTheme (theme);
	sContainer.applyTheme (theme);
	markerListBox.applyTheme (theme);
	for (int i = 0; i < MAXSTEPS; ++i)
	{
		stepControl[i].applyTheme (theme);
		stepControl[i].update ();	// TODO Remove if fixed in BWidgets TK
	}
}

void BChoppr_GUI::onConfigureRequest (BEvents::ExposeEvent* event)
{
	Window::onConfigureRequest (event);

	sz = (getWidth() / 760 > getHeight() / 560 ? getHeight() / 560 : getWidth() / 760);
	resizeGUI ();
}

void BChoppr_GUI::send_record_on ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.ui_on);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, Control_2, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
	monitorSwitch.setValue (1.0);
}

void BChoppr_GUI::send_record_off ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.ui_off);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, Control_2, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
	monitorSwitch.setValue (0.0);
}

void BChoppr_GUI::setMarker (const int markerNr, double value)
{
	if ((markerNr < 0) || (markerNr >= MAXSTEPS - 1)) return;

	// Value 0.0: Automatic
	if (value == 0.0)
	{
		markerWidgets[markerNr].setHasValue (false);
	}

	else
	{
		// Set value and switch off automatic
		value = LIMIT (value, MINMARKERVALUE, 1.0);
		markerWidgets[markerNr].setHasValue (true);
		markerWidgets[markerNr].setValue (value);

		// Validate ancessors
		for (int i = markerNr - 1; i >= 0; --i)
		{
			if (markerWidgets[i].hasValue())
			{
				if (markerWidgets[i].getValue() > value) markerWidgets[i].setValue (value);
				else break;
			}
		}

		// Validate successors
		for (int i = markerNr + 1; i < MAXSTEPS - 1; ++i)
		{
			if (markerWidgets[i].hasValue())
			{
				if (markerWidgets[i].getValue() < value) markerWidgets[i].setValue (value);
				else break;
			}
		}
	}
}

void BChoppr_GUI::setAutoMarkers ()
{
	int nrMarkers = nrSteps - 1;
	int start = 0;
	for (int i = 0; i < nrMarkers; ++i)
	{
		if (!markerWidgets[i].hasValue())
		{
			if ((i == nrMarkers - 1) || (markerWidgets[i + 1].hasValue()))
			{
				double swing = 2.0 * swingControl.getValue() / (swingControl.getValue() + 1.0);
				double anc = (start == 0 ? 0 : markerWidgets[start - 1].getValue());
				double suc = (i == nrMarkers - 1 ? 1 : markerWidgets[i + 1].getValue());
				double diff = suc - anc;
				double dist = i - start + 1.0 + (int (i - start) & 1 ? ((start & 1) ? 2.0 - swing : swing) : 1.0);
				double step = (diff < 0 ? 0 : diff / dist);
				for (int j = start; j <= i; ++j)
				{
					double f = ((j & 1) ? 2.0 - swing : swing);
					anc += f * step;
					markerWidgets[j].setValue (anc);
				}
			}
		}
		else start = i + 1;
	}
}

void BChoppr_GUI::rearrange_controllers ()
{
	int nrStepsi = INT (nrSteps);

	if ((nrStepsi < 1) || (nrStepsi > MAXSTEPS)) return;

	double sw = sContainer.getEffectiveWidth();
	double sx = sContainer.getXOffset();

	for (int i = 0; i < MAXSTEPS; ++i)
	{
		if (i < nrStepsi)
		{
			stepControl[i].moveTo ((i + 0.5) * sw / nrStepsi + sx - 14 * sz, 40 * sz);
			stepControl[i].show ();
		}
		else stepControl[i].hide ();
	}

	for (int i = 0; i < MAXSTEPS - 1; ++i)
	{
		if (i < nrStepsi - 1)
		{
			markerWidgets[i].moveTo (markerWidgets[i].getValue() * sw + sx - 5 * sz, 10 * sz);
			markerWidgets[i].show ();
		}
		else markerWidgets[i].hide ();
	}
}

void BChoppr_GUI::valueChangedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();

		if (widget->getMainWindow ())
		{
			BChoppr_GUI* ui = (BChoppr_GUI*) widget->getMainWindow ();

			// monitor on/off changed
			if (widget == &ui->monitorSwitch)
			{
				int value = INT (widget->getValue ());
				if (value == 1)
				{
					ui->mainMonitor.record_on = true;
					ui->send_record_on ();
				}
				else
				{
					ui->mainMonitor.record_on = false;
					ui->send_record_off ();
				}
				return;
			}

			// Bypass changed
			if (widget == &ui->bypassButton)
			{
				ui->bypass = (float) widget->getValue ();
				ui->write_function (ui->controller, Bypass, sizeof(ui->bypass), 0, &ui->bypass);
				return;
			}

			// Dry/wet changed
			if (widget == &ui->drywetDial)
			{
				ui->drywet = (float) widget->getValue ();
				ui->write_function (ui->controller, DryWet, sizeof(ui->drywet), 0, &ui->drywet);
				return;
			}

			// Attack changed
			if (widget == &ui->attackControl)
			{
				ui->attack = (float) widget->getValue ();
				ui->write_function(ui->controller, Attack, sizeof(ui->attack), 0, &ui->attack);
				ui->redrawStepshape ();
				return;
			}

			// Release changed
			if (widget == &ui->releaseControl)
			{
				ui->release = (float) widget->getValue ();
				ui->write_function(ui->controller, Release, sizeof(ui->release), 0, &ui->release);
				ui->redrawStepshape ();
				return;
			}

			// Step size changed
			if (widget == &ui->sequencesperbarControl)
			{
				ui->sequencesperbar = (float) widget->getValue ();
				ui->write_function(ui->controller, SequencesPerBar, sizeof(ui->sequencesperbar), 0, &ui->sequencesperbar);
				return;
			}

			if (widget == &ui->swingControl)
			{
				ui->swing = (float) widget->getValue ();
				ui->write_function(ui->controller, Swing, sizeof(ui->swing), 0, &ui->swing);

				ui->setAutoMarkers();
				ui->rearrange_controllers();
				ui->redrawSContainer();
				ui->redrawMainMonitor();
				return;
			}

			// nrSteps changed
			if (widget == &ui->nrStepsControl)
			{
				float nrSteps_new = (float) widget->getValue ();
				if (nrSteps_new != ui->nrSteps)
				{
					ui->nrSteps = nrSteps_new;
					ui->setAutoMarkers ();
					ui->rearrange_controllers ();
					ui->write_function(ui->controller, NrSteps, sizeof(ui->nrSteps), 0, &ui->nrSteps);
					ui->redrawMainMonitor ();
					ui->redrawSContainer ();
				}
				return;
			}

			// Step controllers changed
			for (int i = 0; i < MAXSTEPS; i++)
			{
				if ((i < MAXSTEPS - 1) && (widget == &ui->markerWidgets[i]))
				{
					float pos = (ui->markerWidgets[i].hasValue() ? widget->getValue () : 0.0f);
					ui->write_function(ui->controller, StepPositions + i , sizeof (pos), 0, &pos);
					return;
				}

				if (widget == &ui->stepControl[i])
				{
					float step = (float) widget->getValue ();
					ui->write_function(ui->controller, StepLevels + i , sizeof (step), 0, &step);
					return;
				}
			}
		}
	}
}

void BChoppr_GUI::markerClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;
	if (pev->getButton() != BDevices::RIGHT_BUTTON) return;
	Marker* marker = (Marker*)event->getWidget();
	if (!marker) return;
	marker->raiseToTop();
	BChoppr_GUI* ui = (BChoppr_GUI*)marker->getMainWindow();
	if (!ui) return;

	for (int i = 0; i < ui->nrSteps - 1; ++i)
	{
		if (marker == &ui->markerWidgets[i])
		{
			Marker* oldMarker = (Marker*) ui->markerListBox.getParent();
			ui->markerListBox.setValue (UNSELECTED);

			if (oldMarker && (oldMarker == marker))
			{
				if (ui->markerListBox.isVisible()) ui->markerListBox.hide();
				else ui->markerListBox.show ();
			}

			else if (oldMarker && (oldMarker != marker))
			{
				oldMarker->release (&ui->markerListBox);
				marker->add (ui->markerListBox);
				ui->markerListBox.show();
			}

			else
			{
				marker->add (ui->markerListBox);
				ui->markerListBox.show();
			}

		}
	}
}

void BChoppr_GUI::markerDraggedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;
	if (pev->getButton() != BDevices::LEFT_BUTTON) return;
	Marker* marker = (Marker*)event->getWidget();
	if (!marker) return;
	marker->raiseToTop();
	BChoppr_GUI* ui = (BChoppr_GUI*)marker->getMainWindow();
	if (!ui) return;

	for (int i = 0; i < ui->nrSteps - 1; ++i)
	{
		if (marker == &ui->markerWidgets[i])
		{
			double x0 = ui->sContainer.getXOffset();
			double w = ui->sContainer. getEffectiveWidth();
			double frac = (w > 0 ? (pev->getPosition().x + marker->getPosition().x - x0) / w : MINMARKERVALUE);
			frac = LIMIT (frac, MINMARKERVALUE, 1.0);

			// Limit to antecessors value
			for (int j = i - 1; j >= 0; --j)
			{
				if (ui->markerWidgets[j].hasValue())
				{
					if (frac < ui->markerWidgets[j].getValue()) frac = ui->markerWidgets[j].getValue();
					break;
				}
			}

			// Limit to successors value
			for (int j = i + 1; j < ui->nrSteps - 1; ++j)
			{
				if (ui->markerWidgets[j].hasValue())
				{
					if (frac > ui->markerWidgets[j].getValue()) frac = ui->markerWidgets[j].getValue();
					break;
				}
			}

			ui->setMarker (i, frac);
			ui->setAutoMarkers();
			ui->rearrange_controllers();
			ui->redrawSContainer();
			ui->redrawMainMonitor();
			break;
		}
	}
}

void BChoppr_GUI::monitorScrolledCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::WheelEvent* wev = (BEvents::WheelEvent*) event;
	BWidgets::Widget* widget = event->getWidget();
	if (!widget) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)widget->getMainWindow();
	if (!ui) return;

	ui->scale += 0.1 * wev->getDelta().y * ui->scale;
	if (ui->scale < 0.0001f) ui->scale = 0.0001f;
	ui->redrawMainMonitor ();
}

void BChoppr_GUI::monitorDraggedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* wev = (BEvents::PointerEvent*) event;
	BWidgets::Widget* widget = event->getWidget();
	if (!widget) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)widget->getMainWindow();
	if (!ui) return;

	ui->scale += 0.01 * wev->getDelta().y * ui->scale;
	if (ui->scale < 0.0001f) ui->scale = 0.0001f;
	ui->redrawMainMonitor ();
}

void BChoppr_GUI::listBoxChangedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::ValueChangedEvent* vev = (BEvents::ValueChangedEvent*) event;
	BWidgets::ListBox* lb = (BWidgets::ListBox*) vev->getWidget();
	if (!lb) return;
	Marker* m = (Marker*) lb->getParent();
	if (!m) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)m->getMainWindow();
	if (!ui) return;

	double value = vev->getValue();
	if (value == 1.0) m->setHasValue (false);
	else if (value == 2.0) m->setHasValue (true);
	else return;

	lb->hide();
	ui->setAutoMarkers();
	ui->rearrange_controllers();
	ui->redrawSContainer();
	ui->redrawMainMonitor();
}

void BChoppr_GUI::markersAutoClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::ValueChangedEvent* vev = (BEvents::ValueChangedEvent*) event;
	if (vev->getValue() == 0.0) return;
	BWidgets::TextButton* tb = (BWidgets::TextButton*) vev->getWidget();
	if (!tb) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)tb->getMainWindow();
	if (!ui) return;

	for (Marker& m : ui->markerWidgets) m.setHasValue (false);

	ui->setAutoMarkers();
	ui->rearrange_controllers();
	ui->redrawSContainer();
	ui->redrawMainMonitor();
}

void BChoppr_GUI::buttonClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::DrawingSurface* w = (BWidgets::DrawingSurface*) event->getWidget();
	if (!w) return;
	BChoppr_GUI* ui = (BChoppr_GUI*) w->getMainWindow();
	if (!ui) return;

	if (w == &ui->rectButton) {ui->blend = 1; ui->sinButton.rename ("nbutton");}
	if (w == &ui->sinButton) {ui->blend = 2; ui->rectButton.rename ("nbutton");}
	w->rename ("abutton");
	ui->rectButton.applyTheme (ui->theme);
	ui->sinButton.applyTheme (ui->theme);
	ui->redrawButtons ();
	ui->redrawStepshape ();

	float fblend = ui->blend;
	ui->write_function(ui->controller, Blend, sizeof(fblend), 0, &fblend);
}

void BChoppr_GUI::helpButtonClickedCallback (BEvents::Event* event) {system(OPEN_CMD " " HELP_URL);}
void BChoppr_GUI::ytButtonClickedCallback (BEvents::Event* event) {system(OPEN_CMD " " YT_URL);}

bool BChoppr_GUI::init_Stepshape ()
{
	double height = stepshapeDisplay.getEffectiveHeight ();
	pat5 = cairo_pattern_create_linear (0, 0, 0, height);

	return (pat5 && (cairo_pattern_status (pat5) == CAIRO_STATUS_SUCCESS));
}

void BChoppr_GUI::destroy_Stepshape ()
{
	//Destroy also mainMonitors cairo data
	if (pat5 && (cairo_pattern_status (pat5) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat5);
}

void BChoppr_GUI::redrawStepshape ()
{
	double width = stepshapeDisplay.getEffectiveWidth ();
	double height = stepshapeDisplay.getEffectiveHeight ();

	cairo_t* cr = cairo_create (stepshapeDisplay.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	// Draw background
	cairo_set_source_rgba (cr, CAIRO_BG_COLOR);
	cairo_rectangle (cr, 0.0, 0.0, width, height);
	cairo_fill (cr);
	cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::grey));
	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, 0, 0.2 * height);
	cairo_line_to (cr, width, 0.2 * height);
	cairo_move_to (cr, 0, 0.55 * height);
	cairo_line_to (cr, width, 0.55 * height);
	cairo_move_to (cr, 0, 0.9 * height);
	cairo_line_to (cr, width, 0.9 * height);
	cairo_move_to (cr, 0.25 * width, 0);
	cairo_line_to (cr, 0.25 * width, height);
	cairo_move_to (cr, 0.5 * width, 0);
	cairo_line_to (cr, 0.5 * width, height);
	cairo_move_to (cr, 0.75 * width, 0);
	cairo_line_to (cr, 0.75 * width, height);
	cairo_stroke (cr);

	// Draw step shape
	cairo_set_source_rgba (cr, CAIRO_INK1, 1.0);
	cairo_set_line_width (cr, 3);

	cairo_move_to (cr, 0, 0.9 * height);
	cairo_line_to (cr, width * 0.25, 0.9 * height);

	if (blend == 1)
	{
		if ((attack + release) > 1)
		{
			float crosspointX = attack / (attack + release);
			float crosspointY = crosspointX / attack - (crosspointX - (1 - release)) / release;
			cairo_line_to (cr, width * 0.25 + crosspointX * width * 0.5, 0.9 * height - 0.7 * height * crosspointY);
		}
		else
		{
			cairo_line_to (cr, width * 0.25 + attack * width * 0.5, 0.2 * height);
			cairo_line_to (cr, width * 0.75  - release * width * 0.5, 0.2 * height);

		}
	}

	else if (blend == 2)
	{
		for (double i = 0.0; i <= 1.0; i += 0.025)
		{
			double vol = 1.0;
			if (i < attack) vol = sin (M_PI * (i / attack - 0.5));
			if (i > (1 - release)) vol = vol * sin (M_PI * ((1 - i) / release - 0.5));
			cairo_line_to (cr, width * (0.25 + 0.5 * i), height * (0.55 - 0.35 * vol));
		}
	}

	cairo_line_to (cr, width * 0.75, 0.9 * height);
	cairo_line_to (cr, width, 0.9 * height);

	cairo_stroke_preserve (cr);

	cairo_pattern_add_color_stop_rgba (pat5, 0.1, CAIRO_INK1, 1);
	cairo_pattern_add_color_stop_rgba (pat5, 0.9, CAIRO_INK1, 0);
	cairo_set_source (cr, pat5);
	cairo_line_to(cr, 0, 0.9 * height);
	cairo_set_line_width (cr, 0);
	cairo_fill (cr);

	cairo_destroy (cr);

	stepshapeDisplay.update ();
}

bool BChoppr_GUI::init_mainMonitor ()
{
	//Initialize mainMonitor
	mainMonitor.record_on = true;
	mainMonitor.width = 0;
	mainMonitor.height = 0;
	mainMonitor.data.fill (defaultNotification);
	mainMonitor.horizonPos = 0;

	//Initialize mainMonitors cairo data
	double width = monitorDisplay.getEffectiveWidth ();
	double height = monitorDisplay.getEffectiveHeight ();
	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
	cr1 = cairo_create (surface);
	cr2 = cairo_create (surface);
	cr3 = cairo_create (surface);
	cr4 = cairo_create (surface);
	pat1 = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat1, 0.1, CAIRO_INK1, 1);
	cairo_pattern_add_color_stop_rgba (pat1, 0.6, CAIRO_INK1, 0);
	pat2 = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat2, 0.1, CAIRO_INK2, 1);
	cairo_pattern_add_color_stop_rgba (pat2, 0.6, CAIRO_INK2, 0);
	pat3 = cairo_pattern_create_linear (0, height, 0, 0);
	cairo_pattern_add_color_stop_rgba (pat3, 0.1, CAIRO_INK1, 1);
	cairo_pattern_add_color_stop_rgba (pat3, 0.6, CAIRO_INK1, 0);
	pat4 = cairo_pattern_create_linear (0, height, 0, 0);
	cairo_pattern_add_color_stop_rgba (pat4, 0.1, CAIRO_INK2, 1);
	cairo_pattern_add_color_stop_rgba (pat4, 0.6, CAIRO_INK2, 0);

	return (pat4 && (cairo_pattern_status (pat4) == CAIRO_STATUS_SUCCESS) &&
			pat3 && (cairo_pattern_status (pat3) == CAIRO_STATUS_SUCCESS) &&
			pat2 && (cairo_pattern_status (pat2) == CAIRO_STATUS_SUCCESS) &&
			pat1 && (cairo_pattern_status (pat1) == CAIRO_STATUS_SUCCESS) &&
			cr4 && (cairo_status (cr4) == CAIRO_STATUS_SUCCESS) &&
			cr3 && (cairo_status (cr3) == CAIRO_STATUS_SUCCESS)&&
			cr2 && (cairo_status (cr2) == CAIRO_STATUS_SUCCESS) &&
			cr1 && (cairo_status (cr1) == CAIRO_STATUS_SUCCESS) &&
			surface && (cairo_surface_status (surface) == CAIRO_STATUS_SUCCESS));
}

void BChoppr_GUI::destroy_mainMonitor ()
{
	//Destroy also mainMonitors cairo data
	if (pat4 && (cairo_pattern_status (pat4) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat4);
	if (pat3 && (cairo_pattern_status (pat3) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat3);
	if (pat2 && (cairo_pattern_status (pat2) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat2);
	if (pat1 && (cairo_pattern_status (pat1) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat1);
	if (cr4 && (cairo_status (cr4) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr4);
	if (cr3 && (cairo_status (cr3) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr3);
	if (cr2 && (cairo_status (cr2) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr2);
	if (cr1 && (cairo_status (cr1) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr1);
	if (surface && (cairo_surface_status (surface) == CAIRO_STATUS_SUCCESS)) cairo_surface_destroy (surface);
}

void BChoppr_GUI::add_monitor_data (BChopprNotifications* notifications, uint32_t notificationsCount, uint32_t& end)
{
	for (uint32_t i = 0; i < notificationsCount; ++i)
	{
		int monitorpos = notifications[i].position;
		if (monitorpos >= MONITORBUFFERSIZE) monitorpos = MONITORBUFFERSIZE;
		if (monitorpos < 0) monitorpos = 0;

		mainMonitor.data[monitorpos].inputMin = notifications[i].inputMin;
		mainMonitor.data[monitorpos].inputMax = notifications[i].inputMax;
		mainMonitor.data[monitorpos].outputMin = notifications[i].outputMin;
		mainMonitor.data[monitorpos].outputMax = notifications[i].outputMax;
		mainMonitor.horizonPos = monitorpos;
	}
}

void BChoppr_GUI::redrawMainMonitor ()
{
	double width = monitorDisplay.getEffectiveWidth ();
	double height = monitorDisplay.getEffectiveHeight ();

	cairo_t* cr = cairo_create (monitorDisplay.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	// Draw background
	cairo_set_source_rgba (cr, CAIRO_BG_COLOR);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);

	cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::grey));
	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, 0, 0.1 * height);
	cairo_line_to (cr, width, 0.1 * height);
	cairo_move_to (cr, 0, 0.5 * height);
	cairo_line_to (cr, width, 0.5 * height);
	cairo_move_to (cr, 0, 0.9 * height);
	cairo_line_to (cr, width, 0.9 * height);

	uint32_t steps = (uint32_t) nrSteps - 1;
	for (uint32_t i = 0; i < steps; ++i)
	{
		cairo_move_to (cr, markerWidgets[i].getValue() * width, 0);
		cairo_rel_line_to (cr, 0, height);
	}
	cairo_stroke (cr);

	if (mainMonitor.record_on)
	{
		cairo_surface_clear (surface);

		// Draw input (cr, cr3) and output (cr2, cr4) curves
		cairo_move_to (cr1, 0, height * (0.5  - (0.4 * LIM ((mainMonitor.data[0].inputMax / scale), 0.0f, 1.0f))));
		cairo_move_to (cr2, 0, height * (0.5  - (0.4 * LIM ((mainMonitor.data[0].outputMax / scale), 0.0f, 1.0f))));
		cairo_move_to (cr3, 0, height * (0.5  + (0.4 * LIM (-(mainMonitor.data[0].inputMin / scale), 0.0f, 1.0f))));
		cairo_move_to (cr4, 0, height * (0.5  + (0.4 * LIM (-(mainMonitor.data[0].outputMin / scale), 0.0f, 1.0f))));

		for (int i = 0; i < MONITORBUFFERSIZE; ++i)
		{
			double pos = ((double) i) / (MONITORBUFFERSIZE - 1.0f);
			cairo_line_to (cr1, pos * width, height * (0.5  - (0.4 * LIM ((mainMonitor.data[i].inputMax / scale), 0.0f, 1.0f))));
			cairo_line_to (cr2, pos * width, height * (0.5  - (0.4 * LIM ((mainMonitor.data[i].outputMax / scale), 0.0f, 1.0f))));
			cairo_line_to (cr3, pos * width, height * (0.5  + (0.4 * LIM (-(mainMonitor.data[i].inputMin / scale), 0.0f, 1.0f))));
			cairo_line_to (cr4, pos * width, height * (0.5  + (0.4 * LIM (-(mainMonitor.data[i].outputMin / scale), 0.0f, 1.0f))));
		}

		// Visualize input (cr, cr3) and output (cr2, cr4) curves
		cairo_set_source_rgba (cr1, CAIRO_INK1, 1.0);
		cairo_set_line_width (cr1, 3);
		cairo_set_source_rgba (cr2, CAIRO_INK2, 1.0);
		cairo_set_line_width (cr2, 3);
		cairo_stroke_preserve (cr1);
		cairo_stroke_preserve (cr2);
		cairo_set_source_rgba (cr3, CAIRO_INK1, 1.0);
		cairo_set_line_width (cr3, 3);
		cairo_set_source_rgba (cr4, CAIRO_INK2, 1.0);
		cairo_set_line_width (cr4, 3);
		cairo_stroke_preserve (cr3);
		cairo_stroke_preserve (cr4);

		// Visualize input (cr, cr3) and output (cr2, cr4) areas under the curves
		cairo_line_to (cr1, width, height * 0.5);
		cairo_line_to (cr1, 0, height * 0.5);
		cairo_close_path (cr1);
		cairo_line_to (cr2, width, height * 0.5);
		cairo_line_to (cr2, 0, height * 0.5);
		cairo_close_path (cr2);
		cairo_set_source (cr1, pat1);
		cairo_set_line_width (cr1, 0);
		cairo_set_source (cr2, pat2);
		cairo_set_line_width (cr2, 0);
		cairo_fill (cr1);
		cairo_fill (cr2);
		cairo_line_to (cr3, width, height * 0.5);
		cairo_line_to (cr3, 0, height * 0.5);
		cairo_close_path (cr3);
		cairo_line_to (cr4, width, height * 0.5);
		cairo_line_to (cr4, 0, height * 0.5);
		cairo_close_path (cr4);
		cairo_set_source (cr3, pat3);
		cairo_set_line_width (cr3, 0);
		cairo_set_source (cr4, pat4);
		cairo_set_line_width (cr4, 0);
		cairo_fill (cr3);
		cairo_fill (cr4);

		// Draw fade out
		double horizon = ((double) mainMonitor.horizonPos) / (MONITORBUFFERSIZE - 1.0f);
		cairo_pattern_t* pat6 = cairo_pattern_create_linear (horizon * width, 0, horizon * width + 63, 0);
		if (cairo_pattern_status (pat6) == CAIRO_STATUS_SUCCESS)
		{
			cairo_pattern_add_color_stop_rgba (pat6, 0.0, CAIRO_BG_COLOR);
			cairo_pattern_add_color_stop_rgba (pat6, 1.0, CAIRO_TRANSPARENT);
			cairo_set_line_width (cr1, 0.0);
			cairo_set_source (cr1, pat6);
			cairo_rectangle (cr1, horizon * width, 0, 63, height);
			cairo_fill (cr1);
			cairo_pattern_destroy (pat6);
		}

		if (horizon * width > width - 63)
		{
			cairo_pattern_t* pat6 = cairo_pattern_create_linear ((horizon - 1) * width, 0, (horizon - 1) * width + 63, 0);
			if (cairo_pattern_status (pat6) == CAIRO_STATUS_SUCCESS)
			{
				cairo_pattern_add_color_stop_rgba (pat6, 0.0, CAIRO_BG_COLOR);
				cairo_pattern_add_color_stop_rgba (pat6, 1.0, CAIRO_TRANSPARENT);
				cairo_set_line_width (cr1, 0.0);
				cairo_set_source (cr1, pat6);
				cairo_rectangle (cr1, (horizon - 1) * width, 0, 63, height);
				cairo_fill (cr1);
				cairo_pattern_destroy (pat6);
			}
		}

		// Draw horizon line
		cairo_set_source_rgba (cr1, CAIRO_FG_COLOR);
		cairo_set_line_width (cr1, 1);
		cairo_move_to (cr1, horizon * width, 0);
		cairo_line_to (cr1, horizon * width, height);
		cairo_stroke (cr1);
	}

	cairo_set_source_surface (cr, surface, 0, 0);
	cairo_paint (cr);

	cairo_destroy (cr);
	monitorDisplay.update ();
}

void BChoppr_GUI::redrawSContainer ()
{
	double width = sContainer.getEffectiveWidth ();
	double height = sContainer.getEffectiveHeight ();

	cairo_surface_clear (sContainer.getDrawingSurface ());
	cairo_t* cr = cairo_create (sContainer.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	cairo_pattern_t* pat = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat, 0.0, CAIRO_RGBA (BColors::black));
	cairo_pattern_add_color_stop_rgba (pat, 1.0, 0.0, 0.0, 0.0, 0.5);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_set_source (cr, pat);
	cairo_fill (cr);
	cairo_pattern_destroy (pat);

	for (int i = 0; i < nrSteps - 1; ++i)
	{
		cairo_set_line_width (cr, 1.0);
		cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::grey));
		cairo_move_to (cr, markerWidgets[i].getValue() * width, 0);
		cairo_rel_line_to (cr, 0, 30 * sz);
		cairo_line_to (cr, (i + 1) / nrSteps * width, 40 * sz);
		cairo_rel_line_to (cr, 0, 100 * sz);
		cairo_stroke (cr);
	}

	cairo_destroy (cr);
	sContainer.update();
}

void BChoppr_GUI::redrawButtons ()
{
	// rectButton
	double width = rectButton.getEffectiveWidth ();
	double height = rectButton.getEffectiveHeight ();

	cairo_surface_clear (rectButton.getDrawingSurface ());
	cairo_t* cr = cairo_create (rectButton.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	cairo_set_source_rgba (cr, CAIRO_RGBA (*rectButton.getBorder()->getLine()->getColor()));
	cairo_set_line_width (cr, 2.0);

	cairo_move_to (cr, 0.05 * width, 0.9 * height);
	cairo_line_to (cr, 0.25 * width, 0.9 * height);
	cairo_line_to (cr, 0.3 * width, 0.1 * height);
	cairo_line_to (cr, 0.7 * width, 0.1 * height);
	cairo_line_to (cr, 0.75 * width, 0.9 * height);
	cairo_line_to (cr, 0.95 * width, 0.9 * height);
	cairo_stroke (cr);

	cairo_destroy (cr);

	// sinButton
	width = sinButton.getEffectiveWidth ();
	height = sinButton.getEffectiveHeight ();

	cairo_surface_clear (sinButton.getDrawingSurface ());
	cr = cairo_create (sinButton.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	cairo_set_source_rgba (cr, CAIRO_RGBA (*sinButton.getBorder()->getLine()->getColor()));
	cairo_set_line_width (cr, 2.0);

	cairo_move_to (cr, 0.05 * width, 0.9 * height);
	cairo_line_to (cr, 0.15 * width, 0.9 * height);
	for (int i = 0; i <= 10; ++i) cairo_line_to (cr, (0.15 + i * 0.03) * width, (0.5 - 0.4 * sin (double (i - 5) * M_PI / 10)) * height);
	cairo_line_to (cr, 0.55 * width, 0.1 * height);
	for (int i = 0; i <= 10; ++i) cairo_line_to (cr, (0.55 + i * 0.03) * width, (0.5 - 0.4 * sin (double (i + 5) * M_PI / 10)) * height);
	cairo_line_to (cr, 0.95 * width, 0.9 * height);
	cairo_stroke (cr);

	cairo_destroy (cr);

}

LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path,
						  LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget,
						  const LV2_Feature *const *features)
{
	PuglNativeWindow parentWindow = 0;
	LV2UI_Resize* resize = NULL;

	if (strcmp(plugin_uri, BCHOPPR_URI) != 0)
	{
		std::cerr << "BChoppr.lv2#GUI: GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_UI__parent)) parentWindow = (PuglNativeWindow) features[i]->data;
		else if (!strcmp(features[i]->URI, LV2_UI__resize)) resize = (LV2UI_Resize*)features[i]->data;
	}
	if (parentWindow == 0) std::cerr << "BChoppr.lv2#GUI: No parent window.\n";

	// New instance
	BChoppr_GUI* ui;
	try {ui = new BChoppr_GUI (bundle_path, features, parentWindow);}
	catch (std::exception& exc)
	{
		std::cerr << "BChoppr.lv2#GUI: Instantiation failed. " << exc.what () << std::endl;
		return NULL;
	}

	ui->controller = controller;
	ui->write_function = write_function;

	// Reduce min GUI size for small displays
	double sz = 1.0;
	int screenWidth  = getScreenWidth ();
	int screenHeight = getScreenHeight ();
	if ((screenWidth < 820) || (screenHeight < 600)) sz = 0.66;

	/*
	std::cerr << "BChoppr_GUI.lv2 screen size " << screenWidth << " x " << screenHeight <<
			". Set GUI size to " << 760 * sz << " x " << 560 * sz << ".\n";
	*/

	if (resize) resize->ui_resize(resize->handle, 760 * sz, 560 * sz);

	*widget = (LV2UI_Widget) puglGetNativeWindow (ui->getPuglView ());
	ui->send_record_on();
	return (LV2UI_Handle) ui;
}

void cleanup(LV2UI_Handle ui)
{
	BChoppr_GUI* pluginGui = (BChoppr_GUI*) ui;
	delete pluginGui;
}

void portEvent(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	BChoppr_GUI* pluginGui = (BChoppr_GUI*) ui;
	pluginGui->portEvent(port_index, buffer_size, format, buffer);
}

static int callIdle (LV2UI_Handle ui)
{
	BChoppr_GUI* pluginGui = (BChoppr_GUI*) ui;
	pluginGui->handleEvents ();
	return 0;
}

static const LV2UI_Idle_Interface idle = { callIdle };

static const void* extensionData(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) return &idle;
	else return NULL;
}

const LV2UI_Descriptor guiDescriptor = {
		BCHOPPR_GUI_URI,
		instantiate,
		cleanup,
		portEvent,
		extensionData
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor(uint32_t index)
{
	switch (index) {
	case 0: return &guiDescriptor;
	default:return NULL;
    }
}
