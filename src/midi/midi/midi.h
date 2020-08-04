#pragma once
#include <cstdint>
#include "io/read.h"
#include "midi/primitives.h"
#include <functional>

namespace midi {
	struct CHUNK_HEADER {

		char id[4];
		uint32_t size;


	};

	void read_chunk_header(std::istream& in, CHUNK_HEADER* chunkHeader);
	std::string header_id(CHUNK_HEADER chunkHeader);

#pragma pack(push, 1)
	struct MTHD {
		CHUNK_HEADER header;
		uint16_t type;
		uint16_t ntracks;
		uint16_t division;
	};

#pragma pack(pop)
	void read_mthd(std::istream& in, MTHD* mthd);


	bool is_sysex_event(uint8_t byte);
	bool is_meta_event(uint8_t byte);
	bool is_midi_event(uint8_t byte);
	bool is_running_status(uint8_t byte);

	uint8_t extract_midi_event_type(uint8_t status);
	Channel extract_midi_event_channel(uint8_t status);

	bool is_note_off(uint8_t status);
	bool is_note_on(uint8_t status);
	bool is_polyphonic_key_pressure(uint8_t status);
	bool is_control_change(uint8_t status);
	bool is_program_change(uint8_t status);
	bool is_channel_pressure(uint8_t status);
	bool is_pitch_wheel_change(uint8_t status);

	class EventReceiver {
	public:
		virtual void note_on(Duration dt, Channel channel, NoteNumber note, uint8_t velocity) = 0;
		virtual void note_off(Duration dt, Channel channel, NoteNumber note, uint8_t velocity) = 0;
		virtual void polyphonic_key_pressure(Duration dt, Channel channel, NoteNumber note, uint8_t pressure) = 0;
		virtual void control_change(Duration dt, Channel channel, uint8_t controller, uint8_t value) = 0;
		virtual void program_change(Duration dt, Channel channel, Instrument program) = 0;
		virtual void channel_pressure(Duration dt, Channel channel, uint8_t pressure) = 0;
		virtual void pitch_wheel_change(Duration dt, Channel channel, uint16_t value) = 0;

		virtual void meta(Duration dt, uint8_t type, std::unique_ptr<uint8_t[]> data, uint64_t data_size) = 0;
		virtual void sysex(Duration dt, std::unique_ptr<uint8_t[]> data, uint64_t data_size) = 0;
	};



	void read_mtrk(std::istream& in, EventReceiver &ER);



	struct NOTE {
		midi::NoteNumber note_number;
		midi::Time start;
		midi::Duration duration;
		uint8_t velocity;
		midi::Instrument instrument;

		NOTE(midi::NoteNumber note_number, midi::Time start, midi::Duration duration, uint8_t velocity, midi::Instrument instrument) 
			: note_number(note_number), start(start), duration(duration), velocity(velocity), instrument(instrument)  {

		}

		NOTE() {

		}


		bool operator ==(const midi::NOTE note) const
		{
			return (note_number == note.note_number && start == note.start && duration == note.duration && velocity == note.velocity && instrument == note.instrument);
		}

		bool operator !=(const midi::NOTE note)
		{
			return !(*this == note);
		}


		friend std::ostream& operator <<(std::ostream& out, const midi::NOTE& nootje);
		
	};

	struct ChannelNoteCollector : public EventReceiver {
		
		Channel channel;
		Time time;
		midi::Instrument instrument;
		std::map<NoteNumber, NOTE> notes;
		std::function<void(const NOTE&)> note_receiver;


		ChannelNoteCollector(Channel channel, std::function<void(const NOTE&)> note_receiver) : channel(channel), note_receiver(note_receiver) {

		}

		ChannelNoteCollector(Channel channel, Time time, Instrument instrument, std::function<void(const NOTE&)> note_receiver) : channel(channel), time(time), instrument(instrument) {

		}



		virtual void note_on(Duration dt, Channel channel, NoteNumber note, uint8_t velocity) override;

