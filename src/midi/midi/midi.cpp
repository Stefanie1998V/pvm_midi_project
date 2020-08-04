#include "midi/midi.h"
#include "io/read.h"
#include "io/endianness.h"
#include "io/vli.h"

namespace midi {

	void read_chunk_header(std::istream& in, midi::CHUNK_HEADER* chunkHeader) {
		io::read_to<CHUNK_HEADER>(in, chunkHeader);
		io::switch_endianness(&(*chunkHeader).size);
	}

	void read_mthd(std::istream& in, midi::MTHD* mthd) {
		io::read_to<MTHD>(in, mthd);
		io::switch_endianness(&(*mthd).header.size);
		io::switch_endianness(&(*mthd).ntracks);
		io::switch_endianness(&(*mthd).type);
		io::switch_endianness(&(*mthd).division);
	}



	std::string header_id(midi::CHUNK_HEADER chunkHeader) {
		std::string stringske("");
		for (int i = 0; i < 4; i++) {
			stringske += chunkHeader.id[i];
		}

		return stringske;
	}


	bool is_meta_event(uint8_t byte)
	{

		return byte == 0xFF;
	}

	bool is_sysex_event(uint8_t byte)
	{
		return (byte == 0xF0 || byte == 0xF7);
	}

	bool is_midi_event(uint8_t byte)
	{
		uint8_t shifted = byte >> 4;

		uint8_t midievents[7] = { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E };

		for (int i = 0; i < 7; i++) {
			if (shifted == midievents[i]) {
				return true;
			}
		}

		return false;

	}


	uint8_t extract_midi_event_type(uint8_t status) {
		return (status >> 4);
	}
	Channel extract_midi_event_channel(uint8_t status) {
		status = status << 4;
		status = status >> 4;
		return Channel(status);
	}


	bool is_running_status(uint8_t byte)
	{
		byte = byte >> 7;
		return (byte == 0);
	}

	bool is_note_off(uint8_t status) {
		return (status == 0x08);
	}

	bool is_note_on(uint8_t status) {
		return (status == 0x09);
	}

	bool is_polyphonic_key_pressure(uint8_t status) {
		return (status == 0x0A);
	}

	bool is_control_change(uint8_t status) {
		return (status == 0x0B);
	}

	bool is_program_change(uint8_t status) {
		return (status == 0x0C);
	}

	bool is_channel_pressure(uint8_t status) {
		return (status == 0x0D);
	}

	bool is_pitch_wheel_change(uint8_t status) {
		return (status == 0x0E);
	}

