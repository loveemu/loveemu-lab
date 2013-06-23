package scm3Mus;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.LinkedList;

public class DC2 implements SCM3LT
{
    final static int KEY_OFFSET = 15;

    RandomAccessFile inGBA;
    MIDI midi;
    int[] track_ptr;
    int[] counter;
    boolean[] note_flag;
    int[] current_key;

    LinkedList<Integer> pending_note_on_chn;
    LinkedList<Integer> pending_note_on_key;
    LinkedList<Integer> pending_note_on_vel;

    boolean[] track_completed;

    public DC2()
    {
    }

    public boolean init() throws IOException
    {
        //Copy pointers from the main class
        inGBA = Main.inGBA;
        midi = Main.midi;
        track_ptr = Main.track_ptr = new int[MAX_TRACK_COUNT];
        counter = Main.counter = new int[MAX_TRACK_COUNT];
        note_flag = Main.note_flag = new boolean[MAX_TRACK_COUNT];
        current_key = Main.current_key = new int[MAX_TRACK_COUNT];

        pending_note_on_chn = Main.pending_note_on_chn = new LinkedList<Integer>();
        pending_note_on_key = Main.pending_note_on_key = new LinkedList<Integer>();
        pending_note_on_vel = Main.pending_note_on_vel = new LinkedList<Integer>();

        track_completed = Main.track_completed = new boolean[MAX_TRACK_COUNT];

        for(int i=0; i<MAX_TRACK_COUNT; i++)
        {
            track_ptr[i] = Integer.reverseBytes(inGBA.readInt());

            // simple range check
            if (track_ptr[i] < 0 || track_ptr[i] + Main.song_base_offset >= 0x2000000)
            {
                return false;
            }
        }

        // Print pointers
        for(int i=0; i<MAX_TRACK_COUNT; i++)
        {
            System.out.println(String.format("Track %1$d starts from %2$08x", i, Main.song_base_offset + track_ptr[i]));
        }

        return true;
    }

