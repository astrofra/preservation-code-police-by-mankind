import java.awt.Dimension;
import java.awt.Rectangle;

/** Presentation geometry only: the artwork always renders at 520 x 300. */
public final class DisplayOptions {
    public static final int WIDTH = 520, HEIGHT = 300;
    public int width, height;
    public boolean fullscreen, mute, smokeTest, help, explicitDisplay;

    public DisplayOptions(int scale) { width = WIDTH * scale; height = HEIGHT * scale; }

    public static DisplayOptions parse(String[] args, int start, int defaultScale) {
        DisplayOptions options = new DisplayOptions(defaultScale);
        for (int i = start; i < args.length; i++) {
            switch (args[i]) {
                case "--resolution" -> {
                    if (++i == args.length || !args[i].matches("[0-9]+[xX][0-9]+"))
                        throw new IllegalArgumentException("--resolution requires WIDTHxHEIGHT");
                    String[] size = args[i].split("[xX]");
                    options.width = dimension(size[0]); options.height = dimension(size[1]);
                    options.explicitDisplay = true;
                }
                case "--scale" -> {
                    if (++i == args.length || !args[i].matches("[1-4]"))
                        throw new IllegalArgumentException("--scale requires 1, 2, 3 or 4");
                    int scale = Integer.parseInt(args[i]);
                    options.width = WIDTH * scale; options.height = HEIGHT * scale;
                    options.explicitDisplay = true;
                }
                case "--fullscreen" -> { options.fullscreen = true; options.explicitDisplay = true; }
                case "--mute" -> options.mute = true;
                case "--smoke-test" -> options.smokeTest = true;
                case "--help", "-h" -> options.help = true;
                default -> throw new IllegalArgumentException("Unknown option: " + args[i]);
            }
        }
        return options;
    }

    private static int dimension(String text) {
        try {
            int value = Integer.parseInt(text);
            if (value >= 1 && value <= 16384) return value;
        } catch (NumberFormatException ignored) { }
        throw new IllegalArgumentException("Display dimensions must be between 1 and 16384");
    }

    public static Dimension windowSize(int width, int height) {
        Rectangle fit = viewport(width, height);
        return new Dimension(fit.width, fit.height);
    }

    public static Rectangle viewport(int width, int height) {
        if (width <= 0 || height <= 0) return new Rectangle();
        int w = width, h = height;
        if (width * 300L > height * 520L) w = Math.max(1, (int)((height * 520L + 150) / 300));
        else h = Math.max(1, (int)((width * 300L + 260) / 520));
        return new Rectangle((width - w) / 2, (height - h) / 2, w, h);
    }

    public static String help() {
        return "[--resolution WIDTHxHEIGHT] [--scale 1..4] [--fullscreen] [--mute] [--smoke-test]\n"
            + "Window: fit 520:300 within the requested dimensions (rounded to pixels).\n"
            + "Fullscreen: fill the current monitor as far as the aspect ratio permits, with black bars.\n"
            + "The monitor resolution never changes. Nearest-neighbour scaling.\n"
            + "Escape: quit. F11: toggle fullscreen and restore the window size.\n";
    }
}
