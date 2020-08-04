#ifndef TEST_BUILD

#include "app.h"
#include <direct.h>

std::vector<midi::NOTE> notes;
/*int size = 5;
unsigned int framehop = 200;
unsigned int framewidth = 2000;*/
std::map<std::string, unsigned int> arguments;
std::string midifile;
std::string bmpfile;

int main(int argn, char** argv)
{
	_mkdir("images");

	arguments["-w"] = 0;
	arguments["-d"] = 1;
	arguments["-s"] = 1;
	arguments["-h"] = 16;


	for (int i = 1; i < argn; i++) {
		std::string s(argv[i]);

		if (s.substr(0, 1) == std::string("-")) {
			std::string c(argv[i + 1]);
			std::stringstream str(c);
			int x;
			str >> x;
			arguments[argv[i]] = x;
			i++;
		}
	}

	midifile = std::string(argv[argn - 2]);
	bmpfile = std::string(argv[argn - 1]);

	int pos = bmpfile.find("%d");
	std::string first(bmpfile.substr(0, pos));
	std::string last(bmpfile.substr(pos + 2, bmpfile.size() - 1));


	std::ifstream ifs;
	ifs.open(midifile, std::ifstream::in);
	notes = midi::read_notes(ifs);
	int lowest = getLowestNote()._value;
	int highest = getHighestNote()._value;


	imaging::Bitmap image(getLatestEndTime()._value / 10 * arguments["-s"], (highest - lowest + 1) * arguments["-h"]);

	unsigned int frames = image.width() / arguments["-d"] - (arguments["-w"] / arguments["-d"]) + 3;
	double temp = (image.width() + 1) / arguments["-d"];
	frames = std::ceil(temp);



	for (int i = 0; i < notes.size(); i++) {
		midi::NOTE currentNote = notes[i];
		int indexStart = currentNote.start._value / 10 * arguments["-s"];
		int end = indexStart + (currentNote.duration._value / 10 * arguments["-s"]);
		if (currentNote.note_number._value == 0) {
			continue;
		}

		double r = 0, g = 0, b = 0;

		r =((double) ((currentNote.instrument._value * 538 + 12) % 255)) / 256;
		g = ((double)((currentNote.instrument._value * 256 + 28  ) % 255)) / 256;
		b = ((double)((currentNote.instrument._value * 124 + 700) % 255)) / 256;

		imaging::Color c = imaging::Color{ r, g, b };

		double r2 = 0;

		r2 =  128 - currentNote.note_number._value;
		r2 = r2 / 128;

		imaging::Color c2 = imaging::Color{ r2, 0, 0 };


		drawRectangle(image, indexStart, currentNote.note_number._value - lowest, currentNote.duration._value / 10 * arguments["-s"], c, c2);
		
	}

	if (arguments["-w"] <= 0) {
		std::string number = std::to_string(0);

		int x = number.length();
		for (int k = x; k < 5; k++) {
			number = "0" + number;
		}

		std::string savefile(first + number + last);


		imaging::save_as_bmp("images/" + savefile, image);
	}
	else {
		unsigned int count = 0;
		boolean lastslice = false;
		while (count * arguments["-d"] * arguments["-s"] < image.width() - arguments["-w"] || lastslice){

			std::shared_ptr<imaging::Bitmap> imgslice;
			if (lastslice) {
				unsigned int wi = image.width() - count * arguments["-d"] * arguments["-s"];
				imgslice = image.slice(count * arguments["-d"] * arguments["-s"], 0, wi, image.height());

				lastslice = false;
			}
			else {
				if ((count + 1) * arguments["-d"] * arguments["-s"] >= image.width() - arguments["-w"]) {
					lastslice = true;
				}
				imgslice = image.slice(count * arguments["-d"] * arguments["-s"], 0, arguments["-w"], image.height());
				
			}
			
			std::string number = std::to_string(count);

			int x = number.length();
			for (int k = x; k < 5; k++) {
				number = "0" + number;
			}

			std::string savefile(first + number + last);
			imaging::save_as_bmp("images/" + savefile, *imgslice);
			std::cout << "Saved: " << savefile << std::endl;


			count++;
			
			
		}


	}

	

	
	return 0;
}

void drawRectangle(imaging::Bitmap& image, unsigned int x, unsigned int y, unsigned width, imaging::Color c, imaging::Color note) {
	width = width;
	double part = (width * 1) / 10;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < arguments["-h"]; j++) {
			
			Position pos(x + i, (y * arguments["-h"]) + j);
			if (i <= part) {
				image[pos] = note;
			}
			else {
				image[pos] = c;
			}
		}
	}
}

midi::Time getLatestEndTime() {

	midi::Time end = midi::Time(0);
	for (int i = 0; i < notes.size(); i++) {
		midi::NOTE currentNote = notes[i];

		midi::Time currentTime = currentNote.start + currentNote.duration;
		if (currentTime > end) {
			end = currentTime;
		}

	}


	return end;
}

midi::NoteNumber getLowestNote() {

	midi::NoteNumber lowest = midi::NoteNumber(127);
	for (int i = 0; i < notes.size(); i++) {
		midi::NOTE currentNote = notes[i];

		midi::NoteNumber currentNoteNumber = currentNote.note_number;
		if (currentNoteNumber < lowest) {
			if (currentNoteNumber._value != 0) {
				lowest = currentNoteNumber;
			}
		}

	}


	return lowest;
}

midi::NoteNumber getHighestNote() {

	midi::NoteNumber highest = midi::NoteNumber(0);
	for (int i = 0; i < notes.size(); i++) {
		midi::NOTE currentNote = notes[i];

		midi::NoteNumber currentNoteNumber = currentNote.note_number;
		if (currentNoteNumber > highest) {
			highest = currentNoteNumber;
		}

	}


	return highest;
}





#endif