    public void process_event(int track) throws IOException
    {
        int event_addr = track_ptr[track] + Main.song_base_offset;

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

        inGBA.seek(event_addr);

        //Read command
        int command = inGBA.read();
        track_ptr[track]++;

        if(command == 0x00)
        {
            //End track command
            if(note_flag[track])
                midi.addNoteOff(track, current_key[track], 0);
            track_completed[track] = true;
            return;
        }
        else if(command >= 0x01 && command <= 0x7f)
        {
            if(command == 0x7f)
            {
                // rest
                if(note_flag[track])
                    midi.addNoteOff(track, current_key[track], 0);
                note_flag[track] = false;
            }
            else
            {
                // note
                if(note_flag[track])
                    midi.addNoteOff(track, current_key[track], 0);

                current_key[track] = command + KEY_OFFSET;

                pending_note_on_chn.add(track);
                pending_note_on_key.add(current_key[track]);
                pending_note_on_vel.add(100);
                note_flag[track] = true;
            }

            int lengthByte = inGBA.read();
            int length = 0;
            while((lengthByte & 0x80) != 0)
            {
                length = (length << 7) | (lengthByte & 0x7f);
                lengthByte = inGBA.read();
                track_ptr[track]++;
            }
            length = (length << 7) | (lengthByte & 0x7f);
            track_ptr[track]++;
            counter[track] = length;
        }
        //Commands
        else
        {
            int arg1 = inGBA.read();
            int arg2 = inGBA.read();
            int arg3 = inGBA.read();
            int arg4 = inGBA.read();
            int arg5 = inGBA.read();
            int arg6 = inGBA.read();
            //track_ptr[track] += argTbl[command-0xc4]; // TODO
            switch(command)
            {
                case 0x80:
                    //simple volume change
                    midi.addController(track, 7, arg1 * 4);
                    track_ptr[track] += 1;
                    return;

                case 0x81:
                    midi.addMarker("event 81 (" + arg1 + ", " + arg2 + ")");
                    track_ptr[track] += 2;
                    return;

                case 0x82:
                    midi.addMarker("event 82 (" + arg1 + ")");
                    track_ptr[track] += 1;
                    return;

                case 0x83:
                    midi.addMarker("event 83");
                    return;

                case 0x84:
                    midi.addMarker("event 84");
                    return;

                case 0x85:
                    midi.addMarker("event 85");
                    return;

                case 0x86:
                    midi.addMarker("event 86");
                    return;

                case 0x87:
                    midi.addTempoChange(arg1 * 1.18); // TODO: 1.18 is just a random value
                    track_ptr[track] += 1;
                    return;

                case 0x88:
                    midi.addMarker("event 88 (" + arg1 + ")");
                    track_ptr[track] += 1;
                    return;

                case 0x8c:
                    midi.addMarker("event 8c");
                    return;

                case 0x90:
                    midi.addMarker("event 90 (" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg5 + ", " + arg6 + ")");
                    track_ptr[track] += 6;
                    return;

                case 0x94:
                    midi.addMarker("event 94 (" + arg1 + ", " + arg2 + ")");
                    track_ptr[track] += 2;
                    return;

                case 0x96:
                    midi.addMarker("event 96 (" + arg1 + ")");
                    track_ptr[track] += 1;
                    return;

                case 0x97:
                    midi.addMarker("event 97 (" + arg1 + ")");
                    track_ptr[track] += 1;
                    return;

                case 0x98:
                    midi.addMarker("event 98");
                    return;

                case 0x99:
                    midi.addMarker("event 99");
                    return;

                case 0x9a:
                    midi.addMarker("event 9a");
                    return;

                case 0xa0:
                    midi.addMarker("event a0 (" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg4 + ")");
                    track_ptr[track] += 4;
                    return;

                case 0xa1:
                    midi.addMarker("event a1 (" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg4 + ")");
                    track_ptr[track] += 4;
                    return;

                case 0xa5:
                    midi.addMarker("event a5 (" + arg1 + ", " + arg2 + ")");
                    track_ptr[track] += 2;
                    return;

                case 0xa6:
                    midi.addMarker("event a6 (" + arg1 + ", " + arg2 + ")");
                    track_ptr[track] += 2;
                    return;

                case 0xab:
                    midi.addMarker("event ab");
                    return;

                case 0xc0:
                    midi.addMarker("event c0 (" + arg1 + ")");
                    track_ptr[track] += 1;
                    return;

                case 0xc1:
                    midi.addMarker("event c1 (" + arg1 + ", " + arg2 + ")");
                    track_ptr[track] += 2;
                    return;

                case 0xc2:
                    midi.addMarker("event c2 (" + arg1 + ", " + arg2 + ")");
                    track_ptr[track] += 2;
                    return;

                case 0xc8:
                    midi.addMarker("event c8");
                    return;

                case 0xcf:
                    midi.addMarker("Loop Start");
                    return;

                case 0xf1:
                    midi.addMarker("event f1 (" + arg1 + ")");
                    track_ptr[track] += 1;
                    return;

                case 0xf2:
                    midi.addMarker("event f2 (" + arg1 + ")");
                    track_ptr[track] += 1;
                    return;

                case 0xf3:
                    midi.addMarker("event f3 (" + arg1 + ", " + arg2 + ")");
                    track_ptr[track] += 2;
                    return;

                case 0xff:
                    midi.addMarker("event ff");
                    return;

                default :
                    midi.addMarker(String.format("Track %1$d : ignored command 0x%2$02X at %3$08x", track, command, event_addr));
                    System.out.println(String.format("Track %1$d : Ignored command 0x%2$02X at %3$08x", track, command, event_addr));

                    //End track for the safe
                    if(note_flag[track])
                        midi.addNoteOff(track, current_key[track], 0);
                    track_completed[track] = true;
            }
        }
    }

}
