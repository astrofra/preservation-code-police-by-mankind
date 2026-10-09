import java.awt.*;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.nio.file.*;
import java.util.Arrays;
import javax.imageio.ImageIO;

/** Checks presentation geometry, actual reference playback, F11 and screen mode. */
public final class DisplayWindowChecks {
    static void require(boolean condition, String message) { if (!condition) throw new AssertionError(message); }
    static void geometry() {
        int[][] cases = {{1280,720,1248,720,16,0},{800,600,800,462,0,69},
            {1920,1080,1872,1080,24,0},{1024,768,1024,591,0,88},
            {600,1000,600,346,0,327},{3440,1440,2496,1440,472,0},{1040,600,1040,600,0,0}};
        for (int[] c : cases) {
            require(DisplayOptions.windowSize(c[0],c[1]).equals(new Dimension(c[2],c[3])), "window geometry " + Arrays.toString(c));
            require(DisplayOptions.viewport(c[0],c[1]).equals(new Rectangle(c[4],c[5],c[2],c[3])), "fullscreen geometry");
        }
        for (String value : new String[]{"0x720","-1x720","1280","1280x","12x3x4","999999999999x720","1280x720bad"}) {
            try { DisplayOptions.parse(new String[]{"--resolution",value},0,1); throw new AssertionError("Accepted " + value); }
            catch (IllegalArgumentException expected) { }
        }
    }
    static BufferedImage paint(DemoWindow display) throws Exception {
        BufferedImage[] image = new BufferedImage[1];
        EventQueue.invokeAndWait(() -> {
            image[0] = new BufferedImage(display.surface.getWidth(), display.surface.getHeight(), BufferedImage.TYPE_INT_RGB);
            Graphics2D graphics = image[0].createGraphics(); display.surface.paint(graphics); graphics.dispose();
        });
        return image[0];
    }
    static void sameMode(DisplayMode before, DisplayMode after) {
        require(before.getWidth()==after.getWidth() && before.getHeight()==after.getHeight()
            && before.getBitDepth()==after.getBitDepth() && before.getRefreshRate()==after.getRefreshRate(), "Monitor resolution/mode changed");
    }
    public static void main(String[] args) throws Exception {
        geometry();
        Path root=Path.of(args[0]), out=root.resolve("java-desktop/build/display-checks"); Files.createDirectories(out);
        // An artificial high-contrast pattern makes interpolation errors observable.
        BufferedImage pattern=new BufferedImage(520,300,BufferedImage.TYPE_INT_RGB);
        for(int y=0;y<300;y++) for(int x=0;x<520;x++) pattern.setRGB(x,y,((x+y)&1)==0 ? 0xff00ff : 0x00ff00);
        DemoWindow[] window=new DemoWindow[1];
        EventQueue.invokeAndWait(() -> {
            window[0]=new DemoWindow("Display validation",new DisplayOptions(2),g -> g.drawImage(pattern,0,0,null),() -> {});
            window[0].show(false);
        });
        DemoWindow display=window[0];
        try {
            BufferedImage scaled=paint(display);
            for(int y=0;y<600;y++) for(int x=0;x<1040;x++)
                require(scaled.getRGB(x,y)==pattern.getRGB(x/2,y/2),"Nearest-neighbour pixel mismatch");
            Dimension before=display.surface.getSize();
            GraphicsDevice device=display.frame.getGraphicsConfiguration().getDevice();
            DisplayMode mode=device.getDisplayMode();
            EventQueue.invokeAndWait(() -> display.surface.getActionMap().get("fullscreen").actionPerformed(null));
            Thread.sleep(350);
            require(display.isFullscreen(),"F11 did not enter fullscreen"); sameMode(mode,device.getDisplayMode());
            BufferedImage fullscreen=paint(display); Rectangle area=display.viewport();
            require(area.equals(DisplayOptions.viewport(fullscreen.getWidth(),fullscreen.getHeight())),"Fullscreen fit");
            for(int y=0;y<fullscreen.getHeight();y++) for(int x=0;x<fullscreen.getWidth();x++) {
                int rgb=fullscreen.getRGB(x,y)&0xffffff;
                require(area.contains(x,y) ? rgb==0xff00ff || rgb==0x00ff00 : rgb==0,"Fullscreen filter or black bands");
            }
            ImageIO.write(fullscreen,"png",out.resolve("fullscreen-pattern.png").toFile());
            EventQueue.invokeAndWait(() -> display.surface.getActionMap().get("fullscreen").actionPerformed(null));
            Thread.sleep(250);
            require(!display.isFullscreen() && before.equals(display.surface.getSize()),"Window size not restored");
            sameMode(mode,device.getDisplayMode());
            EventQueue.invokeAndWait(() -> {
                var insets=display.frame.getInsets(); display.frame.setSize(800+insets.left+insets.right,600+insets.top+insets.bottom);
            });
            Thread.sleep(250);
            require(display.surface.getSize().equals(new Dimension(800,462)),"Manual resize aspect ratio");
        } finally { EventQueue.invokeAndWait(() -> display.frame.dispatchEvent(new WindowEvent(display.frame,WindowEvent.WINDOW_CLOSING))); }

        OriginalHost.Session[] session=new OriginalHost.Session[1];
        DisplayOptions options=DisplayOptions.parse(new String[]{"--resolution","800x600","--mute"},0,1);
        EventQueue.invokeAndWait(() -> {
            try {
                session[0]=OriginalHost.open(root,options);
                Object timer=ReferenceSupport.field(session[0].reference().demo,"kTime");
                var start=timer.getClass().getDeclaredField("StartTime"); start.setAccessible(true); start.setLong(timer,System.currentTimeMillis()-45000);
            } catch(Exception e) { throw new RuntimeException(e); }
        });
        OriginalHost.Session original=session[0];
        try {
            Thread.sleep(500);
            ReferenceSupport.call(original.reference().demo,"stop",new Class<?>[0]);
            ((Thread)ReferenceSupport.field(original.reference().demo,"mainthread")).join(3000);
            require(original.reference().component.getSize().equals(new Dimension(520,300)),"Original buffer resized");
            require(original.display().surface.getSize().equals(new Dimension(800,462)),"CLI resolution not applied");
            BufferedImage image=paint(original.display());
            require(image.getRGB(400,230)!=0xff000000,"Original demo is not visible through the scaling host");
            ImageIO.write(image,"png",out.resolve("original-scaled.png").toFile());
        } finally { EventQueue.invokeAndWait(() -> original.display().frame.dispatchEvent(new WindowEvent(original.display().frame,WindowEvent.WINDOW_CLOSING))); }
        System.out.println("PASS: CLI parsing, seven geometries, exact nearest-neighbour pixels, fullscreen black bands, unchanged monitor mode, F11 round trip, resize, original 520x300 engine and scaled playback.");
    }
}
