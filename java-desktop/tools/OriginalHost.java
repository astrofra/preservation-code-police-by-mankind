import java.awt.EventQueue;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.nio.file.Path;
import java.util.ArrayList;
import javax.swing.JFrame;

/** Separate reference viewer; no archived .class file is patched or recompiled. */
public final class OriginalHost {
    public static void main(String[] args) throws Exception {
        Path root = Path.of(args[0]);
        SceneAssets assets = new SceneAssets(root.resolve("original/mkd_codepolice"));
        EventQueue.invokeAndWait(() -> {
            try {
                ReferenceSupport reference = new ReferenceSupport(root, assets, new ArrayList<>(), true);
                JFrame frame = new JFrame("Code Police — original bytecode reference");
                reference.component.setPreferredSize(new java.awt.Dimension(520, 300));
                frame.add(reference.component);
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
            } catch (Exception exception) { throw new RuntimeException(exception); }
        });
    }
}