	void read_mtrk(std::istream& in, EventReceiver& ER)
	{
		char id[4];
		io::read_to<char>(in, id, 4);


		uint32_t length = io::read<uint32_t>(in);

		/*CHUNK_HEADER header;
		io::read_to<CHUNK_HEADER>(in, &header);*/

		uint8_t lastStatus = 0;

		//io::switch_endianness(&length);

		while (true) {
			uint64_t time = io::read_variable_length_integer(in);
			uint8_t ident = io::read<uint8_t>(in);
			midi::Duration duration = midi::Duration(time);

			//std::cout << std::hex << ident << std::endl;
			if (is_meta_event(ident)) {

				uint8_t type = io::read<uint8_t>(in);
				uint64_t data_length = io::read_variable_length_integer(in);
				auto data = io::read_array<uint8_t>(in, data_length);

				ER.meta(duration, type, std::move(data), data_length);
				if (type == 0x2F) {
					break;
				}
			}
			else if (is_sysex_event(ident)) {
				uint64_t data_length = io::read_variable_length_integer(in);
				auto data = io::read_array<uint8_t>(in, data_length);

				ER.sysex(duration, std::move(data), data_length);
			}
			else
			{
				if (is_running_status(ident)) {
					in.putback(ident);
					ident = lastStatus;
				}
				else {
					lastStatus = ident;
				}

				Channel channel = extract_midi_event_channel(ident);
				uint8_t type = extract_midi_event_type(ident);

				if (is_note_off(type))
				{
					midi::NoteNumber note;
					io::read_to(in, &note, 1);
					uint8_t velocity = io::read<uint8_t>(in);
					ER.note_off(duration, channel, note, velocity);
				}
				else if (is_note_on(type))
				{
					midi::NoteNumber note;
					io::read_to(in, &note, 1);
					uint8_t velocity = io::read<uint8_t>(in);
					ER.note_on(duration, channel, note, velocity);

				}
				else if (is_polyphonic_key_pressure(type))
				{
					midi::NoteNumber note;
					io::read_to(in, &note, 1);
					uint8_t pressure = io::read<uint8_t>(in);
					ER.polyphonic_key_pressure(duration, channel, note, pressure);
				}
				else if (is_control_change(type))
				{
					uint8_t controller = io::read<uint8_t>(in);
					uint8_t value = io::read<uint8_t>(in);
					ER.control_change(duration, channel, controller, value);
				}
				else if (is_program_change(type))
				{
					midi::Instrument program;
					io::read_to(in, &program);
					ER.program_change(duration, channel, program);
				}
				else if (is_channel_pressure(type))
				{
					uint8_t pressure = io::read<uint8_t>(in);
					ER.channel_pressure(duration, channel, pressure);
				}
				else if (is_pitch_wheel_change(type))
				{
					uint8_t lower_bits = io::read<uint8_t>(in);
					lower_bits = lower_bits << 1;
					lower_bits = lower_bits >> 1;

					uint16_t lower = 0;
					lower = lower_bits | lower;

					//1010000000

					uint8_t upper_bits = io::read<uint8_t>(in);
					upper_bits = upper_bits << 1;
					upper_bits = upper_bits >> 1;

					uint16_t upper = 0;
					upper = upper_bits | upper;

					upper = upper << 7;

					uint16_t value = lower | upper;


					ER.pitch_wheel_change(duration, channel, value);
				}
			}
		}


	}

	std::ostream& midi::operator <<(std::ostream& out, const midi::NOTE& nootje)
	{
		out << "Note(number=" << nootje.note_number << ",start=" << nootje.start << ",duration=" << nootje.duration << ",instrument=" << nootje.instrument << ")";
		return out;
	}