		virtual void note_off(Duration dt, Channel channel, NoteNumber note, uint8_t velocity) override;

		virtual void polyphonic_key_pressure(Duration dt, Channel channel, NoteNumber note, uint8_t pressure) override;

		virtual void control_change(Duration dt, Channel channel, uint8_t controller, uint8_t value) override;

		virtual void program_change(Duration dt, Channel channel, Instrument program) override;

		virtual void channel_pressure(Duration dt, Channel channel, uint8_t pressure) override;

		virtual void pitch_wheel_change(Duration dt, Channel channel, uint16_t value) override;

		virtual void meta(Duration dt, uint8_t type, std::unique_ptr<uint8_t[]> data, uint64_t data_size) override;

		virtual void sysex(Duration dt, std::unique_ptr<uint8_t[]> data, uint64_t data_size) override;

	};


	struct EventMulticaster : public EventReceiver {

		std::vector<std::shared_ptr<midi::EventReceiver>> RES;


		EventMulticaster(std::vector<std::shared_ptr<midi::EventReceiver>> RES) : RES(RES){}


		// Inherited via EventReceiver
		virtual void note_on(Duration dt, Channel channel, NoteNumber note, uint8_t velocity) override;

		virtual void note_off(Duration dt, Channel channel, NoteNumber note, uint8_t velocity) override;

		virtual void polyphonic_key_pressure(Duration dt, Channel channel, NoteNumber note, uint8_t pressure) override;

		virtual void control_change(Duration dt, Channel channel, uint8_t controller, uint8_t value) override;

		virtual void program_change(Duration dt, Channel channel, Instrument program) override;

		virtual void channel_pressure(Duration dt, Channel channel, uint8_t pressure) override;

		virtual void pitch_wheel_change(Duration dt, Channel channel, uint16_t value) override;

		virtual void meta(Duration dt, uint8_t type, std::unique_ptr<uint8_t[]> data, uint64_t data_size) override;

		virtual void sysex(Duration dt, std::unique_ptr<uint8_t[]> data, uint64_t data_size) override;

	};


	struct NoteCollector : public EventReceiver {

		EventMulticaster EM;
		NoteCollector(std::function<void(const NOTE&)> note_receiver) : EM(generate_channels(note_receiver)){}

	public:
		//static std::vector<std::shared_ptr<midi::EventReceiver>> generate_channels(std::function<void(const NOTE&)> note_receiver);
		static std::vector<std::shared_ptr<midi::EventReceiver>> generate_channels(std::function<void(const NOTE&)> note_receiver)
		{
			std::vector<std::shared_ptr<midi::EventReceiver>> receivers;
			for (int i = 0; i < 16; i++) {
				receivers.push_back(std::make_shared<ChannelNoteCollector>(Channel(i), note_receiver));
			}

			return receivers;
		}

		// Inherited via EventReceiver
		virtual void note_on(Duration dt, Channel channel, NoteNumber note, uint8_t velocity) override;

		virtual void note_off(Duration dt, Channel channel, NoteNumber note, uint8_t velocity) override;

		virtual void polyphonic_key_pressure(Duration dt, Channel channel, NoteNumber note, uint8_t pressure) override;

		virtual void control_change(Duration dt, Channel channel, uint8_t controller, uint8_t value) override;

		virtual void program_change(Duration dt, Channel channel, Instrument program) override;

		virtual void channel_pressure(Duration dt, Channel channel, uint8_t pressure) override;

		virtual void pitch_wheel_change(Duration dt, Channel channel, uint16_t value) override;

		virtual void meta(Duration dt, uint8_t type, std::unique_ptr<uint8_t[]> data, uint64_t data_size) override;

		virtual void sysex(Duration dt, std::unique_ptr<uint8_t[]> data, uint64_t data_size) override;


		

	};

	
	std::vector<NOTE> read_notes(std::istream& in);


}

