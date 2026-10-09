import java.awt.Image;
import java.awt.Toolkit;
import java.awt.Canvas;
import java.awt.MediaTracker;
import java.awt.image.ImageObserver;
import java.awt.image.PixelGrabber;
import java.io.IOException;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/** Platform resource loader. The original script and media remain external files. */
public final class SceneAssets {
    public final Path directory;
    public final String script;

    public SceneAssets(Path directory) throws IOException {
        this.directory = directory.toAbsolutePath().normalize();
        String html = Files.readString(this.directory.resolve("page.html"), StandardCharsets.ISO_8859_1);
        Matcher match = Pattern.compile("<PARAM\\s+NAME=\"SCRIPT\"\\s+VALUE=\"(.*?)\"\\s*>",
                Pattern.CASE_INSENSITIVE | Pattern.DOTALL).matcher(html);
        if (!match.find()) throw new IOException("Missing SCRIPT parameter in page.html");
        script = match.group(1);
    }

    public static SceneAssets installed() throws Exception {
        String explicit = System.getProperty("codepolice.assets");
        if (explicit != null) return new SceneAssets(Path.of(explicit));
        Path location = Path.of(SceneAssets.class.getProtectionDomain().getCodeSource().getLocation().toURI());
        Path base = Files.isDirectory(location) ? location : location.getParent();
        for (Path candidate : new Path[] {base.resolve("assets"), base.resolve("../assets")}) {
            if (Files.isRegularFile(candidate.resolve("page.html"))) return new SceneAssets(candidate);
        }
        throw new IOException("Cannot find assets next to the application: " + base);
    }

    public URL baseURL() {
        try { return directory.toUri().toURL(); }
        catch (Exception exception) { throw new IllegalStateException(exception); }
    }

    public static Image image(URL url) {
        Image image = Toolkit.getDefaultToolkit().createImage(url);
        try {
            MediaTracker tracker = new MediaTracker(new Canvas());
            tracker.addImage(image, 0);
            tracker.waitForID(0, 10000L);
            if (tracker.isErrorID(0) || !tracker.checkID(0))
                throw new IOException("Cannot prepare image: " + url);
            PixelGrabber load = new PixelGrabber(image, 0, 0, -1, -1, false);
            if (!load.grabPixels(10000L) || (load.getStatus() & ImageObserver.ERROR) != 0)
                throw new IOException("Cannot decode image: " + url);
            return image;
        } catch (Exception exception) {
            if (exception instanceof InterruptedException) Thread.currentThread().interrupt();
            throw new IllegalStateException("Image loading failed: " + url, exception);
        }
    }

    public Image image(String relative) {
        try { return image(new URL(baseURL(), relative)); }
        catch (Exception exception) { throw new IllegalStateException(exception); }
    }
}
