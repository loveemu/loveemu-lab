package dmfMus;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.LinkedList;

import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.Sequence;

public class HokutoDMF implements DMF
{
    RandomAccessFile inSEQ;
    Sequence midi;
    int[] track_ptr;
    int[] counter;
    boolean[] note_flag;
    int[] current_key;

    LinkedList<Integer> pending_note_on_chn;
    LinkedList<Integer> pending_note_on_key;
    LinkedList<Integer> pending_note_on_vel;

    boolean[] track_completed;

    public HokutoDMF()
    {
    }

    public boolean init() throws IOException, InvalidMidiDataException
    {
        //Copy pointers from the main class
        inSEQ = Main.inSEQ;
        midi = Main.midi;
        track_ptr = Main.track_ptr = new int[Main.num_tracks];
        counter = Main.counter = new int[Main.num_tracks];
        note_flag = Main.note_flag = new boolean[Main.num_tracks];
        current_key = Main.current_key = new int[Main.num_tracks];

        pending_note_on_chn = Main.pending_note_on_chn = new LinkedList<Integer>();
        pending_note_on_key = Main.pending_note_on_key = new LinkedList<Integer>();
        pending_note_on_vel = Main.pending_note_on_vel = new LinkedList<Integer>();

        track_completed = Main.track_completed = new boolean[Main.num_tracks];

        for(int i=0; i<Main.num_tracks; i++)
        {
            track_ptr[i] = inSEQ.readInt();

            // simple range check
            if (track_ptr[i] < 0 || track_ptr[i] >= 0x200000 || track_ptr[i] >= inSEQ.length())
            {
                return false;
            }
        }

        // Print pointers
        for(int i=0; i<Main.num_tracks; i++)
        {
            midi.getTracks()[i].add(MidiEventCreator.createTrackNameEvent(0, String.format("Track %1$d - %2$08x", i, track_ptr[i])));
            System.out.println(String.format("Track %1$d starts from %2$08x", i, track_ptr[i]));
        }

        return true;
    }

