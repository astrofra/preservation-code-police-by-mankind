import java.awt.EventQueue;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.awt.image.PixelGrabber;
import java.lang.reflect.Field;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.List;
import javax.imageio.ImageIO;

/** Runs the original and restored renderer in lockstep, including sequential particle state. */
public final class CompareOriginal {
    static final Class<?>[] NO_ARGS = new Class<?>[0];
    static int[] render(Object demo, int time) throws Exception {
        Object script = ReferenceSupport.field(demo, "THEkScript");
        Object part = ReferenceSupport.call(script, "part", new Class<?>[]{int.class}, time);
        ReferenceSupport.call(part, "rendah", new Class<?>[]{int.class}, time);
        Object context = ReferenceSupport.field(demo, "RenderContext");
        Image frame = (Image) ReferenceSupport.call(context, "createImage", NO_ARGS);
        int[] pixels = new int[520 * 300];
        if (!new PixelGrabber(frame, 0, 0, 520, 300, pixels, 0, 520).grabPixels(5000))
            throw new AssertionError("Cannot read frame");
        return pixels;
    }
    static void state(Object original, Object restored, String mapName) throws Exception {
        Hashtable<?, ?> left = (Hashtable<?, ?>) ReferenceSupport.field(original, mapName);
        Hashtable<?, ?> right = (Hashtable<?, ?>) ReferenceSupport.field(restored, mapName);
        if (!left.keySet().equals(right.keySet())) throw new AssertionError(mapName + " keys");
        for (Object key : left.keySet()) {
            Object a = left.get(key), b = right.get(key);
            for (Field field : a.getClass().getDeclaredFields()) {
                if (field.isSynthetic() || field.getName().startsWith("this$")) continue;
                if (!(field.getType().isPrimitive() || field.getType().isArray())) continue;
                field.setAccessible(true);
                Object av = field.get(a), bv = ReferenceSupport.field(b, field.getName());
                if (!java.util.Objects.deepEquals(av, bv))
                    throw new AssertionError(mapName + "/" + key + "." + field.getName());
            }
        }
    }
    static void save(Path path, int[] pixels) throws Exception {
        BufferedImage image = new BufferedImage(520, 300, BufferedImage.TYPE_INT_RGB);
        image.setRGB(0, 0, 520, 300, pixels, 0, 520);
        ImageIO.write(image, "png", path.toFile());
    }
    public static void main(String[] args) throws Exception {
        Path root = Path.of(args[0]);
        SceneAssets assets = new SceneAssets(root.resolve("java-desktop/assets"));
        List<DemoAudio.Event> expected = new ArrayList<>(), actual = new ArrayList<>();
        ReferenceSupport[] reference = new ReferenceSupport[1];
        kraycasting restored = new kraycasting();
        EventQueue.invokeAndWait(() -> {
            try {
                reference[0] = new ReferenceSupport(root, assets, expected, false);
                ReferenceSupport.seed(888);
                ReferenceSupport.call(reference[0].demo, "init", NO_ARGS);
                restored.configure(assets, true, actual);
                ReferenceSupport.seed(888);
                restored.init();
            } catch (Exception exception) { throw new RuntimeException(exception); }
        });
        Path captures = root.resolve("java-desktop/build/comparison");
        Files.createDirectories(captures);
        int count = 0;
        int[] captureTimes = {500, 1500, 2500, 3550, 4050, 4500, 6500, 8000, 9500, 11000, 13000};
        try (ReferenceSupport old = reference[0]) {
            // 50 Hz is a controlled test cadence, not a historical playback-speed claim.
            for (int time = 0; time <= 22200; time += 2) {
                ReferenceSupport.seed(100000L + time);
                int[] a = render(old.demo, time);
                ReferenceSupport.seed(100000L + time);
                int[] b = render(restored, time);
                if (!Arrays.equals(a, b)) {
                    save(captures.resolve("mismatch-original-" + time + ".png"), a);
                    save(captures.resolve("mismatch-restored-" + time + ".png"), b);
                    long pixels = java.util.stream.IntStream.range(0, a.length).filter(i -> a[i] != b[i]).count();
                    throw new AssertionError("Pixel mismatch at tick " + time + ": " + pixels);
                }
                if (!expected.equals(actual)) throw new AssertionError("Sound commands at tick " + time + "\n" + expected + "\n" + actual);
                state(old.demo, restored, "CamerHash");
                state(old.demo, restored, "partcHash");
                if (Arrays.binarySearch(captureTimes, time) >= 0) {
                    save(captures.resolve("original-" + time + ".png"), a);
                    save(captures.resolve("restored-" + time + ".png"), b);
                }
                if (time % 1000 == 0) System.out.println("PASS tick " + time + ": pixels, camera, particles and sound commands");
                count++;
            }
            System.out.println("PASS: " + count + " sequential frames matched the original classes exactly; " + actual.size() + " sound commands.");
        } finally { restored.close(); }
    }
}
