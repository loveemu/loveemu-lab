package scm3Mus;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.Arrays;
import java.util.LinkedList;

/**
 * SCM3LT to MIDI converter by loveemu
 * Original source (c) 2012 by Bregalad
 * This is free and open source software
 */

public class Main
{
    final static byte[] MCM_SIGNATURE = { 'M', 'C', 'M', 0 };

    //Output MIDI file
    static DataOutputStream outMID;
    //Input ROM file
    static RandomAccessFile inGBA;

    //Offset of MCM music archive data in ROM
    static int mcm_offset;
    //Offset of song header table in ROM
    static int song_hdr_table_offset;
    //Offset from logical address in song to actual address in ROM
    static int song_base_offset;
    //Number of songs
    static int num_songs;

    //Pointers to multiple track data
    static int[] track_ptr;
    //Counters until next event comes on a track
    static int[] counter;
    //True if a note is currently playing
    static boolean[] note_flag;
    //Number of the MIDI key of the note currently playing
    static int[] current_key;

    //True if the entire track has been decoded at least once
    static boolean[] track_completed;

    //FIFO for note on events
    static LinkedList<Integer> pending_note_on_chn;
    static LinkedList<Integer> pending_note_on_key;
    static LinkedList<Integer> pending_note_on_vel;

    //Linearise volume flag (currently unused)
    static boolean lv = false;

    //Which sound engine version is used
    static SCM3LT engine_ver;
    //Output midi stream
    static MIDI midi;

    public static void main(String[] args) throws Exception
    {
        System.out.println("SCM3LT to MIDI ripper");

        parseArguments(args);

        byte[] signature = new byte[4];
        inGBA.seek(mcm_offset);
        inGBA.read(signature);
        if (!Arrays.equals(signature, MCM_SIGNATURE))
        {
            System.out.println(String.format("Invalid MCM signature at %1$08x", mcm_offset));
            System.exit(-1);
        }

        int numHdrTables;
        System.out.println();
        System.out.println(String.format("MCM Header Start at %1$08x", mcm_offset));
        System.out.println(String.format("0004: %1$d", Integer.reverseBytes(inGBA.readInt())));
        System.out.println(String.format("0008: %1$d", Integer.reverseBytes(inGBA.readInt())));
        numHdrTables = Integer.reverseBytes(inGBA.readInt());
        System.out.println(String.format("000c: %1$d", numHdrTables));
        System.out.println(String.format("0010: %1$d", Integer.reverseBytes(inGBA.readInt())));
        inGBA.skipBytes(4 * numHdrTables);
        num_songs = Integer.reverseBytes(inGBA.readInt());
        if (num_songs > 1024) // for safe
        {
            System.out.println(String.format("Too many songs - %1$d", num_songs));
            System.exit(-1);
        }

        for(int song=0; song < num_songs; song++)
        {
            int rom_offset = song_hdr_table_offset + 0x10 + (song * 0x60);
            inGBA.seek(rom_offset);

            //Output MIDIs are in a new folder called "music"
            File dir = new File("music");
            dir.mkdir();

            System.out.println();
            System.out.println("Converting song " + song + "...");

            midi = new MIDI(24);

            midi.addMarker("Converted by Scm3Mus");
            //GS reset
            midi.addSysex(new byte[] {0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7f, 0x00, 0x41});

            //Exit if there is no song to be converted anymore
            if(!engine_ver.init()) break;

            //Reset counters
            for(int i=0; i<counter.length; i++)
                counter[i] = 0;

            //Open output file once we know the pointer points to correct data
            //(this avoids creating blank files when there is an error)
            try
            {
                outMID = new DataOutputStream(new FileOutputStream("music\\" + song + ".mid"));
            }
            catch (FileNotFoundException e)
            {
                System.out.println("Invalid output file.");
                System.exit(-1);
            }

            //This is the main loop which will process all channels
            //until they are all inactive
            int i = 100000;
            try
            {
                while(tick())
                {
                    //This is a security, if we somehow miss the end of the song,
                    //or if we are ripping garbage data, we will eventually exit the loop
                    if(i-- == 0)
                    {
                        System.out.println("Time out");
                        break;
                    }
                }
            }
            catch(Exception e)
            {
                System.out.println("An exception occured");
                continue;
            }
            System.out.println("Dump complete. Now outputing MIDI file...");
            midi.write(outMID);
        }
        //Close files
        inGBA.close();
        System.out.println(" Done !");
    }

    static boolean tick() throws IOException
    {
        //Process all tracks
        for(int track = 0; track<counter.length; track++)
        {
            if (counter[track] > 0)
            {
                counter[track]--;
            }
            //Process events until counter non-null or pointer null
            //This might not be executed if counter both are non null.
            while(!track_completed[track] && counter[track] <= 0)
            {
                engine_ver.process_event(track);
            }
        }

        for(int track = 0; track<counter.length; track++)
        {
            // process portamento, fade, etc.
        }

        //Compute if all still active channels are completely decoded
        boolean all_completed_flag = true;
        for(int i = 0; i < track_ptr.length; i++)
            all_completed_flag &= track_completed[i];

        //If everything is completed, the main program should quit its loop
        if(all_completed_flag)
            return false;

        //Add pending note ons after all events are processed
        while(!pending_note_on_key.isEmpty())
            midi.addNoteOn(pending_note_on_chn.removeFirst(), pending_note_on_key.removeFirst(), pending_note_on_vel.removeFirst());

        //Increment MIDI time
        midi.time_ctr++;
        return true;
    }

    static void parseArguments(String[] args)
    {
        if (args.length < 2)
            printInstructions();

        String game = args[0];
        if(game.equals("DC2"))
        {
            mcm_offset = 0x4d4b44;
            song_hdr_table_offset = 0x4d62c0;
            song_base_offset = song_hdr_table_offset + 0x7110;
            engine_ver = new DC2();
        }
        else
        {
            //If an invalid ROM is given
            System.out.println("Invalid ROM code : " + args[0]);
            printInstructions();
        }

        //Open the input and output files
        try
        {
            inGBA = new RandomAccessFile(args[1], "r");
        }
        catch(FileNotFoundException e)
        {
            System.out.println("Invalid input GBA file : " + args[1]);
            System.exit(-1);
        }

        for(int i=2; i<args.length; i++)
        {
            if(args[i].equals("-lv"))
                lv = true;
            else
                printInstructions();
        }
    }

    static void printInstructions()
    {
        System.out.println("Rips sequence data from SCM3LT to MIDI (.mid) format.");
        System.out.println("\nUsage : Main [rom] infile.gba");
        System.out.println("Possible ROMs : ");
        System.out.println("DC2 : 1573 - Digi Communication 2 in 1 Datou! Black Gemagema Dan (J).gba");
        System.out.println("-lv : Linearise volume and velocities. This should be used to have the output \"sound\" like the original song," +
                           "but shouldn't be used to get an exact dump of sequence data.");
        System.exit(-1);
    }
}
