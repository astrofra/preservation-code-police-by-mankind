import java.awt.EventQueue;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.nio.file.Path;
import java.util.ArrayList;
import javax.swing.JFrame;

/** Separate reference viewer; no archived .class file is patched or recompiled. */
public final class OriginalHost {
    public static void main(String[] args) throws Exception {
        if (args.length < 1 || args.length > 2 || (args.length == 2 && !args[1].equals("--smoke-test"))) {
            throw new IllegalArgumentException("Usage: OriginalHost PACKAGE_OR_REPOSITORY_ROOT [--smoke-test]");
        }
        boolean smokeTest = args.length == 2;
        Path root = Path.of(args[0]);
        SceneAssets assets = new SceneAssets(root.resolve("original/mkd_codepolice"));
        EventQueue.invokeAndWait(() -> {
            try {
                ReferenceSupport reference = new ReferenceSupport(root, assets, new ArrayList<>(), true);
                JFrame frame = new JFrame("Code Police — original bytecode reference");
                reference.component.setPreferredSize(new java.awt.Dimension(520, 300));
                frame.add(reference.component);
                frame.setResizable(false);
                frame.pack();
                frame.setLocationRelativeTo(null);
                frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
                frame.addWindowListener(new WindowAdapter() {
                    @Override public void windowClosing(WindowEvent event) {
                        try { ReferenceSupport.call(reference.demo, "stop", new Class<?>[0]); reference.close(); }
                        catch (Exception exception) { throw new RuntimeException(exception); }
                        finally { frame.dispose(); }
                    }
                });
                ReferenceSupport.call(reference.demo, "init", new Class<?>[0]);
                frame.setVisible(true);
                ReferenceSupport.call(reference.demo, "start", new Class<?>[0]);
                if (smokeTest) {
                    javax.swing.Timer timer = new javax.swing.Timer(8000, event -> {
                        try {
                            Thread worker = (Thread) ReferenceSupport.field(reference.demo, "mainthread");
                            if (!worker.isAlive() || reference.component.getWidth() != 520 || reference.component.getHeight() != 300)
                                throw new AssertionError("Original applet is not running at 520 x 300");
                            frame.dispatchEvent(new WindowEvent(frame, WindowEvent.WINDOW_CLOSING));
                            worker.join(3000L);
                            if (worker.isAlive() || frame.isDisplayable()) throw new AssertionError("Original applet did not close");
                            System.out.println("PASS: original bytecode, 520x300 without cinema frame, running worker and clean shutdown");
                        } catch (Throwable failure) {
                            failure.printStackTrace();
                            System.exit(1);
                        }
                    });
                    timer.setRepeats(false);
                    timer.start();
                }
            } catch (Exception exception) { throw new RuntimeException(exception); }
        });
    }
}
