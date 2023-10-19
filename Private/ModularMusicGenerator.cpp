extern "C" {
}
#include "ModularMusicGenerator.h"

#include "SimpleBeatLayer.h"
#include "IntensityLayer.h"
#include "SimpleMIDIPlayer.h"
#include "Metronome.h"

#include <iostream>

#include <filesystem>
#include <set>
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
#include "AdvancedMIDIParser.h"
#include "MIDIParserException.h"
#include "LoggingMIDIParser.h"

static fluid_synth_t* synth;
static TaskScheduler timeScheduler;
static int undertalesfID;


 class MIDIParserChild : public AdvancedMIDIParser
 {
 public:
     using Super = AdvancedMIDIParser;
     std::set<int> programIDs;

     virtual void OnSysEventLoaded(uint32_t deltaTime, SysexEvent& sysEvent) override 
     {
         Super::OnSysEventLoaded(deltaTime, sysEvent);

         cout << "track-sysex" << '\n';
         cout.stream << "  time: " << deltaTime << std::endl;
     } 
     virtual void OnMetaEventLoaded(uint32_t deltaTime, MetaEvent& metaEvent) override 
     {
         Super::OnMetaEventLoaded(deltaTime, metaEvent);

         cout << "track-meta\n";
         cout << "   time: " << deltaTime << '\n';
         cout << "   type: " << (int) metaEvent.type << "[" << MidiMetaToStr(metaEvent.type) << "]\n"; 
         cout.stream << "   length: " << (int) metaEvent.length << std::endl; 
     }

     virtual void OnChannelEventLoaded(uint32_t deltaTime, ChannelEvent& channelEvent, bool isOpti) override
     {
         AdvancedMIDIParser::OnChannelEventLoaded(deltaTime, channelEvent, isOpti);

         cout << "track-midi" << '\n';
         cout << "  time: " << deltaTime << '\n';
         cout << "  status: " << (int) channelEvent.message << "[" <<  ENoteEventToStr(channelEvent.message) << "]" << '\n';
         cout << "  channel: " << (int) channelEvent.channel << '\n';
         cout << "  param1: " << (int) channelEvent.param1 << '\n';
         cout.stream << "  param2: " << (int) channelEvent.param2 << std::endl;
     }

     virtual void OnNoteOn(int channel, int key, int velocity) 
     {
         timeScheduler.RunAt(timePerTrack[currentTrackIndex] * (tempo / ticksPerQuarterNote) / 1000, [channel, key, velocity]()
         {
 //             // if (channelEvent.param1 > 55 && channelEvent.param1 < 62)
 //             // if (channelEvent.param1 > 50 && channelEvent.param1 !=69 && channelEvent.channel < 7)
 // fluid_synth_program_select(synth, 0, undertalesfID, 0, 64);
             fluid_synth_noteon(synth, channel, key, velocity);
         });
     }
     virtual void OnNoteOff(int channel, int key) 
     {
         timeScheduler.RunAt(timePerTrack[currentTrackIndex] * (tempo / ticksPerQuarterNote) / 1000, [channel, key]()
         {
             fluid_synth_noteoff(synth, channel, key);
         });
     }
     virtual void OnProgramChange(int channel, int program) 
     {
         timeScheduler.RunAt(timePerTrack[currentTrackIndex] * (tempo / ticksPerQuarterNote) / 1000, [channel, program]()
         {
             fluid_synth_program_change(synth, channel, program);
         });
     }
     virtual void OnControlChange(int channel, EControlChange ctrl, int value) 
     {
         timeScheduler.RunAt(timePerTrack[currentTrackIndex] * (tempo / ticksPerQuarterNote) / 1000, [channel, ctrl, value]()
         {
             fluid_synth_cc(synth, channel, (int)ctrl, value);
         });
     }
     virtual void OnPitchBend(int channel, int value) 
     {
         timeScheduler.RunAt(timePerTrack[currentTrackIndex], [channel, value]()
         {
             fluid_synth_pitch_bend(synth, channel, value);
         });
     }


     virtual void OnKeySignature(uint8_t sf, uint8_t mi) override
     {
         // std::cout << "sf : " << (int)sf << std::endl;
         // if (mi == 0)
         //     std::cout << "mi : major" << std::endl;
         // else if (mi == 1)
         //     std::cout << "mi : minor" << std::endl;
         // else
         //     std::cout << "mi : " << (int)mi << std::endl;
     }

     virtual void OnText(const char* text, uint32_t length) override
     {
         // std::cout << "OnText : " << std::string(text, length) << std::endl;
     }
     virtual void OnCopyright(const char* copyright, uint32_t length) override
     {
         // std::cout << "OnCopyright : " << std::string(copyright, length) << std::endl;
     }
     virtual void OnTrackName(const char* trackName, uint32_t length) override
     {
         // std::cout << "OnTrackName : " << std::string(trackName, length) << std::endl;
     }
     virtual void OnInstrumentName(const char* instrumentName, uint32_t length) override
     {
         // std::cout << "OnInstrumentName : " << std::string(instrumentName, length) << std::endl;
     }
     virtual void OnLyric(const char* lyric, uint32_t length) override
     {
         // std::cout << "OnLyric : " << std::string(lyric, length) << std::endl;
     }
     virtual void OnMarker(const char* markerName, uint32_t length) override
     {
         // std::cout << "OnMarker : " << std::string(markerName, length) << std::endl;
     }
     virtual void OnCuePoint(const char* cuePointName, uint32_t length) override
     {
         // std::cout << "OnCuePoint : " << std::string(cuePointName, length) << std::endl;
     }
 };

