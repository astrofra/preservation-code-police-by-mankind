import java.awt.Image;
import java.awt.image.BufferedImage;
import java.awt.image.PixelGrabber;
import java.io.*;
import java.lang.reflect.Field;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;
import javax.imageio.ImageIO;
import javax.sound.sampled.*;

/** Preparation and reference tools only; no Java dependency in the native app. */
public class ExportNative {
    static String sha(byte[] b) throws Exception { return HexFormat.of().formatHex(MessageDigest.getInstance("SHA-256").digest(b)); }
    static long hash(int[] pixels) { long h=0xcbf29ce484222325L; for(int p:pixels) { h^=Integer.toUnsignedLong(p|0xff000000); h*=0x100000001b3L; } return h; }
    static double[] state(Object demo) throws Exception {
        List<Double> values=new ArrayList<>();
        for(String mapName:List.of("CamerHash","partcHash")) {
            Hashtable<?,?> map=(Hashtable<?,?>)ReferenceSupport.field(demo,mapName);
            for(String key:new TreeSet<>(map.keySet().stream().map(Object::toString).toList())) {
                Object obj=map.get(key);
                Field[] fields=obj.getClass().getDeclaredFields();
                Arrays.sort(fields,Comparator.comparing(Field::getName));
                for(Field f:fields) {
                    f.setAccessible(true);
                    if(f.getType()==double.class) values.add(f.getDouble(obj));
                    if(f.getType()==int.class) values.add((double)f.getInt(obj));
                    if(f.getType()==double[].class) for(double d:(double[])f.get(obj)) values.add(d);
                }
            }
        }
        return values.stream().mapToDouble(Double::doubleValue).toArray();
    }
    static void assets(Path root,Path out) throws Exception {
        Path release=root.resolve("original/mkd_codepolice");
        SceneAssets assets=new SceneAssets(release);
        Files.createDirectories(out.resolve("Images")); Files.createDirectories(out.resolve("Sounds"));
        Files.writeString(out.resolve("scene.txt"),assets.script);
        StringBuilder manifest=new StringBuilder("source\tsource_sha256\tconverted\tconverted_sha256\tARGB_sha256\n");
        try(var files=Files.list(release.resolve("Images"))) {
            for(Path p:files.sorted().toList()) {
                if(!p.toString().matches("(?i).*\\.(gif|jpg)$")) continue;
                Image image=SceneAssets.image(p.toUri().toURL());
                int w=image.getWidth(null),h=image.getHeight(null); int[] pixels=new int[w*h];
                new PixelGrabber(image,0,0,w,h,pixels,0,w).grabPixels();
                BufferedImage png=new BufferedImage(w,h,BufferedImage.TYPE_INT_ARGB); png.setRGB(0,0,w,h,pixels,0,w);
                Path dest=out.resolve("Images/"+p.getFileName()+".png");
                ImageIO.write(png,"png",dest.toFile());
                int[] roundtrip=ImageIO.read(dest.toFile()).getRGB(0,0,w,h,null,0,w);
                if(!Arrays.equals(pixels,roundtrip)) throw new AssertionError("PNG pixels: "+p);
                ByteArrayOutputStream buf=new ByteArrayOutputStream(); DataOutputStream data=new DataOutputStream(buf);
                for(int pixel:pixels)data.writeInt(pixel);
                manifest.append("Images/").append(p.getFileName()).append('\t').append(sha(Files.readAllBytes(p))).append('\t').append(out.relativize(dest)).append('\t').append(sha(Files.readAllBytes(dest))).append('\t').append(sha(buf.toByteArray())).append('\n');
            }
        }
        try(var files=Files.list(release.resolve("Sounds"))) {
            for(Path p:files.sorted().toList()) if(p.toString().toLowerCase().endsWith(".au")) {
                Files.copy(p,out.resolve("Sounds/"+p.getFileName()),StandardCopyOption.REPLACE_EXISTING);
                try(AudioInputStream au=AudioSystem.getAudioInputStream(p.toFile());
                    AudioInputStream pcm=AudioSystem.getAudioInputStream(new AudioFormat(8000,16,1,true,true),au)) {
                    Path ref=root.resolve("native-sdl2/build/pcm/"+p.getFileName()+".s16be");
                    Files.createDirectories(ref.getParent()); Files.write(ref,pcm.readAllBytes());
                }
            }
        }
        Files.writeString(out.resolve("image-manifest.tsv"),manifest);
        System.out.println("External assets prepared; all 53 PNG pixel arrays round-trip exactly.");
    }
    public static void main(String[] args) throws Exception {
        Path root=Path.of(args[1]); Path out=Path.of(args[2]);
        if(args[0].equals("assets")) { assets(root,out); return; }
        SceneAssets assets=new SceneAssets(root.resolve("original/mkd_codepolice"));
        List<DemoAudio.Event> events=new ArrayList<>();
        PrintStream log=System.out;
        try(ReferenceSupport reference=new ReferenceSupport(root,assets,events,false);
            DataOutputStream data=new DataOutputStream(new BufferedOutputStream(Files.newOutputStream(out)))) {
            System.setOut(new PrintStream(OutputStream.nullOutputStream()));
            ReferenceSupport.seed(888); ReferenceSupport.call(reference.demo,"init",new Class<?>[0]);
            System.setOut(log);
            data.writeInt(0x43505231); data.writeInt(11101);
            int eventIndex=0;
            int[] captures={0,500,1500,2500,3550,4050,4500,6500,8000,9500,11000,13000,17000,20000,21900,22200};
            for(int tick=0;tick<=22200;tick+=2) {
                ReferenceSupport.seed(100000L+tick);
                int[] pixels=CompareOriginal.render(reference.demo,tick);
                data.writeInt(tick); data.writeLong(hash(pixels));
                double[] state=state(reference.demo); data.writeInt(state.length); for(double d:state)data.writeDouble(d);
                data.writeInt(events.size()-eventIndex);
                while(eventIndex<events.size()) { var e=events.get(eventIndex++); data.writeUTF(e.resource()); data.writeUTF(e.command()); }
                boolean capture=Arrays.binarySearch(captures,tick)>=0;
                data.writeBoolean(capture); if(capture) {
                    for(int p:pixels)data.writeInt(p|0xff000000);
                    CompareOriginal.save(out.resolveSibling("original-"+tick+".png"),pixels);
                }
                if(tick%2000==0)log.println("Reference tick "+tick);
            }
        } finally { System.setOut(log); }
        System.out.println("Original bytecode reference: 11101 frames, "+events.size()+" audio commands.");
    }
}
