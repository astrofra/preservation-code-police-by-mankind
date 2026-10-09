import java.awt.EventQueue;
import java.awt.event.WindowEvent;
import java.nio.file.Path;
import java.util.ArrayList;

/** Reference viewer: presentation options never resize or patch the archived engine. */
public final class OriginalHost {
    public record Session(ReferenceSupport reference, DemoWindow display) { }

    public static Session open(Path root, DisplayOptions options) throws Exception {
        SceneAssets assets = new SceneAssets(root.resolve("original/mkd_codepolice"));
        ReferenceSupport reference = new ReferenceSupport(root, assets, new ArrayList<>(), !options.mute);
        try {
            ReferenceSupport.call(reference.demo, "init", new Class<?>[0]);
            DemoWindow display = new DemoWindow("Code Police — Mankind (original Java)", options,
                reference.component::update, () -> {
                    try {
                        ReferenceSupport.call(reference.demo, "stop", new Class<?>[0]);
                        Thread worker = (Thread) ReferenceSupport.field(reference.demo, "mainthread");
                        if (worker != null) {
                            worker.join(3000L);
                            if (worker.isAlive()) throw new IllegalStateException("Original render thread did not stop");
                        }
                        reference.close();
                    } catch (Exception exception) { throw new RuntimeException(exception); }
                });
            display.show(options.fullscreen);
            ReferenceSupport.call(reference.demo, "start", new Class<?>[0]);
            System.out.println("Original 520x300 buffer; display=" + display.surface.getWidth() + "x" + display.surface.getHeight()
                + "; fullscreen=" + display.isFullscreen() + "; nearest neighbour; monitor resolution unchanged.");
            return new Session(reference, display);
        } catch (Exception exception) { reference.close(); throw exception; }
    }

    public static void main(String[] args) throws Exception {
        if (args.length == 1 && (args[0].equals("--help") || args[0].equals("-h"))) {
            System.out.println("Usage: OriginalHost PACKAGE_OR_REPOSITORY_ROOT " + DisplayOptions.help()); return;
        }
        if (args.length < 1) throw new IllegalArgumentException("Usage: OriginalHost PACKAGE_OR_REPOSITORY_ROOT " + DisplayOptions.help());
        DisplayOptions options = DisplayOptions.parse(args, 1, 1);
        if (options.help) { System.out.println("Usage: OriginalHost PACKAGE_OR_REPOSITORY_ROOT " + DisplayOptions.help()); return; }
        Path root = Path.of(args[0]);
        EventQueue.invokeAndWait(() -> {
            try {
                Session session = open(root, options);
                if (options.smokeTest) {
                    javax.swing.Timer timer = new javax.swing.Timer(8000, event -> {
                        try {
                            ReferenceSupport reference = session.reference();
                            Thread worker = (Thread) ReferenceSupport.field(reference.demo, "mainthread");
                            if (!worker.isAlive() || reference.component.getWidth() != 520 || reference.component.getHeight() != 300)
                                throw new AssertionError("Original buffer is not running at 520 x 300");
                            session.display().frame.dispatchEvent(new WindowEvent(session.display().frame, WindowEvent.WINDOW_CLOSING));
                            if (worker.isAlive() || session.display().frame.isDisplayable()) throw new AssertionError("Original applet did not close");
                            System.out.println("PASS: original bytecode, fixed 520x300 buffer, scaled display and clean shutdown");
                        } catch (Throwable failure) { failure.printStackTrace(); System.exit(1); }
                    });
                    timer.setRepeats(false); timer.start();
                }
            } catch (Exception exception) { throw new RuntimeException(exception); }
        });
    }
}