void displayError(const std::string& s)
{
    std::cout << "\033[1;31m" << s << "\033[0m\n" << std::endl;    
}

void displaySuccess(const std::string& s)
{
    std::cout << "\033[1;32m" << s << "\033[0m\n" << std::endl;    
}

void TryLoadAllFiles()
{
    int nbFailures = 0;

    int i = 0;
    std::string path = "C:/Users/thoma/Downloads/archive";
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
    {
        if (entry.is_directory() || entry.path().extension().string() != ".mid")
            continue;

        std::cout << "Trying to load : " << i << " / " << entry.path().string() << std::endl;
        i++;
        if (i < 151)
            continue;

        try 
        {

            LoggingMIDIParser parser("output.txt");

            std::ifstream file (entry.path(), std::ios::in|std::ios::binary|std::ios::ate);
            if (file.is_open())
            {
                size_t size = file.tellg();
                char* memblock = new char [size];
                file.seekg (0, std::ios::beg);
                file.read (memblock, size);
                file.close();

                parser.LoadFromBytes(memblock, size);
                // parser.OnLoadedFromFile(filename);

                delete[] memblock;
            }
            else
            {
                throw std::runtime_error("Couldn't open file : " + entry.path().string());
            }

            displaySuccess("Loaded with success! " + entry.path().string());
            // std::cout << "Loaded with success! " << entry.path().string() << std::endl;
        }
        catch (const MIDIParserException& e)
        {
            displayError("MIDIParserException : " + std::string(e.what()));
            // std::cout << "MIDIParserException : " << e.what() << std::endl;
        }
        catch (const std::exception& e)
        {
            displayError("std::exception : " + std::string(e.what()));
            // std::cout << "std::exception : " << e.what() << std::endl;
        }
    }
}

void InputLoop()
{
    while (true)
    {
        std::string input;
        std::cin >> input;

        if (input[0] == '>')
        {
           std::string subStr = input.substr(1, input.length() - 1);
           int nbSeconds = std::stoi(subStr);
        }
    }


}

