import java.awt.Canvas;
import java.awt.Dimension;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.awt.image.ImageObserver;
import java.awt.image.ImageProducer;
import java.awt.image.PixelGrabber;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

/** Small replacement for the applet host. Rendering calculations stay in kraycasting. */
public class DesktopSurface extends Canvas implements AutoCloseable {
    private SceneAssets assets;
    private boolean silent;
    private List<DemoAudio.Event> events;
    private final List<DemoAudio> sounds = new ArrayList<>();

    public void configure(SceneAssets assets, boolean silent, List<DemoAudio.Event> events) {
        this.assets = assets;
        this.silent = silent;
        this.events = events;
        setPreferredSize(new Dimension(520, 300));
        setSize(520, 300);
    }
    public String getParameter(String name) { return "SCRIPT".equals(name) ? assets.script : null; }
    public URL getDocumentBase() { return assets.baseURL(); }
    public Image getImage(URL base, String name) {
        try { return SceneAssets.image(new URL(base, name)); }
        catch (Exception exception) { throw new IllegalStateException(exception); }
    }
    public DemoAudio getAudioClip(URL base, String name) {
        try {
            DemoAudio sound = new DemoAudio(new URL(base, name), silent, events);
            sounds.add(sound);
            return sound;
        } catch (Exception exception) { throw new IllegalStateException(exception); }
    }
    @Override public Image createImage(ImageProducer producer) {
        // Take an immutable snapshot before the engine reuses its two pixel buffers.
        Image image = getToolkit().createImage(producer);
        int[] pixels = new int[520 * 300];
        PixelGrabber grabber = new PixelGrabber(image, 0, 0, 520, 300, pixels, 0, 520);
        try {
            if (!grabber.grabPixels(10000L) || (grabber.getStatus() & ImageObserver.ERROR) != 0)
                throw new IllegalStateException("Cannot publish frame");
        } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
            throw new IllegalStateException("Frame publication interrupted", exception);
        }
        BufferedImage snapshot = new BufferedImage(520, 300, BufferedImage.TYPE_INT_ARGB);
        snapshot.setRGB(0, 0, 520, 300, pixels, 0, 520);
        image.flush();
        return snapshot;
    }
    public List<DemoAudio> audioClips() { return List.copyOf(sounds); }
    @Override public void close() { sounds.forEach(DemoAudio::close); }
}
