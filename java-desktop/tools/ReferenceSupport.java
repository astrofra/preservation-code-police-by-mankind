import java.awt.Component;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.file.Path;
import java.util.List;

/** Test-only JDK <=25 adapter. It executes the archived classes without rewriting them. */
public final class ReferenceSupport implements AutoCloseable {
    final URLClassLoader loader;
    final Object demo;
    final Component component;

    ReferenceSupport(Path root, SceneAssets assets, List<DemoAudio.Event> events, boolean liveAudio) throws Exception {
        loader = new URLClassLoader(new URL[] {root.resolve("original/mkd_codepolice").toUri().toURL()},
                ClassLoader.getPlatformClassLoader());
        demo = loader.loadClass("kraycasting").getConstructor().newInstance();
        component = (Component) demo;
        component.setSize(520, 300);
        Class<?> applet = Class.forName("java.applet.Applet");
        Class<?> context = Class.forName("java.applet.AppletContext");
        Class<?> stub = Class.forName("java.applet.AppletStub");
        Class<?> audio = Class.forName("java.applet.AudioClip");
        Object contextProxy = Proxy.newProxyInstance(context.getClassLoader(), new Class<?>[]{context}, (p, m, a) -> {
            if (m.getName().equals("getImage")) return SceneAssets.image((URL) a[0]);
            if (m.getName().equals("getAudioClip")) {
                URL url = (URL) a[0];
                if (liveAudio) return applet.getMethod("newAudioClip", URL.class).invoke(null, url);
                String name = url.getPath().substring(url.getPath().lastIndexOf('/') + 1);
                return Proxy.newProxyInstance(audio.getClassLoader(), new Class<?>[]{audio}, (p2, m2, a2) -> {
                    events.add(new DemoAudio.Event(name, m2.getName()));
                    return null;
                });
            }
            return null;
        });
        Object stubProxy = Proxy.newProxyInstance(stub.getClassLoader(), new Class<?>[]{stub}, (p, m, a) -> switch (m.getName()) {
            case "getCodeBase", "getDocumentBase" -> assets.baseURL();
            case "isActive" -> true;
            case "getParameter" -> "SCRIPT".equals(a[0]) ? assets.script : null;
            case "getAppletContext" -> contextProxy;
            default -> null;
        });
        applet.getMethod("setStub", stub).invoke(demo, stubProxy);
    }

    static Object field(Object object, String name) throws Exception {
        Field field = object.getClass().getDeclaredField(name);
        field.setAccessible(true);
        return field.get(object);
    }
    static Object call(Object object, String name, Class<?>[] types, Object... args) throws Exception {
        Method method = object.getClass().getDeclaredMethod(name, types);
        method.setAccessible(true);
        return method.invoke(object, args);
    }
    static void seed(long value) throws Exception {
        Field field = Class.forName("java.lang.Math$RandomNumberGeneratorHolder").getDeclaredField("randomNumberGenerator");
        field.setAccessible(true);
        ((java.util.Random) field.get(null)).setSeed(value);
    }
    @Override public void close() throws Exception { loader.close(); }
}
