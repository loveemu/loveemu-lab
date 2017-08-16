/**
 *
 */
package dmfMus;

import java.io.IOException;

import javax.sound.midi.InvalidMidiDataException;

/**
 * This interface is used so that all different DMF formats in different games
 * can be ripped using the same main class
 */
interface DMF
{
    boolean init() throws IOException, InvalidMidiDataException;
    void process_event(int track) throws IOException, InvalidMidiDataException;
}