	void ChannelNoteCollector::note_on(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{


		if (velocity == 0) {

			this->note_off(dt, channel, note, velocity);
		}
		else {
			this->time += dt;
			if (this->channel == channel) {
				if (this->notes.count(note)) {
					this->note_off(midi::Duration(0), channel, note, velocity);
				}


				NOTE nNote;
				nNote.instrument = this->instrument;
				nNote.note_number = note;
				nNote.velocity = velocity;
				nNote.start = this->time;

				this->notes[note] = nNote;
			}

		}

	}

	void ChannelNoteCollector::note_off(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{
		this->time += dt;
		if (this->channel == channel) {
			this->notes[note].duration = this->time - this->notes[note].start;

			this->note_receiver(notes[note]);
			this->notes.erase(note);
		}



	}

	void ChannelNoteCollector::polyphonic_key_pressure(Duration dt, Channel channel, NoteNumber note, uint8_t pressure)
	{
		this->time += dt;

	}

	void ChannelNoteCollector::control_change(Duration dt, Channel channel, uint8_t controller, uint8_t value)
	{
		this->time += dt;
	}

	void ChannelNoteCollector::program_change(Duration dt, Channel channel, Instrument program)
	{
		this->time += dt;
		if (this->channel == channel) {
			this->instrument = program;
		}
	}

	void ChannelNoteCollector::channel_pressure(Duration dt, Channel channel, uint8_t pressure)
	{
		this->time += dt;
		if (this->channel == channel) {}
	}

	void ChannelNoteCollector::pitch_wheel_change(Duration dt, Channel channel, uint16_t value)
	{
		this->time += dt;
		if (this->channel == channel) {}
	}

	void ChannelNoteCollector::meta(Duration dt, uint8_t type, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		this->time += dt;
	}

	void ChannelNoteCollector::sysex(Duration dt, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		this->time += dt;
	}








	void EventMulticaster::note_on(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{
		for (int i = 0; i < this->RES.size(); i++) {
			this->RES[i]->note_on(dt, channel, note, velocity);
		}
	}

	void EventMulticaster::note_off(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{
		for (int i = 0; i < this->RES.size(); i++) {
			this->RES[i]->note_off(dt, channel, note, velocity);
		}
	}

	void EventMulticaster::polyphonic_key_pressure(Duration dt, Channel channel, NoteNumber note, uint8_t pressure)
	{
		for (int i = 0; i < this->RES.size(); i++) {
			this->RES[i]->polyphonic_key_pressure(dt, channel, note, pressure);
		}
	}

	void EventMulticaster::control_change(Duration dt, Channel channel, uint8_t controller, uint8_t value)
	{
		for (int i = 0; i < this->RES.size(); i++) {
			this->RES[i]->control_change(dt, channel, controller, value);
		}
	}

	void EventMulticaster::program_change(Duration dt, Channel channel, Instrument program)
	{
		for (int i = 0; i < this->RES.size(); i++) {
			this->RES[i]->program_change(dt, channel, program);
		}
	}

	void EventMulticaster::channel_pressure(Duration dt, Channel channel, uint8_t pressure)
	{
		for (int i = 0; i < this->RES.size(); i++) {
			this->RES[i]->channel_pressure(dt, channel, pressure);
		}
	}

	void EventMulticaster::pitch_wheel_change(Duration dt, Channel channel, uint16_t value)
	{
		for (int i = 0; i < this->RES.size(); i++) {
			this->RES[i]->pitch_wheel_change(dt, channel, value);
		}
	}

	void EventMulticaster::meta(Duration dt, uint8_t type, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		for (int i = 0; i < this->RES.size(); i++) {
			this->RES[i]->meta(dt, type, std::move(data), data_size);
		}
	}

	void EventMulticaster::sysex(Duration dt, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		for (int i = 0; i < this->RES.size(); i++) {
			this->RES[i]->sysex(dt, std::move(data), data_size);
		}
	}






	void NoteCollector::note_on(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{
		this->EM.note_on(dt, channel, note, velocity);
	}

	void NoteCollector::note_off(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{
		this->EM.note_off(dt, channel, note, velocity);
	}

	void NoteCollector::polyphonic_key_pressure(Duration dt, Channel channel, NoteNumber note, uint8_t pressure)
	{
		this->EM.polyphonic_key_pressure(dt, channel, note, pressure);
	}

	void NoteCollector::control_change(Duration dt, Channel channel, uint8_t controller, uint8_t value)
	{
		this->EM.control_change(dt, channel, controller, value);
	}

	void NoteCollector::program_change(Duration dt, Channel channel, Instrument program)
	{
		this->EM.program_change(dt, channel, program);
	}

	void NoteCollector::channel_pressure(Duration dt, Channel channel, uint8_t pressure)
	{
		this->EM.channel_pressure(dt, channel, pressure);
	}

	void NoteCollector::pitch_wheel_change(Duration dt, Channel channel, uint16_t value)
	{
		this->EM.pitch_wheel_change(dt, channel, value);
	}

	void NoteCollector::meta(Duration dt, uint8_t type, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		this->EM.meta(dt, type, std::move(data), data_size);
	}

	void NoteCollector::sysex(Duration dt, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		this->EM.sysex(dt, std::move(data), data_size);
	}






	std::vector<NOTE> read_notes(std::istream& in)
	{
		std::vector<NOTE> x;


		midi:MTHD mthd;
		midi::read_mthd(in, &mthd);

		for (int i = 0; i < mthd.ntracks; i++) {
			NoteCollector nc([&x](const midi::NOTE& note) { x.push_back(note); });
			midi::read_mtrk(in, nc);
		}

		return x;
	}


}