void MyCFunc()
{
    // std::thread first (TryLoadAllFiles);
    // // TryLoadAllFiles();
    // InputLoop();

    // return;
    // // v();

    // Create the FluidSynth settings and synthesizer objects
    fluid_settings_t* settings = new_fluid_settings();
    // fluid_synth_t* synth = new_fluid_synth(settings);
    synth = new_fluid_synth(settings);

    // Create an audio driver
    fluid_audio_driver_t* adriver = new_fluid_audio_driver(settings, synth);

    // Increase the volume (gain) for the entire synth
    double volume = 0.1; // Adjust the volume level (1.0 is the default, higher values increase the volume)
    fluid_settings_setnum(settings, "synth.gain", volume);

    // Load a SoundFont file
    // const char* soundfontPath = "C:/Users/thoma/Downloads/EarthBound__Unused_Instruments_.sf2";
    // const char* soundfontPath = "C:/Users/thoma/Downloads/undertale.sf2";
    // int sfID = fluid_synth_sfload(synth, soundfontPath, 1);

    undertalesfID = fluid_synth_sfload(synth, "C:/Users/thoma/Downloads/undertale.sf2", 1);
    int sfID = fluid_synth_sfload(synth, "C:/Users/thoma/Downloads/Touhou.sf2", 1);
    int basssfID = fluid_synth_sfload(synth, "C:/Users/thoma/Downloads/ColomboGMGS2__SF2_/ColomboGMGS2.sf2", 1);

            std::cout << "===================" << std::endl;

            // parse_file("C:/Users/thoma/Downloads/Never-Gonna-Give-You-Up-1.mid");
            // parse_file("C:/Users/thoma/Downloads/bach/bach_847.mid");
            // parse_file("C:/Users/thoma/Downloads/archive/Lucifer/Selfpity.mid");
            // return;

    MIDIParserChild parser;
    try 
    {
        // parser.LoadFromFile("C:/Users/thoma/Downloads/Never-Gonna-Give-You-Up-1.mid");
        // parser.LoadFromFile("C:/Users/thoma/Downloads/midi archive/E-Piano MIDI (2).mid");

        // parser.LoadFromFile("C:/Users/thoma/Downloads/midi archive/Rhodes MIDI (2).mid");

        // parser.LoadFromFile("C:/Users/thoma/Downloads/midi archive/Cymatics - Lofi MIDI 2 - C Maj.mid");
        // parser.LoadFromFile("C:/Users/thoma/Downloads/midi archive/Cymatics - Lofi MIDI 1 - C Maj.mid");

        //  parser.LoadFromFile("C:/Users/thoma/Downloads/bach/bach_850.mid");
        // parser.LoadFromFile("C:/Users/thoma/Downloads/bach/bach_847.mid");

        // parser.LoadFromFile("C:/Users/thoma/Downloads/al_adagi.mid"); // CRASH
        // parser.LoadFromFile("C:/Users/thoma/Downloads/Hooters_-_And_We_Danced.mid");

        // http://www.madore.org/~david/music/midi/
        // parser.LoadFromFile("C:/Users/thoma/Downloads/turc.mid");
        // parser.LoadFromFile("C:/Users/thoma/Downloads/rulebrit.mid");
        // parser.LoadFromFile("C:/Users/thoma/Downloads/legdam10.mid");

        // parser.LoadFromFile("C:/Users/thoma/Downloads/lofi-record (4).mp3.mid");
        // parser.LoadFromFile("C:/Users/thoma/Downloads/e-gmd-v1.0.0-midi/e-gmd-v1.0.0/drummer1/session1/149_latin-brazilian-baiao_95_fill_4-4_3.midi");
        // parser.LoadFromFile("C:/Users/thoma/Downloads/e-gmd-v1.0.0-midi/e-gmd-v1.0.0/drummer9/session1/24_rock_90_beat_4-4_23.midi");
    
        std::string s = "C:/Users/thoma/Downloads/archive/Ludwig_van_Beethoven/";
        std::string s2 = "C:/Users/thoma/Downloads/archive/";
        // parser.LoadFromFile((s + "Fur_Elise.2.mid").c_str());
        parser.LoadFromFile("C:/Users/thoma/Downloads/archive/Bach_Johann_Sebastian/Toccata_and_Fugue_in_D_minor,_BWV_565.1.mid");

        // parser.LoadFromFile("C:/Users/thoma/Downloads/archive/Zappa/The_Black_Page_1.1.mid");
        // parser.LoadFromFile((s + "Fur_Elise.1.mid").c_str());
        // parser.LoadFromFile((s + "Piano_Sonata_No._14_in_C-sharp_minor,_Op._27_No._2_Moonlight_I._Adagio_sostenuto.mid").c_str());
    
        // parser.LoadFromFile((s + "Ode_to_Joy_from_the_9th_Symphony.mid").c_str());
        // parser.LoadFromFile((s + "Piano_Sonata_No._14_in_C-sharp_minor,_Op._27_No._2_Moonlight_I._Adagio_sostenuto.mid").c_str());
        // parser.LoadFromFile((s2 + "Lucifer/Selfpity.mid").c_str());

    }
    catch (const MIDIParserException& e)
    
    {
        std::cout << "ERROR : " << e.what() << std::endl;
        return;
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR : " << e.what() << std::endl;
        return;
    }

    // for (const auto& instru : parser.programIDs)
    // {
    //     std::cout << instru << std::endl;
    // }

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
        // fluid_player_add(player2, "C:/Users/thoma/Downloads/Never-Gonna-Give-You-Up-1.mid");
        fluid_player_add(player2, "C:/Users/thoma/Downloads/archive/Lucifer/Selfpity.mid");
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

        int undertalesfID2 = undertalesfID;
        auto addRhodes = [&player, undertalesfID2](int key, float timeAdded, float duration)
        {
            FullNote fullNote;
            fullNote.fontID = undertalesfID2;
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

