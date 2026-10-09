import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.ImageIcon;

public final class CodePoliceDesktop {
    public record Session(JFrame window, kraycasting demo) { }

    public static Session open(SceneAssets assets, boolean silent) {
        if (!EventQueue.isDispatchThread()) throw new IllegalStateException("Use the AWT event thread");
        kraycasting demo = new kraycasting();
        demo.configure(assets, silent, null);
        JFrame frame = new JFrame("Code Police — Mankind (2001)");
        frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        Image top = assets.image("Images/html_bord_haut.jpg");
        Image left = assets.image("Images/html_bord_gauche.jpg");
        Image right = assets.image("Images/html_bord_droit.jpg");
        Image bottom = assets.image("Images/html_bord_bas.jpg");
        JPanel presentation = new JPanel(null) {
            @Override protected void paintComponent(Graphics graphics) {
                super.paintComponent(graphics);
                graphics.drawImage(top, 0, 0, this);
                graphics.drawImage(left, 0, 13, this);
                graphics.drawImage(right, 585, 13, this);
                graphics.drawImage(bottom, 0, 313, this);
            }
        };
        presentation.setBackground(Color.BLACK);
        presentation.setPreferredSize(new Dimension(640, 480));
        demo.setBounds(65, 13, 520, 300);
        presentation.add(demo);
        JButton close = new JButton(new ImageIcon(assets.image("Images/html_close.gif")));
        close.setBounds(0, 0, 15, 15);
        close.setBorderPainted(false);
        close.setContentAreaFilled(false);
        close.setToolTipText("Quitter");
        close.addActionListener(event -> frame.dispatchEvent(new WindowEvent(frame, WindowEvent.WINDOW_CLOSING)));
        presentation.add(close);
        frame.setContentPane(presentation);
        frame.setResizable(false);
        frame.pack();
        frame.setLocationRelativeTo(null);
        frame.addWindowListener(new WindowAdapter() {
            @Override public void windowClosing(WindowEvent event) {
                try { demo.stop(); }
                finally { demo.close(); frame.dispose(); }
            }
        });
        demo.addKeyListener(new KeyAdapter() {
            @Override public void keyPressed(KeyEvent event) {
                if (event.getKeyCode() == KeyEvent.VK_ESCAPE)
                    frame.dispatchEvent(new WindowEvent(frame, WindowEvent.WINDOW_CLOSING));
            }
        });
        try {
            demo.init();
            frame.setVisible(true);
            demo.requestFocusInWindow();
            demo.start();
            return new Session(frame, demo);
        } catch (RuntimeException exception) {
            demo.close(); frame.dispose(); throw exception;
        }
    }

    public static Session open(SceneAssets assets, DisplayOptions options) {
        if (!EventQueue.isDispatchThread()) throw new IllegalStateException("Use the AWT event thread");
        kraycasting demo = new kraycasting();
        demo.configure(assets, options.mute, null);
        demo.init();
        DemoWindow display = new DemoWindow("Code Police — Mankind (Java)", options, demo::update,
            () -> { try { demo.stop(); } finally { demo.close(); } });
        display.show(options.fullscreen);
        demo.start();
        return new Session(display.frame, demo);
    }

    public static void main(String[] args) throws Exception {
        DisplayOptions options = DisplayOptions.parse(args, 0, 1);
        if (options.help) { System.out.println("Usage: CodePoliceDesktop " + DisplayOptions.help()); return; }
        SceneAssets assets = SceneAssets.installed();
        boolean mute = options.mute;
        boolean probe = options.smokeTest;
        Thread.setDefaultUncaughtExceptionHandler((thread, error) -> {
            error.printStackTrace();
            EventQueue.invokeLater(() -> JOptionPane.showMessageDialog(null,
                    error.toString(), "Code Police", JOptionPane.ERROR_MESSAGE));
        });
        EventQueue.invokeLater(() -> {
            Session session = options.explicitDisplay ? open(assets, options) : open(assets, mute);
            if (probe) {
                javax.swing.Timer timer = new javax.swing.Timer(8000, event -> {
                    boolean running = session.demo().isRunning();
                    boolean sound = session.demo().audioClips().stream().anyMatch(DemoAudio::isRunning);
                    session.window().dispatchEvent(new WindowEvent(session.window(), WindowEvent.WINDOW_CLOSING));
                    if (!running || (!mute && !sound) || session.demo().isRunning()) {
                        System.err.println("FAIL: packaged application smoke test");
                        System.exit(1);
                    }
                    System.out.println("PASS: packaged GUI, animation, audio and clean shutdown; assets=" + assets.directory);
                });
                timer.setRepeats(false);
                timer.start();
            }
        });
    }
}
