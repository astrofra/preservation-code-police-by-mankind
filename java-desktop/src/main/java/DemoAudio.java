import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;

/** Desktop replacement for AudioClip; kSound keeps its recovered command logic. */
public final class DemoAudio implements AutoCloseable {
    public record Event(String resource, String command) { }
    public final String resource;
    public final AudioFormat format;
    public final byte[] pcm;
    private final List<Event> events;
    private Clip clip;
    private boolean closed;

    public DemoAudio(URL url, boolean silent, List<Event> events) {
        this.resource = url.getPath().substring(url.getPath().lastIndexOf('/') + 1);
        this.events = events;
        try (AudioInputStream source = AudioSystem.getAudioInputStream(url)) {
            AudioFormat input = source.getFormat();
            format = new AudioFormat(AudioFormat.Encoding.PCM_SIGNED, input.getSampleRate(),
                    16, input.getChannels(), input.getChannels() * 2, input.getSampleRate(), false);
            try (AudioInputStream decoded = AudioSystem.getAudioInputStream(format, source)) {
                pcm = decoded.readAllBytes();
            }
            if (!silent) {
                clip = AudioSystem.getClip();
                clip.open(format, pcm, 0, pcm.length);
            }
        } catch (Exception exception) {
            if (clip != null) clip.close();
            throw new IllegalStateException("Cannot load/play audio: " + url, exception);
        }
    }

    public synchronized void play() { command("play", false); }
    public synchronized void loop() { command("loop", true); }
    private void command(String command, boolean loop) {
        if (closed) return;
        if (events != null) events.add(new Event(resource, command));
        if (clip != null) {
            clip.stop();
            clip.setFramePosition(0);
            clip.loop(loop ? Clip.LOOP_CONTINUOUSLY : 0);
        }
    }
    public synchronized void stop() {
        if (closed) return;
        if (events != null) events.add(new Event(resource, "stop"));
        if (clip != null) clip.stop();
    }
    public synchronized boolean isRunning() { return clip != null && clip.isRunning(); }
    public synchronized long framePosition() { return clip == null ? 0 : clip.getLongFramePosition(); }
    @Override public synchronized void close() {
        if (clip != null) { clip.stop(); clip.close(); }
        closed = true;
    }
}
