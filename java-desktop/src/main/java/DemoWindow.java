import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsDevice;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.function.Consumer;
import javax.swing.AbstractAction;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.KeyStroke;
import javax.swing.Timer;

/** Hosts a fixed-size engine offscreen and scales its published image only. */
public final class DemoWindow {
    public final JFrame frame;
    public final JPanel surface;
    private final Timer refresh;
    private Rectangle windowBounds;
    private Dimension correctedSize;
    private boolean fullscreen, switching, closed;
    private GraphicsDevice fullscreenDevice;

    public DemoWindow(String title, DisplayOptions options, Consumer<Graphics2D> paint, Runnable stop) {
        frame = new JFrame(title);
        surface = new JPanel() {
            @Override protected void paintComponent(Graphics graphics) {
                super.paintComponent(graphics);
                Rectangle area = viewport();
                if (area.isEmpty()) return;
                Graphics2D scaled = (Graphics2D) graphics.create();
                try {
                    scaled.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
                    scaled.translate(area.x, area.y);
                    scaled.scale(area.width / 520.0, area.height / 300.0);
                    scaled.clipRect(0, 0, 520, 300);
                    paint.accept(scaled);
                } finally { scaled.dispose(); }
            }
        };
        surface.setBackground(Color.BLACK);
        correctedSize = DisplayOptions.windowSize(options.width, options.height);
        surface.setPreferredSize(correctedSize);
        frame.setContentPane(surface);
        frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        frame.pack();
        frame.setLocationRelativeTo(null);
        frame.addComponentListener(new ComponentAdapter() {
            @Override public void componentResized(ComponentEvent event) {
                if (fullscreen || switching || closed) return;
                Dimension size = surface.getSize();
                if (size.width < 1 || size.height < 1 || size.equals(correctedSize)) return;
                correctedSize = DisplayOptions.windowSize(size.width, size.height);
                if (!size.equals(correctedSize)) {
                    var insets = frame.getInsets();
                    frame.setSize(correctedSize.width + insets.left + insets.right,
                                  correctedSize.height + insets.top + insets.bottom);
                }
            }
        });
        bind("ESCAPE", "quit", () -> frame.dispatchEvent(new WindowEvent(frame, WindowEvent.WINDOW_CLOSING)));
        bind("F11", "fullscreen", () -> setFullscreen(!fullscreen));
        refresh = new Timer(20, event -> surface.repaint());
        frame.addWindowListener(new WindowAdapter() {
            @Override public void windowClosing(WindowEvent event) {
                if (closed) return;
                closed = true;
                refresh.stop();
                try { stop.run(); }
                finally {
                    if (fullscreenDevice != null) fullscreenDevice.setFullScreenWindow(null);
                    frame.dispose();
                }
            }
        });
    }

    private void bind(String key, String name, Runnable action) {
        surface.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke(key), name);
        surface.getActionMap().put(name, new AbstractAction() {
            @Override public void actionPerformed(java.awt.event.ActionEvent event) { action.run(); }
        });
    }

    public void show(boolean fullscreen) {
        if (fullscreen) setFullscreen(true);
        else frame.setVisible(true);
        refresh.start();
    }

    public Rectangle viewport() {
        // No bands in windowed mode; its dimensions have already been corrected.
        return fullscreen ? DisplayOptions.viewport(surface.getWidth(), surface.getHeight())
                          : new Rectangle(0, 0, surface.getWidth(), surface.getHeight());
    }
    public boolean isFullscreen() { return fullscreen; }

    public void setFullscreen(boolean enabled) {
        if (enabled == fullscreen || closed) return;
        switching = true;
        try {
            GraphicsDevice device = frame.getGraphicsConfiguration().getDevice();
            Rectangle screen = frame.getGraphicsConfiguration().getBounds();
            if (enabled) windowBounds = frame.getBounds();
            if (!enabled && fullscreenDevice != null) {
                fullscreenDevice.setFullScreenWindow(null);
                fullscreenDevice = null;
            }
            frame.dispose();
            frame.setUndecorated(enabled);
            fullscreen = enabled;
            // Never call setDisplayMode: fullscreen retains the current mode.
            frame.setBounds(enabled ? screen : windowBounds);
            if (enabled) {
                fullscreenDevice = device;
                device.setFullScreenWindow(frame);
            }
            frame.setVisible(true);
            surface.requestFocusInWindow();
        } finally { switching = false; }
    }
}
