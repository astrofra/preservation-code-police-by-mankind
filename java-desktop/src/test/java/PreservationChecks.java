import java.nio.file.Path;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public final class PreservationChecks {
    public static void main(String[] args) throws Exception {
        SceneAssets assets = new SceneAssets(Path.of(args[0]));
        List<DemoAudio.Event> events = new ArrayList<>();
        int images = 0, sounds = 0;
        try (var paths = Files.list(assets.directory.resolve("Images"))) {
            for (Path path : paths.toList()) {
                var image = SceneAssets.image(path.toUri().toURL());
                if (image.getWidth(null) <= 0 || image.getHeight(null) <= 0) throw new AssertionError(path);
                images++;
            }
        }
        try (var paths = Files.list(assets.directory.resolve("Sounds"))) {
            for (Path path : paths.toList()) {
                try (DemoAudio audio = new DemoAudio(path.toUri().toURL(), true, events)) {
                    if (audio.pcm.length == 0 || audio.format.getSampleRate() != 8000) throw new AssertionError(path);
                    audio.play(); audio.loop(); audio.stop();
                    sounds++;
                }
            }
        }
        kraycasting demo = new kraycasting();
        demo.configure(assets, true, events);
        String[] parts = demo.SplitSTag(demo.SplitSTag(assets.script, "KSCRIPT")[0], "KPART");
        int duration = 0;
        for (String part : parts) duration += Integer.parseInt(demo.FindInto(demo.SplitSTag(part, "D")[0], 0, ",").trim());
        if (parts.length != 12 || duration != 22200 || images != 53 || sounds != 10)
            throw new AssertionError("Asset or script inventory changed");
        if (demo.SplitSTag("<K PART>disabled</KPART>", "KPART") != null) throw new AssertionError("Disabled tag activated");
        System.out.println("PASS: 53 AWT images, 10 Java Sound decodes, original tag semantics, 12 parts / 222 seconds");
    }
}
