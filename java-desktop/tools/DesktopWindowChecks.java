import java.awt.EventQueue;
import java.awt.Frame;
import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.event.MouseEvent;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import javax.imageio.ImageIO;

/** Real window, audio-device and full-loop smoke test on the host machine. */
public final class DesktopWindowChecks {
    public static void main(String[] args) throws Exception {
        Path root = Path.of(args[0]);
        int duration = args.length > 1 ? Integer.parseInt(args[1]) : 226;
        int offset = args.length > 2 ? Integer.parseInt(args[2]) : 0;
        SceneAssets assets = new SceneAssets(root.resolve("java-desktop/assets"));
        Path output = root.resolve("java-desktop/build/window-checks" + (offset == 0 ? "" : "-loop"));
        Files.createDirectories(output);
        List<Throwable> failures = java.util.Collections.synchronizedList(new ArrayList<>());
        Thread.setDefaultUncaughtExceptionHandler((thread, exception) -> { failures.add(exception); exception.printStackTrace(); });
        CodePoliceDesktop.Session[] session = new CodePoliceDesktop.Session[1];
        EventQueue.invokeAndWait(() -> session[0] = CodePoliceDesktop.open(assets, false));
        var window = session[0].window();
        var demo = session[0].demo();
        if (offset != 0) {
            Object timer = ReferenceSupport.field(demo, "kTime");
            var startField = timer.getClass().getDeclaredField("StartTime");
            startField.setAccessible(true);
            startField.setLong(timer, System.currentTimeMillis() - offset * 1000L);
        }
        Robot robot = new Robot();
        long start = System.nanoTime();
        boolean heardActiveLine = false, looped = false;
        int previousPart = -1;
        List<String> observations = new ArrayList<>();
        try {
            for (int second = 0; second <= duration; second++) {
                long remaining = start + second * 1_000_000_000L - System.nanoTime();
                if (remaining > 0) Thread.sleep(remaining / 1_000_000L);
                if (!demo.isRunning()) throw new AssertionError("Engine stopped at second " + second
                        + "; windowDisplayable=" + window.isDisplayable()
                        + "; boolend=" + ReferenceSupport.field(demo, "boolend"));
                if (!failures.isEmpty()) throw new AssertionError("Worker exception", failures.get(0));
                Object published = ReferenceSupport.field(demo, "applet_image");
                Object script = ReferenceSupport.field(demo, "THEkScript");
                Object part = ReferenceSupport.field(script, "lastpartplayed");
                if (part != null) {
                    int partStart = (int) ReferenceSupport.field(part, "TimeLength");
                    if (partStart < previousPart) looped = true;
                    if (partStart != previousPart) {
                        String line = "second=" + second + " partStartTick=" + partStart;
                        System.out.println(line); observations.add(line); previousPart = partStart;
                    }
                }
                for (DemoAudio audio : demo.audioClips()) if (audio.isRunning() && audio.framePosition() > 0) heardActiveLine = true;
                if (second == 7 || second == 27 || second == 47 || second == 87 || second == 117 || second == 137 || second == 224) {
                    Rectangle[] bounds = new Rectangle[1];
                    EventQueue.invokeAndWait(() -> {
                        window.toFront();
                        bounds[0] = new Rectangle(window.getContentPane().getLocationOnScreen(), window.getContentPane().getSize());
                    });
                    Thread.sleep(150);
                    ImageIO.write(robot.createScreenCapture(bounds[0]), "png", output.resolve("screen-" + second + "s.png").toFile());
                    if (published instanceof BufferedImage image)
                        ImageIO.write(image, "png", output.resolve("frame-" + second + "s.png").toFile());
                }
                if (second == 18) {
                    EventQueue.invokeAndWait(() -> {
                        demo.dispatchEvent(new MouseEvent(demo, MouseEvent.MOUSE_MOVED, 0, 0, 100, 100, 0, false));
                        demo.dispatchEvent(new MouseEvent(demo, MouseEvent.MOUSE_PRESSED, 0, 0, 100, 100, 1, false, MouseEvent.BUTTON1));
                        demo.dispatchEvent(new MouseEvent(demo, MouseEvent.MOUSE_RELEASED, 0, 0, 100, 100, 1, false, MouseEvent.BUTTON1));
                        window.setState(Frame.ICONIFIED);
                    });
                }
                if (second == 19) EventQueue.invokeAndWait(() -> { window.setState(Frame.NORMAL); window.toFront(); });
            }
            if (!heardActiveLine) throw new AssertionError("No active audio device observed");
            if (offset + duration >= 224 && !looped) throw new AssertionError("Cycle did not restart");
        } finally {
            EventQueue.invokeAndWait(() -> window.dispatchEvent(new WindowEvent(window, WindowEvent.WINDOW_CLOSING)));
        }
        if (demo.isRunning() || window.isDisplayable()) throw new AssertionError("Window or engine did not close");
        if (demo.audioClips().stream().anyMatch(DemoAudio::isRunning)) throw new AssertionError("Audio remained active");
        String result = "PASS: visible window, " + duration + " seconds, initial timeline offset=" + offset + " seconds, live audio device, loop=" + looped
                + ", minimize/restore, pointer events and clean close. Physical listening not assessed.";
        observations.add(result); System.out.println(result);
        Files.write(output.resolve("result.txt"), observations);
    }
}
