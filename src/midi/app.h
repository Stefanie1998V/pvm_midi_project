#pragma once

#include "midi/midi.h"
#include "imaging/color.h"
#include <iostream>
#include "imaging/bitmap.h"
#include "imaging/bmp-format.h"
#include <map>
#include <string>
#include <math.h>

midi::Time getLatestEndTime();
midi::NoteNumber getLowestNote();
midi::NoteNumber getHighestNote();
void drawRectangle(imaging::Bitmap& image, unsigned int x, unsigned int y, unsigned width, imaging::Color c, imaging::Color c2);
