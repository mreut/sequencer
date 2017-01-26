/***** Includes *****/

#include "MidiOut.hpp"


/***** Class Methods *****/
MidiOut::MidiOut(
    void)
{
    this->p_handle_ = NULL;
}

MidiOut::~MidiOut(
    void)
{
    if (NULL != this->p_handle_) {
        snd_rawmidi_close(this->p_handle_);
    }
}

int32_t MidiOut::open(
    const char* p_port_name)
{
#ifndef __MIDI_STUB
    int mode = SND_RAWMIDI_SYNC;
    int status = 0;
    
    status = snd_rawmidi_open(NULL, &(this->p_handle_), p_port_name, mode);
    if (0 > status) {
        fprintf(stderr, "Failed to open MIDI output: %s", snd_strerror(status));
        return -1;
    }
#endif
    return 0;
}

int32_t MidiOut::note_on(
    uint8_t note,
    uint8_t velocity)
{
    int status = 0;
    uint8_t message[3] = {0x90, 0, 0};
    
    message[1] = note;
    message[2] = velocity;

#ifndef __MIDI_STUB
    status = snd_rawmidi_write(this->p_handle_, message, 3);
    if (0 > status) {
        return -1;
    }
#endif

    return 0;
}

int32_t MidiOut::note_off(
    uint8_t note,
    uint8_t velocity)
{
    int status = 0;
    uint8_t message[3] = {0x80, 0, 0};
    
    message[1] = note;
    message[2] = velocity;

#ifndef __MIDI_STUB
    status = snd_rawmidi_write(this->p_handle_, message, 3);
    if (0 > status) {
        return -1;
    }
#endif

    return 0;
}
