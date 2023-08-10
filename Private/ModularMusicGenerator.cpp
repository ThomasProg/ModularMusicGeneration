extern "C" {
}
#include "ModularMusicGenerator.h"

#include "SimpleBeatLayer.h"
#include "IntensityLayer.h"
#include "SimpleMIDIPlayer.h"
#include "Metronome.h"

#include <iostream>

#include <fluidsynth.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <map>
#include "SchedulerExample.h"
#include "MIDIParser.h"
#include "TaskScheduler.h"
#include "dump.h"
#include "Debug.h"

static fluid_synth_t* synth;
static TaskScheduler timeScheduler;

class MIDIParserChild : public MIDIParser
{
public:

    // only 3 bytes
    // msPerQuarterNote;
    int tempo =  500000; // 120 bpm by default 
    std::vector<uint32_t> timePerTrack; // in ms
    uint32_t currentTrackIndex = 0;
    int16_t ticksPerQuarterNote;

    virtual void OnTrackLoaded() override
    {
        currentTrackIndex++;
    }

    virtual void OnFileHeaderDataLoaded(FileHeaderData& fileHeaderData) override
    {
        timePerTrack.resize(fileHeaderData.nbTracks);
        ticksPerQuarterNote = fileHeaderData.delta.ticksPerQuarterNote;
    }

    virtual void OnSysEventLoaded(uint32_t deltaTime, SysexEvent& sysEvent) override 
    {
        timePerTrack[currentTrackIndex] += deltaTime;

        cout << "track-sysex" << '\n';
        cout << "  time: " << deltaTime << '\n';
    } 
    virtual void OnMetaEventLoaded(uint32_t deltaTime, MetaEvent& metaEvent) override 
    {
        timePerTrack[currentTrackIndex] += deltaTime;

        cout << "track-meta\n";
        cout << "   time: " << deltaTime << '\n';
        cout << "   type: " << (int) metaEvent.type << "[" << MidiMetaToStr(metaEvent.type) << "]\n"; 
        cout << "   length: " << (int) metaEvent.length << "\n"; 

        switch (metaEvent.type)
        {
            case EMidiMeta::SET_TEMPO:

                tempo = (metaEvent.bytes[0] << 16) + (metaEvent.bytes[1] << 8) + metaEvent.bytes[2];
                break;

            case EMidiMeta::TIME_SIGNATURE:
            {
                assert(metaEvent.length == 4);
                int nominator = metaEvent.bytes[0];
                int denominator = 2 << metaEvent.bytes[1];
                int clocks = metaEvent.bytes[2];
                int notes = metaEvent.bytes[3];
            }
                break;

            case EMidiMeta::END_OF_TRACK:
                break;

            case EMidiMeta::TRACK_NAME:
            {
                std::string trackName((char*)metaEvent.bytes, metaEvent.length);
                break;
            }

            default:
                throw std::runtime_error( MidiMetaToStr(metaEvent.type) );
                break;    
        }
    }
    virtual void OnChannelEventLoaded(uint32_t deltaTime, ChannelEvent& channelEvent, bool isOpti) override 
    {
        timePerTrack[currentTrackIndex] += deltaTime;

        cout << "track-midi" << '\n';
        cout << "  time: " << deltaTime << '\n';
        cout << "  status: " << (int) channelEvent.message << "[" <<  ENoteEventToStr(channelEvent.message) << "]" << '\n';
        cout << "  channel: " << (int) channelEvent.channel << '\n';
        cout << "  param1: " << (int) channelEvent.param1 << '\n';
        cout << "  param2: " << (int) channelEvent.param2 << '\n';

        switch (channelEvent.message)
        {
            case ENoteEvent::NOTE_ON:
                timeScheduler.RunAt(timePerTrack[currentTrackIndex] * (tempo / ticksPerQuarterNote) / 1000, [channelEvent]()
                {
                    fluid_synth_noteon(synth, channelEvent.channel, channelEvent.param1, channelEvent.param2);
                });
                break;

            case ENoteEvent::NOTE_OFF:
                timeScheduler.RunAt(timePerTrack[currentTrackIndex] * (tempo / ticksPerQuarterNote) / 1000, [channelEvent]()
                {
                    fluid_synth_noteoff(synth, channelEvent.channel, channelEvent.param1);
                });
                break;

            case ENoteEvent::PGM_CHANGE:
                timeScheduler.RunAt(timePerTrack[currentTrackIndex] * (tempo / ticksPerQuarterNote) / 1000, [channelEvent]()
                {
                    fluid_synth_program_change(synth, channelEvent.channel, channelEvent.param1);
                });
                break;

            case ENoteEvent::CONTROL_CHANGE:
                timeScheduler.RunAt(timePerTrack[currentTrackIndex] * (tempo / ticksPerQuarterNote) / 1000, [channelEvent]()
                {
                    fluid_synth_cc(synth, channelEvent.channel, channelEvent.param1, channelEvent.param2);
                });
                break;

            case ENoteEvent::PITCH_BEND:
                timeScheduler.RunAt(timePerTrack[currentTrackIndex], [channelEvent]()
                {
                    fluid_synth_pitch_bend(synth, channelEvent.channel, channelEvent.param1);
                });
                break;

            default:
                std::cout <<ENoteEventToStr(channelEvent.message) << std::endl;
                throw std::runtime_error( ENoteEventToStr(channelEvent.message) );
                break;
        }
    }
};