    public void process_event(int track) throws IOException, InvalidMidiDataException
    {
        int event_addr = track_ptr[track];

        ////Argument size table
        //final int[] argTbl =
        //{
        //    1, 2, 1, 2, 2, 3, 0,
        //    3, 0, 2, 0, 1, 0, 0, 0,
        //    0, 0, 0, 1, 0, 0, 1, 1,
        //    1, 1, 1, 1, 1, 1, 0, 1,
        //    0, 0, 0, 0, 0, 1, 1, 1,
        //    0, 0, 0, 0, 0, 1, 2, 1,
        //    2, 2, 1, 3, 2, 0, 0, 0,
        //    0, 0, 0, 0, 0
        //};

        inSEQ.seek(event_addr);

        //Read command
        int command = inSEQ.read();
        track_ptr[track]++;

        if(command == 0x100) // TODO: fix dummy constant
        {
            //End track command
            if(note_flag[track])
            {
                midi.getTracks()[track].add(MidiEventCreator.createNoteOffEvent(Main.midiTick, track, current_key[track]));
            }
            track_completed[track] = true;
            return;
        }
        /*
        else if(command >= 0x01 && command <= 0x7f)
        {
            if(command == 0x7f)
            {
                // rest
                if(note_flag[track])
                {
                    midi.getTracks()[track].add(MidiEventCreator.createNoteOffEvent(Main.midiTick, track, current_key[track]));
                }
                note_flag[track] = false;
            }
            else
            {
                // note
                if(note_flag[track])
                {
                    midi.getTracks()[track].add(MidiEventCreator.createNoteOffEvent(Main.midiTick, track, current_key[track]));
                }

                current_key[track] = command + KEY_OFFSET;
                if (current_key[track] > 127)
                {
                    System.out.println("Warning: key out of range: " + current_key[track] + String.format(" at %1$08x", event_addr));
                    current_key[track] = 127;
                }

                pending_note_on_chn.add(track);
                pending_note_on_key.add(current_key[track]);
                pending_note_on_vel.add(100);
                note_flag[track] = true;
            }

            int lengthByte = inSEQ.read();
            int length = 0;
            while((lengthByte & 0x80) != 0)
            {
                length = (length << 7) | (lengthByte & 0x7f);
                lengthByte = inSEQ.read();
                track_ptr[track]++;
            }
            length = (length << 7) | (lengthByte & 0x7f);
            track_ptr[track]++;
            counter[track] = length;
        }
        */
        //Commands
        else
        {
            /*
            int arg1 = inSEQ.read();
            int arg2 = inSEQ.read();
            int arg3 = inSEQ.read();
            int arg4 = inSEQ.read();
            int arg5 = inSEQ.read();
            int arg6 = inSEQ.read();
            */
            //track_ptr[track] += argTbl[command-0xc4]; // TODO: oplen table
            switch(command)
            {
            /*
                case 0x80:
                    //simple volume change
                    midi.getTracks()[track].add(MidiEventCreator
                        .createControlChangeEvent(Main.midiTick, track, 7, arg1 * 4));
                    track_ptr[track] += 1;
                    return;

                case 0x81:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 81 (" + arg1 + ", " + arg2 + ")"));
                    track_ptr[track] += 2;
                    return;

                case 0x82:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 82 (" + arg1 + ")"));
                    track_ptr[track] += 1;
                    return;

                case 0x83:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 83"));
                    return;

                case 0x84:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 84"));
                    return;

                case 0x85:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 85"));
                    return;

                case 0x86:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 86"));
                    return;

                case 0x87:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTempoEvent(Main.midiTick, arg1 * 1.18)); // TODO: tempo * 1.18 is just a random value
                    track_ptr[track] += 1;
                    return;

                case 0x88:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 88 (" + arg1 + ")"));
                    track_ptr[track] += 1;
                    return;

                case 0x8c:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 8c"));
                    return;

                case 0x90:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 90 (" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg5 + ", " + arg6 + ")"));
                    track_ptr[track] += 6;
                    return;

                case 0x94:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 94 (" + arg1 + ", " + arg2 + ")"));
                    track_ptr[track] += 2;
                    return;

                case 0x96:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 96 (" + arg1 + ")"));
                    track_ptr[track] += 1;
                    return;

                case 0x97:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 97 (" + arg1 + ")"));
                    track_ptr[track] += 1;
                    return;

                case 0x98:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 98"));
                    return;

                case 0x99:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 99"));
                    return;

                case 0x9a:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event 9a"));
                    return;

                case 0xa0:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event a0 (" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg4 + ")"));
                    track_ptr[track] += 4;
                    return;

                case 0xa1:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event a1 (" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg4 + ")"));
                    track_ptr[track] += 4;
                    return;

                case 0xa5:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event a5 (" + arg1 + ", " + arg2 + ")"));
                    track_ptr[track] += 2;
                    return;

                case 0xa6:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event a6 (" + arg1 + ", " + arg2 + ")"));
                    track_ptr[track] += 2;
                    return;

                case 0xab:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event ab"));
                    return;

                case 0xc0:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "repeat start (" + arg1 + ")"));
                    track_ptr[track] += 1;
                    return;

                case 0xc1:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "repeat break (" + arg1 + ", " + arg2 + ")"));
                    track_ptr[track] += 2;
                    return;

                case 0xc2:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "repeat end (" + arg1 + ", " + arg2 + ")"));
                    track_ptr[track] += 2;
                    return;

                case 0xc8:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event c8"));
                    return;

                case 0xcf:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createMarkerEvent(Main.midiTick, "Loop"));
                    return;

                case 0xf1:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event f1 (" + arg1 + ")"));
                    track_ptr[track] += 1;
                    return;

                case 0xf2:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event f2 (" + arg1 + ")"));
                    track_ptr[track] += 1;
                    return;

                case 0xf3:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event f3 (" + arg1 + ", " + arg2 + ")"));
                    track_ptr[track] += 2;
                    return;

                case 0xff:
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, "event ff"));
                    return;
*/
                default :
                    midi.getTracks()[track].add(MidiEventCreator
                        .createTextEvent(Main.midiTick, String.format("Ignored command 0x%1$02X at %2$08x", command, event_addr)));
                    System.out.println(String.format("Track %1$d : Ignored command 0x%2$02X at %3$08x", track, command, event_addr));

                    //End track for the safe
                    if(note_flag[track])
                    {
                        midi.getTracks()[track].add(MidiEventCreator.createNoteOffEvent(Main.midiTick, track, current_key[track]));
                    }
                    track_completed[track] = true;
            }
        }
    }

}
