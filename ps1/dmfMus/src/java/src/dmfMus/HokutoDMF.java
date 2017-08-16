package dmfMus;

import java.io.IOException;
import java.io.RandomAccessFile;

import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.Sequence;

public class HokutoDMF implements DMF
{
    RandomAccessFile inSEQ;
    Sequence midi;
    int[] track_head;
    int[] track_ptr;
    int[] counter;

    boolean[] track_completed;

    public HokutoDMF()
    {
    }

    public boolean init() throws IOException, InvalidMidiDataException
    {
        //Copy pointers from the main class
        inSEQ = Main.inSEQ;
        midi = Main.midi;
        track_head = Main.track_ptr = new int[Main.num_tracks];
        track_ptr = Main.track_ptr = new int[Main.num_tracks];
        counter = Main.counter = new int[Main.num_tracks];

        //Call-Return stack
        Main.callStackPtr = new int[Main.num_tracks];
        Main.callStack = new int[Main.num_tracks][];

        track_completed = Main.track_completed = new boolean[Main.num_tracks];

        for(int i=0; i<Main.num_tracks; i++)
        {
            track_head[i] = inSEQ.readInt();
            track_ptr[i] = track_head[i];

            // simple range check
            if (track_ptr[i] < 0 || track_ptr[i] >= 0x200000 || track_ptr[i] >= inSEQ.length())
            {
                return false;
            }

            // init call stack
            Main.callStackPtr[i] = 0;
            Main.callStack[i] = new int[Main.CALL_STACK_LENGTH];
        }

        // Print pointers
        for(int i=0; i<Main.num_tracks; i++)
        {
            midi.getTracks()[i].add(MidiEventCreator.createTrackNameEvent(0, String.format("Track %1$d - %2$08x", i, track_ptr[i])));
            System.out.println(String.format("- Track %1$d starts from %2$08x", i, track_ptr[i]));

            // Default bend range
            midi.getTracks()[i].add(MidiEventCreator
                    .createControlChangeEvent(0, i, MidiEventCreator.CONTROL_RPNMSB, 0));
            midi.getTracks()[i].add(MidiEventCreator
                    .createControlChangeEvent(0, i, MidiEventCreator.CONTROL_RPNLSB, 0));
            midi.getTracks()[i].add(MidiEventCreator
                    .createControlChangeEvent(0, i, MidiEventCreator.CONTROL_DATAENTRYMSB, 16));
        }
        System.out.println();

        return true;
    }

    public void process_event(int track) throws IOException, InvalidMidiDataException
    {
        try
        {
            int event_addr = track_ptr[track];

            inSEQ.seek(event_addr);

            //Read command
            int command = inSEQ.read();
            track_ptr[track]++;

            if(command >= 0x00 && command <= 0x7f)
            {
                int key = command;
                int velocity = inSEQ.read();
                int duration = Main.readVarInt(inSEQ);
                track_ptr[track] += 1 + Main.getVarIntLength(duration);

                midi.getTracks()[track].add(MidiEventCreator.createNoteOnEvent(Main.midiTick, track, key, velocity));
                midi.getTracks()[track].add(MidiEventCreator.createNoteOffEvent(Main.midiTick + duration, track, key));
            }
            //Commands
            else
            {
                int arg1;
                switch(command)
                {
                case 0x84:
                    // return
                    if (Main.callStackPtr[track] > 0) {
                        Main.callStackPtr[track]--;
                        track_ptr[track] = Main.callStack[track][Main.callStackPtr[track]];
                    }
                    return;

                case 0x85:
                    // call
                    arg1 = Main.readVarInt(inSEQ);
                    track_ptr[track] += Main.getVarIntLength(arg1);

                    Main.callStack[track][Main.callStackPtr[track]] = track_ptr[track];
                    Main.callStackPtr[track]++;
                    track_ptr[track] = track_head[track] + arg1;

                    // next op, without delta-time
                    process_event(track);
                    return;

                case 0x86: // voice slot assignment?
                    arg1 = inSEQ.read();
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 86 (" + arg1 + ")"));
                    track_ptr[track] += 1;
                    return;

                case 0x89:
                    arg1 = inSEQ.read();
                    midi.getTracks()[track].add(MidiEventCreator
                        .createProgramChangeEvent(Main.midiTick, track, arg1));
                    track_ptr[track] += 1;
                    return;

                case 0x8b:
                    // Pitch Bend
                    arg1 = inSEQ.read();
                    arg1 |= (inSEQ.read() << 8);
                    arg1 -= 0x4000;

                    midi.getTracks()[track].add(MidiEventCreator
                        .createPitchBendEvent(Main.midiTick, track, arg1 / 2));
                    track_ptr[track] += 2;
                    return;

                case 0x8d:
                    arg1 = inSEQ.read();
                    midi.getTracks()[track].add(MidiEventCreator
                            .createControlChangeEvent(Main.midiTick, track, MidiEventCreator.CONTROL_MODULATION, arg1));
                    track_ptr[track] += 1;
                    return;

                case 0x8e:
                    arg1 = inSEQ.read();
                    midi.getTracks()[track].add(MidiEventCreator
                            .createControlChangeEvent(Main.midiTick, track, MidiEventCreator.CONTROL_VOLUME, arg1));
                    track_ptr[track] += 1;
                    return;

                case 0x8f:
                    arg1 = inSEQ.read();
                    midi.getTracks()[track].add(MidiEventCreator
                            .createControlChangeEvent(Main.midiTick, track, MidiEventCreator.CONTROL_PANPOT, arg1));
                    track_ptr[track] += 1;
                    return;

                case 0x90:
                    arg1 = inSEQ.read();
                    midi.getTracks()[track].add(MidiEventCreator
                        .createControlChangeEvent(Main.midiTick, track, MidiEventCreator.CONTROL_EXPRESSION, arg1));
                    track_ptr[track] += 1;
                    return;

                case 0x96:
                    arg1 = inSEQ.read();
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 96 (" + arg1 + ")"));
                    track_ptr[track] += 1;
                    return;

                case 0x98:
                    //Repeat start? (127 for infinite loop)
                    arg1 = inSEQ.read();
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 98 (repeat start) (" + arg1 + ")"));
                    track_ptr[track] += 1;
                    return;

                case 0x99:
                    //Repeat end
                    midi.getTracks()[track].add(MidiEventCreator
                            .createTextEvent(Main.midiTick, "event 99 (repeat end)"));
                    return;

                case 0x9b:
                    //End track command
                    track_completed[track] = true;
                    return;

                case 0x9c:
                    int tempoValue;
                    tempoValue = inSEQ.readShort() & 0xffff;
                    tempoValue = (tempoValue << 8) | inSEQ.read();

                    midi.getTracks()[track].add(MidiEventCreator
                        .createTempoEvent(Main.midiTick, 60000000.0 / tempoValue));
                    track_ptr[track] += 3;
                    return;

                case 0x9d:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 9d"));
                    return;

                default :
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, String.format("Ignored command 0x%1$02X at %2$08x", command, event_addr)));
                    System.out.println(String.format("Track %1$d : Ignored command 0x%2$02X at %3$08x", track, command, event_addr));
                    System.out.println();

                    //End track for the safe
                    track_completed[track] = true;
                }
            }
        }
        catch (InvalidMidiDataException e)
        {
            System.err.println(String.format("Error in Track %1$d at %2$08x", track + 1, track_ptr[track]));
            System.err.println();
            throw e;
        }
    }

}