void MyCFunc()
{
    // v();

    // Create the FluidSynth settings and synthesizer objects
    fluid_settings_t* settings = new_fluid_settings();
    // fluid_synth_t* synth = new_fluid_synth(settings);
    synth = new_fluid_synth(settings);

    // Create an audio driver
    fluid_audio_driver_t* adriver = new_fluid_audio_driver(settings, synth);

    // Increase the volume (gain) for the entire synth
    double volume = 0.5; // Adjust the volume level (1.0 is the default, higher values increase the volume)
    fluid_settings_setnum(settings, "synth.gain", volume);

    // Load a SoundFont file
    // const char* soundfontPath = "C:/Users/thoma/Downloads/EarthBound__Unused_Instruments_.sf2";
    // const char* soundfontPath = "C:/Users/thoma/Downloads/undertale.sf2";
    // int sfID = fluid_synth_sfload(synth, soundfontPath, 1);

    int basssfID = fluid_synth_sfload(synth, "C:/Users/thoma/Downloads/ColomboGMGS2__SF2_/ColomboGMGS2.sf2", 1);
    int sfID = fluid_synth_sfload(synth, "C:/Users/thoma/Downloads/Touhou.sf2", 1);
    int undertalesfID = fluid_synth_sfload(synth, "C:/Users/thoma/Downloads/undertale.sf2", 1);

            std::cout << "===================" << std::endl;

            // parse_file("C:/Users/thoma/Downloads/Never-Gonna-Give-You-Up-1.mid");
            // return;

    MIDIParserChild parser;
    try 
    {
        parser.LoadFromFile("C:/Users/thoma/Downloads/Never-Gonna-Give-You-Up-1.mid");
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR : " << e.what() << std::endl;
        return;
    }

    cout.stream.close();

    auto programBeginTime = std::chrono::high_resolution_clock::now();

    double delta_ms;
    uint32_t previousTimeInMs = -1;
    while (1)
    {
        auto frameStartTime = std::chrono::high_resolution_clock::now();

        double time = std::chrono::duration<double, std::milli>(frameStartTime-programBeginTime).count();

        // the work...
        uint32_t timeInMs = uint32_t(time);
        if (timeInMs != previousTimeInMs)
        {
            timeScheduler.Update(timeInMs);
            previousTimeInMs = timeInMs; 
        }

    }

    

    // fluid_synth_program_select(synth, 0, sfID, 0, 64);
    // fluid_synth_noteon(synth, 0, 60, 100); // channel 0, velocity 64
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // fluid_synth_noteoff(synth, 0, 60);

    // Sleep to allow the SoundFont to load
    // std::this_thread::sleep_for(std::chrono::seconds(2));

    // Change instrument
    // fluid_synth_program_change(synth, 0, 5);

    // // Play some notes to generate music
    // int notes[] = {60, 62, 64, 65, 67, 69, 71, 72};
    // for (int note : notes) {
    //     fluid_synth_noteon(synth, 0, note, 120); // channel 0, velocity 64
    //     std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //     fluid_synth_noteoff(synth, 0, note);
    // }

    {


        fluid_player_t* player2 = new_fluid_player(synth);
        fluid_player_add(player2, "C:/Users/thoma/Downloads/Never-Gonna-Give-You-Up-1.mid");
        /* play the midi files, if any */
        fluid_player_play(player2);
        /* wait for playback termination */
        fluid_player_join(player2);



        SimpleMIDIPlayer player;
        player.synth = synth;
        auto addBeat = [&player, sfID](int key, float timeAdded, float duration)
        {
            FullNote fullNote;
            fullNote.fontID = sfID;
            fullNote.channel = 0;
            fullNote.bankID = 128;
            fullNote.presetID = 25;
            fullNote.velocity = 60;
            fullNote.key = key;
            player.AddNote(fullNote, timeAdded, duration);
        };

        auto addRhodes = [&player, undertalesfID](int key, float timeAdded, float duration)
        {
            FullNote fullNote;
            fullNote.fontID = undertalesfID;
            fullNote.channel = 0;
            fullNote.bankID = 0;
            fullNote.presetID = 64;
            fullNote.velocity = 35;
            fullNote.key = key;
            player.AddNote(fullNote, timeAdded, duration);
        };

        auto addBass = [&player, basssfID](int key, float timeAdded, float duration)
        {
            FullNote fullNote;
            fullNote.fontID = basssfID;
            fullNote.channel = 1;
            fullNote.bankID = 0;
            fullNote.presetID = 35;
            fullNote.velocity = 40;
            fullNote.key = key;
            player.AddNote(fullNote, timeAdded, duration);
        };

        Metronome metronome;
        metronome.SetBPM(60);

        for (int i = 0; i < 50; i++)
        {
            addBeat(36, metronome, 1);
            metronome.AddQuarter();
            addBeat(36, metronome, 1);
            metronome.AddQuarter();
            addBeat(40, metronome, 1);
            metronome.AddQuarter();
            metronome.AddQuarter();
        }

        // addBass(56, 1, 1);
        // // addBass(64, 1, 2);

        addRhodes(60, 33, 5);
        addRhodes(64, 33, 5);

        // addBass(56, 4, 4);
        // addRhodes(60, 4, 2);
        // addRhodes(56, 6, 1);
        // addRhodes(52, 7, 1);

        // addRhodes(60, 4, 2);
        // addRhodes(62, 4, 2);

        // addRhodes(64, 4, 2);

        // addRhodes(60, 5, 1);
        // addRhodes(58, 5.5, 1);

        // addRhodes(60, 6, 2);
        // addRhodes(64, 6, 2);

        // addRhodes(64, 6, 4);
        // addRhodes(66, 6, 4);

        

        // addBass(64, 2, 2);

        // addBass(60, 4, 2);
        // addBass(64, 4, 2);


        // for (int i = 0; i < 20; i++)
        // {
        //     addBeat(36, i, 1);
        //     addBeat(36, i+0.25f, 1);
        //     addBeat(40, i+0.5, 1);

        //     // addRhodes(62, i, 1.99);
        //     addRhodes(60, i*2, 1.99);
        //     addRhodes(64, i*2, 1.99);
        //     // addRhodes(62, i+0.5, 1.99);
        //     // addRhodes(60, i+0.75, 1.99);
        //     // addRhodes(62, i+1, 1.99);
        //     // addRhodes(68, i, 1.99);
        // }

        player.time = 30.f;

        player.Play();
        return;
    }


    {
        fluid_synth_program_change(synth, 0, 0);
        fluid_synth_program_select(synth, 0, sfID, 1, 4);
        // fluid_synth_program_select(synth, 0, sfID, 8, 4);

        for (int i = 0; i < 5; i++)
        {
            fluid_synth_noteon(synth, 0, 65, 110); // channel 0, velocity 64
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            fluid_synth_noteoff(synth, 0, 65);
            
            fluid_synth_noteon(synth, 0, 60, 110); // channel 0, velocity 64
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            fluid_synth_noteoff(synth, 0, 60);
        }

    }



    {
        ModularMusicGenerator musicGenerator;

        {
            std::shared_ptr<IntensityLayer> intensityLayer = std::make_shared<IntensityLayer>();

            intensityLayer->Compute(1, 0, 10);

            // fluid_synth_program_change(synth, 0, 35);
            fluid_synth_program_select(synth, 0, sfID, 1, 4);
            // fluid_synth_program_select(synth, 0, sfID, 8, 4);

            for (const auto& v : intensityLayer->intensities)
            {
                std::cout << v.intensity * 200 << '\t' << std::endl;

                fluid_synth_noteon(synth, 0, 65, v.intensity * 120); // channel 0, velocity 64
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                fluid_synth_noteoff(synth, 0, 65);
            }

        }

        // Init Layers
        {
            // std::shared_ptr<SimpleBeatLayer> beatLayer = std::make_shared<SimpleBeatLayer>();

            // musicGenerator.layerManager.AddLayer(beatLayer);

            // std::vector<Note> notes;
            // beatLayer->GetNotes(notes, 0, 4);

            // // Change instrument
            // fluid_synth_program_change(synth, 0, 35);

            // std::map<float, SimpleNote> simpleNotes;
            // for (const Note& note : notes)
            // {
            //     simpleNotes.emplace(note.time, note.GetSimpleNote());
            // }

            // for (const Note& note : notes) {
            //     fluid_synth_noteon(synth, 0, note.note, note.velocity); // channel 0, velocity 64
            //     std::this_thread::sleep_for(std::chrono::milliseconds(int(note.duration * 1000)));
            //     fluid_synth_noteoff(synth, 0, note.note);
            // }
        }
    }

    //musicGenerator.nextChunkNotes = musicGenerator.layerManager.GetFollowingNotes();



    //     std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // for (int note : notes) {
    //     // fluid_synth_noteon(synth, 0, note, 64); // channel 0, velocity 64
    //     // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //     fluid_synth_noteoff(synth, 0, note);
    // }

    // Clean up and release resources
    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}





