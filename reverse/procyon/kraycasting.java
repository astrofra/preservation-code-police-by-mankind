import java.applet.AudioClip;
import java.awt.image.ImageProducer;
import java.awt.image.MemoryImageSource;
import java.awt.image.PixelGrabber;
import java.awt.image.ImageObserver;
import java.awt.Graphics;
import java.util.Hashtable;
import java.awt.Image;
import java.applet.Applet;

// 
// Decompiled by Procyon v0.6.0
// 

public class kraycasting extends Applet implements Runnable
{
    private kScript THEkScript;
    private kRenderContext RenderContext;
    private raycasting rcs;
    private Image applet_image;
    private Thread mainthread;
    private boolean boolend;
    private kTimer kTime;
    private byte[] ct;
    private Hashtable partcHash;
    private Hashtable ImageHash;
    private Hashtable RectaHash;
    private Hashtable SoundHash;
    private Hashtable TableHash;
    private Hashtable CamerHash;
    
    public kraycasting() {
        this.mainthread = null;
        this.boolend = false;
    }
    
    int FindHowMany(final String s, final String str) {
        int n = 1;
        for (int length = s.length(), index = 0, fromIndex = 0; index != -1 && fromIndex < length; fromIndex = index + 1) {
            index = s.indexOf(str, fromIndex);
            if (index != -1) {
                ++n;
            }
        }
        return n;
    }
    
    String FindInto(final String s, final int n, final String s2) {
        int n2 = 2;
        int n3 = 1;
        final int length = s.length();
        for (int index = 0, fromIndex = 0; index != -1 && fromIndex < length; fromIndex = index + 1) {
            index = s.indexOf(s2, fromIndex);
            if (index != -1) {
                ++n2;
            }
        }
        final int[] array = new int[n2];
        array[0] = -1;
        array[n2 - 1] = length;
        for (int fromIndex2 = 0, index2 = 0; index2 != -1 && fromIndex2 < length; fromIndex2 = index2 + 1) {
            index2 = s.indexOf(s2, fromIndex2);
            if (index2 != -1) {
                array[n3] = index2;
                ++n3;
            }
        }
        return s.substring(array[n] + 1, array[n + 1]);
    }
    
    String[] SplitSTag(final String s, final String s2) {
        final String string = "<" + s2 + ">";
        final String string2 = "</" + s2 + ">";
        final int length = s.length();
        int n = 0;
        for (int fromIndex = 0, index = 0; index != -1 && fromIndex < length; fromIndex = index + string.length()) {
            index = s.indexOf(string, fromIndex);
            if (index != -1) {
                ++n;
            }
        }
        if (n == 0) {
            return null;
        }
        final String[] array = new String[n];
        for (int n2 = 0, fromIndex2 = 0, index2 = 0; index2 != -1 && fromIndex2 < length; fromIndex2 = index2 + string.length(), array[n2] = s.substring(fromIndex2, s.indexOf(string2, fromIndex2)), ++n2) {
            index2 = s.indexOf(string, fromIndex2);
            if (index2 != -1) {}
        }
        return array;
    }
    
    static /* synthetic */ void access$10(final kraycasting kraycasting, final Hashtable camerHash) {
        kraycasting.CamerHash = camerHash;
    }
    
    static /* synthetic */ void access$12(final kraycasting kraycasting, final Hashtable rectaHash) {
        kraycasting.RectaHash = rectaHash;
    }
    
    static /* synthetic */ void access$14(final kraycasting kraycasting, final Hashtable soundHash) {
        kraycasting.SoundHash = soundHash;
    }
    
    static /* synthetic */ void access$17(final kraycasting kraycasting, final boolean boolend) {
        kraycasting.boolend = boolend;
    }
    
    static /* synthetic */ void access$4(final kraycasting kraycasting, final Hashtable tableHash) {
        kraycasting.TableHash = tableHash;
    }
    
    static /* synthetic */ void access$6(final kraycasting kraycasting, final Hashtable imageHash) {
        kraycasting.ImageHash = imageHash;
    }
    
    static /* synthetic */ void access$8(final kraycasting kraycasting, final Hashtable partcHash) {
        kraycasting.partcHash = partcHash;
    }
    
    public void init() {
        this.RenderContext = new kRenderContext(this);
        this.applet_image = this.RenderContext.createImage();
        this.THEkScript = new kScript(this.getParameter("SCRIPT"));
        this.rcs = new raycasting(this.RenderContext);
        this.ct = new byte[8192];
        for (int i = 0; i < 64; ++i) {
            for (int j = 0; j < 64; ++j) {
                this.ct[i << 6 | j] = new Integer(j * (i + 1) >> 6).byteValue();
            }
        }
        for (int k = 64; k < 128; ++k) {
            for (int l = 0; l < 64; ++l) {
                this.ct[k << 6 | l] = new Integer(l + ((63 - l) * (k - 64) >> 6)).byteValue();
            }
        }
        this.kTime = new kTimer();
    }
    
    public void run() {
        while (!this.boolend) {
            final int time = this.kTime.getTime();
            this.THEkScript.part(time).rendah(time);
            this.applet_image = this.RenderContext.createImage();
            this.repaint();
            try {
                Thread.sleep(20L);
            }
            catch (final InterruptedException ex) {}
        }
    }
    
    public void start() {
        (this.mainthread = new Thread(this)).start();
    }
    
    public void stop() {
        kSound.StopAll();
        this.boolend = true;
    }
    
    public void update(final Graphics graphics) {
        graphics.drawImage(this.applet_image, 0, 0, this);
    }
    
    public class kTexture
    {
        int Modulo;
        int Height;
        int[] pixels;
        
        public kTexture(final Image img) {
            try {
                while ((this.Modulo = img.getWidth(null)) < 0) {}
                this.Height = img.getHeight(null);
                this.pixels = new int[this.Modulo * this.Height];
                new PixelGrabber(img, 0, 0, this.Modulo, this.Height, this.pixels, 0, this.Modulo).grabPixels();
            }
            catch (final InterruptedException ex) {}
        }
    }
    
    private class kRenderContext
    {
        int ClipX1;
        int ClipY1;
        int ClipX2;
        int ClipY2;
        int Modulo;
        int Height;
        int[] Chunk2;
        int[] CurrentChunk;
        Applet app;
        private MemoryImageSource RC_imageMIS1;
        private MemoryImageSource RC_imageMIS2;
        private kRenderContext RC_RootContext;
        private kRenderContext RC_FatherContext;
        private int MaxX;
        private int MaxY;
        private int MaxPreX;
        private int MaxPreY;
        private kPixel[][] Pretable;
        private int CurPreX;
        private int CurPreY;
        private int CurX;
        private int CurY;
        private double PosX;
        private double PosY;
        private double PosZ;
        private double floor;
        private double ceiling;
        private boolean drawable;
        
        public kRenderContext(final Applet app) {
            int maxPreX = 0;
            this.app = app;
            this.ClipX1 = 0;
            this.ClipY1 = 0;
            this.ClipX2 = app.getSize().width;
            this.ClipY2 = app.getSize().height;
            final int n = this.ClipX2 - this.ClipX1;
            final int n2 = this.ClipY2 - this.ClipY1;
            this.drawable = true;
            final double n3 = 1.0 / n;
            final double n4 = 1.0 / n2;
            this.Modulo = this.ClipX2;
            this.Height = this.ClipY2;
            this.Chunk2 = new int[this.Modulo * this.Height];
            this.CurrentChunk = new int[this.Modulo * this.Height];
            this.RC_imageMIS1 = new MemoryImageSource(this.Modulo, this.Height, this.CurrentChunk, 0, this.Modulo);
            this.RC_imageMIS2 = new MemoryImageSource(this.Modulo, this.Height, this.Chunk2, 0, this.Modulo);
            this.RC_RootContext = this;
            this.MaxX = this.Modulo;
            this.MaxY = this.Height;
            this.MaxPreX = (this.MaxX >> 4) + 2;
            this.MaxPreY = (this.MaxY >> 4) + 2;
            this.Pretable = new kPixel[this.MaxPreX][this.MaxPreY];
            final int n5 = n >> 1;
            final int n6 = n2 >> 1;
            int n7 = 16;
            int maxPreY = 0;
            for (int i = 0; i < n2 + n7; i += n7) {
                if (i + 16 > n2) {
                    n7 = n2 - i;
                }
                int n8 = 16;
                maxPreX = 0;
                for (int j = 0; j < n + n8; j += n8) {
                    if (j + 16 > n) {
                        n8 = n - j;
                    }
                    this.Pretable[maxPreX][maxPreY] = new kPixel();
                    this.Pretable[maxPreX][maxPreY].XI = (j - n5) * n3;
                    this.Pretable[maxPreX][maxPreY].YI = (i - n6) * n4;
                    ++maxPreX;
                }
                ++maxPreY;
            }
            this.MaxPreX = maxPreX;
            this.MaxPreY = maxPreY;
        }
        
        public kRenderContext(final kRenderContext kRenderContext) {
            this.ClipX1 = kRenderContext.ClipX1;
            this.ClipY1 = kRenderContext.ClipY1;
            this.ClipX2 = kRenderContext.ClipX2;
            this.ClipY2 = kRenderContext.ClipY2;
            this.Modulo = kRenderContext.Modulo;
            this.Height = kRenderContext.Height;
            this.app = kRenderContext.app;
            this.RC_RootContext = kRenderContext;
            this.RC_FatherContext = kRenderContext;
            this.drawable = kRenderContext.drawable;
        }
        
        public kRenderContext(final kRenderContext kRenderContext, final double n, final double n2, final double n3, final double n4) {
            this.RC_RootContext = kRenderContext.RC_RootContext;
            this.Pretable = new kPixel[this.RC_RootContext.MaxPreX][this.RC_RootContext.MaxPreY];
            this.SetRect(kRenderContext, n, n2, n3, n4);
        }
        
        public void SetRect(final kRenderContext kRenderContext, double n, double n2, double n3, double n4) {
            int maxPreX = 0;
            final double n5 = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final double n6 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            this.drawable = true;
            if (n < 0.0) {
                n = 0.0;
            }
            if (n2 < 0.0) {
                n2 = 0.0;
            }
            if (n3 > 1.0) {
                n3 = 1.0;
            }
            if (n4 > 1.0) {
                n4 = 1.0;
            }
            this.ClipX1 = kRenderContext.ClipX1 + (int)(n5 * n);
            this.ClipY1 = kRenderContext.ClipY1 + (int)(n6 * n2);
            this.ClipX2 = kRenderContext.ClipX1 + (int)(n5 * n3);
            this.ClipY2 = kRenderContext.ClipY1 + (int)(n6 * n4);
            this.Modulo = kRenderContext.Modulo;
            this.Height = kRenderContext.Height;
            this.app = kRenderContext.app;
            final int maxX = this.ClipX2 - this.ClipX1;
            final int maxY = this.ClipY2 - this.ClipY1;
            final double n7 = 1.0 / maxX;
            final double n8 = 1.0 / maxY;
            if (maxX <= 0 || maxY <= 0 || this.ClipX1 < kRenderContext.ClipX1 || this.ClipY1 < kRenderContext.ClipY1 || this.ClipX2 > kRenderContext.ClipX2 || this.ClipY2 > kRenderContext.ClipY2) {
                this.drawable = false;
                return;
            }
            this.MaxX = maxX;
            this.MaxY = maxY;
            this.MaxPreX = (this.MaxX >> 4) + 2;
            this.MaxPreY = (this.MaxY >> 4) + 2;
            final int n9 = maxX >> 1;
            final int n10 = maxY >> 1;
            int n11 = 16;
            int maxPreY = 0;
            for (int i = 0; i < maxY + n11; i += n11) {
                if (i + 16 > maxY) {
                    n11 = maxY - i;
                }
                int n12 = 16;
                maxPreX = 0;
                for (int j = 0; j < maxX + n12; j += n12) {
                    if (j + 16 > maxX) {
                        n12 = maxX - j;
                    }
                    this.Pretable[maxPreX][maxPreY] = new kPixel();
                    this.Pretable[maxPreX][maxPreY].XI = (j - n9) * n7;
                    this.Pretable[maxPreX][maxPreY].YI = (i - n10) * n8;
                    ++maxPreX;
                }
                ++maxPreY;
            }
            this.MaxPreX = maxPreX;
            this.MaxPreY = maxPreY;
        }
        
        public void SetSub(final kRenderContext kRenderContext, final int clipX1, final int clipY1, final int clipX2, final int clipY2) {
            this.ClipX1 = clipX1;
            this.ClipY1 = clipY1;
            this.ClipX2 = clipX2;
            this.ClipY2 = clipY2;
            this.CurrentChunk = kRenderContext.CurrentChunk;
            this.RC_RootContext = kRenderContext.RC_RootContext;
        }
        
        static /* synthetic */ void access$2(final kRenderContext kRenderContext, final double posX) {
            kRenderContext.PosX = posX;
        }
        
        static /* synthetic */ void access$4(final kRenderContext kRenderContext, final double posY) {
            kRenderContext.PosY = posY;
        }
        
        static /* synthetic */ void access$6(final kRenderContext kRenderContext, final double posZ) {
            kRenderContext.PosZ = posZ;
        }
        
        public Image createImage() {
            final MemoryImageSource rc_imageMIS2 = this.RC_imageMIS2;
            this.RC_imageMIS2 = this.RC_imageMIS1;
            this.RC_imageMIS1 = rc_imageMIS2;
            final int[] currentChunk = this.CurrentChunk;
            this.CurrentChunk = this.Chunk2;
            this.Chunk2 = currentChunk;
            return this.app.createImage(this.RC_imageMIS2);
        }
    }
    
    public class kcamera
    {
        public double X;
        public double Y;
        public double Z;
        public double O1;
        public double O2;
        public double O3;
        public double FOV;
        
        public kcamera(final double x, final double y, final double z, final double o1, final double o2, final double o3, final double fov) {
            this.X = x;
            this.Y = y;
            this.Z = z;
            this.FOV = fov;
            this.O1 = o1;
            this.O2 = o2;
            this.O3 = o3;
        }
        
        public void set(final double x, final double y, final double z, final double o1, final double o2, final double o3, final double fov) {
            this.X = x;
            this.Y = y;
            this.Z = z;
            this.FOV = fov;
            this.O1 = o1;
            this.O2 = o2;
            this.O3 = o3;
        }
        
        public void target(final double x, final double y, final double z, double n, double n2, double n3, final double fov, final double o1) {
            this.X = x;
            this.Y = y;
            this.Z = z;
            this.FOV = fov;
            this.O1 = o1;
            n -= x;
            n2 -= y;
            n3 -= z;
            if (n3 == 0.0) {
                n3 = 1.0E-6;
            }
            double atan = Math.atan(n / n3);
            if (n3 > 0.0) {
                atan += 3.141592653589793;
            }
            final double a = atan + 3.141592653589793;
            this.O3 = a;
            double n4 = n3 * Math.cos(a) + n * Math.sin(a);
            if (n4 == 0.0) {
                n4 = 1.0E-6;
            }
            double atan2 = Math.atan(n2 / n4);
            if (n4 > 0.0) {
                atan2 += 3.141592653589793;
            }
            this.O2 = -(atan2 + 3.141592653589793);
        }
    }
    
    private class kparticle
    {
        double[] posx;
        double[] posy;
        double[] posz;
        double[] Vx;
        double[] Vy;
        double[] Vz;
        double[] Ax;
        double[] Ay;
        double[] Az;
        double aveX;
        double aveY;
        double aveZ;
        double[] force;
        int nbparticle;
        
        public kparticle(int nbparticle, final double n, final double n2, final double n3, final double n4, final double n5, final double n6, final double n7, final double n8) {
            if (nbparticle == 0) {
                nbparticle = 1;
            }
            this.nbparticle = nbparticle;
            this.posx = new double[nbparticle];
            this.posy = new double[nbparticle];
            this.posz = new double[nbparticle];
            this.Vx = new double[nbparticle];
            this.Vy = new double[nbparticle];
            this.Vz = new double[nbparticle];
            this.Ax = new double[nbparticle];
            this.Ay = new double[nbparticle];
            this.Az = new double[nbparticle];
            this.force = new double[nbparticle];
            this.reset(n, n2, n3, n4, n5, n6, n7, n8);
        }
        
        public void drawflare(final kcamera kcamera, final kTexture kTexture, final kRenderContext kRenderContext, final double n, final double n2) {
            final double cos = Math.cos(kcamera.O1);
            final double sin = Math.sin(kcamera.O1);
            final double cos2 = Math.cos(kcamera.O2);
            final double sin2 = Math.sin(kcamera.O2);
            final double cos3 = Math.cos(kcamera.O3);
            final double sin3 = Math.sin(kcamera.O3);
            for (int i = 0; i < this.nbparticle; ++i) {
                final double n3 = this.posx[i] - kcamera.X;
                final double n4 = this.posy[i] - kcamera.Y;
                final double n5 = this.posz[i] - kcamera.Z;
                final double n6 = -sin3 * n5 + cos3 * n3;
                final double n7 = n4;
                final double n8 = cos3 * n5 + sin3 * n3;
                final double n9 = n6;
                final double n10 = cos2 * n7 + sin2 * n8;
                kraycasting.this.rcs.flare3D(cos * n9 + sin * n10, -sin * n9 + cos * n10, -sin2 * n7 + cos2 * n8, n, n2, kcamera.FOV, kRenderContext, kTexture);
            }
        }
        
        public void drawsprit(final kcamera kcamera, final kTexture kTexture, final kRenderContext kRenderContext, final double n, final double n2) {
            final double cos = Math.cos(kcamera.O1);
            final double sin = Math.sin(kcamera.O1);
            final double cos2 = Math.cos(kcamera.O2);
            final double sin2 = Math.sin(kcamera.O2);
            final double cos3 = Math.cos(kcamera.O3);
            final double sin3 = Math.sin(kcamera.O3);
            for (int i = 0; i < this.nbparticle; ++i) {
                final double n3 = this.posx[i] - kcamera.X;
                final double n4 = this.posy[i] - kcamera.Y;
                final double n5 = this.posz[i] - kcamera.Z;
                final double n6 = -sin3 * n5 + cos3 * n3;
                final double n7 = n4;
                final double n8 = cos3 * n5 + sin3 * n3;
                final double n9 = n6;
                final double n10 = cos2 * n7 + sin2 * n8;
                kraycasting.this.rcs.Setzoom3D(cos * n9 + sin * n10, -sin * n9 + cos * n10, -sin2 * n7 + cos2 * n8, n, n2, kcamera.FOV, kRenderContext, kTexture, 256.0);
            }
        }
        
        public void move() {
            double n = 0.0;
            double n2 = 0.0;
            double n3 = 0.0;
            final double n4 = 1.0 / this.nbparticle;
            for (int i = 0; i < this.nbparticle; ++i) {
                final double[] vx = this.Vx;
                final int n5 = i;
                vx[n5] += this.Ax[i];
                final double[] vy = this.Vy;
                final int n6 = i;
                vy[n6] += this.Ay[i];
                final double[] vz = this.Vz;
                final int n7 = i;
                vz[n7] += this.Az[i];
                final double[] posx = this.posx;
                final int n8 = i;
                posx[n8] += this.Vx[i];
                final double[] posy = this.posy;
                final int n9 = i;
                posy[n9] += this.Vy[i];
                final double[] posz = this.posz;
                final int n10 = i;
                posz[n10] += this.Vz[i];
                n += this.posx[i];
                n2 += this.posy[i];
                n3 += this.posz[i];
            }
            this.aveX = n * n4;
            this.aveY = n2 * n4;
            this.aveZ = n3 * n4;
        }
        
        public void mvtfloor(final double n, final double n2, final double n3, final double n4) {
            final double aveX = this.aveX;
            final double aveY = this.aveY;
            final double aveZ = this.aveZ;
            for (int i = 0; i < this.nbparticle; ++i) {
                final double n5 = this.posx[i];
                final double n6 = this.posy[i];
                final double n7 = this.posz[i];
                if (n6 < n) {
                    this.posy[i] = n;
                    final double[] vy = this.Vy;
                    final int n8 = i;
                    vy[n8] *= -1.0;
                }
                if (n6 > n2) {
                    this.posy[i] = n2;
                    final double[] vy2 = this.Vy;
                    final int n9 = i;
                    vy2[n9] *= -1.0;
                }
                final double[] vx = this.Vx;
                final int n10 = i;
                vx[n10] *= n4;
                final double[] vy3 = this.Vy;
                final int n11 = i;
                vy3[n11] *= n4;
                final double[] vz = this.Vz;
                final int n12 = i;
                vz[n12] *= n4;
                this.Ax[i] = 0.0;
                this.Ay[i] = n3;
                this.Az[i] = 0.0;
            }
            this.move();
        }
        
        public void mvtgalax() {
            final double aveX = this.aveX;
            final double aveY = this.aveY;
            final double aveZ = this.aveZ;
            for (int i = 0; i < this.nbparticle; ++i) {
                final double n = this.posx[i];
                final double n2 = this.posy[i];
                final double n3 = this.posz[i];
                final double n4 = Math.pow(n * n + n2 * n2 + n3 * n3, 1.5) * 0.001;
                this.Ax[i] = -n * n4;
                this.Ay[i] = -n2 * n4;
                this.Az[i] = -n3 * n4;
            }
            this.move();
        }
        
        public void reset(final double n, final double n2, final double n3, final double n4, final double n5, final double n6, final double n7, final double n8) {
            double n9 = 0.0;
            double n10 = 0.0;
            double n11 = 0.0;
            final double n12 = 1.0 / this.nbparticle;
            double n13 = 0.0;
            double n14 = 0.0;
            final double n15 = 6.283185307179586 * n * n12;
            final double n16 = 6.283185307179586 * n2 * n12;
            for (int i = 0; i < this.nbparticle; ++i) {
                final double n17 = n3 + Math.random() * (n4 - n3);
                final double n18 = 0.0;
                final double n19 = Math.cos(n13) * n17;
                final double n20 = Math.sin(n13) * n17;
                this.posx[i] = Math.cos(n14) * n18 - Math.sin(n14) * n20;
                this.posy[i] = n19;
                this.posz[i] = Math.sin(n14) * n18 + Math.cos(n14) * n20;
                this.force[i] = 1.0;
                final double n21 = 0.05;
                this.Vx[i] = this.posz[i] * n21;
                this.Vy[i] = this.posx[i] * n21;
                this.Vz[i] = this.posy[i] * n21;
                n9 += this.posx[i];
                n10 += this.posy[i];
                n11 += this.posz[i];
                n13 += n15;
                n14 += n16;
            }
            this.aveX = n9 * n12;
            this.aveY = n10 * n12;
            this.aveZ = n11 * n12;
        }
    }
    
    private class kTable
    {
        double[] datetable;
        double[] floattable;
        int lengthtable;
        
        public kTable(final String s) {
            this.lengthtable = kraycasting.this.FindHowMany(s, "|") - 1;
            this.datetable = new double[this.lengthtable];
            this.floattable = new double[this.lengthtable];
            for (int i = 0; i < this.lengthtable; ++i) {
                final String trim = kraycasting.this.FindInto(s, 1 + i, "|").trim();
                this.floattable[i] = new Double(kraycasting.this.FindInto(trim, 1, ",").trim());
                this.datetable[i] = new Double(kraycasting.this.FindInto(trim, 0, ",").trim());
            }
        }
        
        public double spline(final double n) {
            final int n2 = this.lengthtable - 1;
            if (n <= this.datetable[0]) {
                return this.floattable[0];
            }
            int n3 = 0;
            int n4 = n2;
            int n5 = n3 + n4 >> 1;
            final int n6 = n4;
            if (n >= this.datetable[n6]) {
                return this.floattable[n6];
            }
            int n7;
            int n8;
            for (n7 = n5, n8 = n5 + 1; n < this.datetable[n7] || n >= this.datetable[n8]; n5 = (n7 = n3 + n4 >> 1), n8 = n5 + 1) {
                if (n < this.datetable[n7]) {
                    n4 = n5;
                }
                else {
                    n3 = n5;
                }
            }
            int n9;
            if (n5 == 0) {
                n9 = n7;
            }
            else {
                n9 = n7 - 1;
            }
            int n10;
            if (n5 + 1 == n2) {
                n10 = n8;
            }
            else {
                n10 = n8 + 1;
            }
            final double n11 = this.datetable[n7];
            final double n12 = this.datetable[n8];
            final double n13 = n12 - n11;
            final double n14 = (this.floattable[n8] - this.floattable[n9]) / (n12 - this.datetable[n9]);
            final double n15 = (this.floattable[n10] - this.floattable[n7]) / (this.datetable[n10] - n11);
            final double n16 = n14;
            final double n17 = this.floattable[n7];
            final double n18 = this.floattable[n8];
            final double n19 = n13;
            final double n20 = n19 * n19;
            final double n21 = (-(n19 * n15) - n16 * 2.0 * n19 + 3.0 * (n18 - n17)) / n20;
            final double n22 = (n15 - 2.0 * n21 * n19 - n16) / (3.0 * n20);
            final double n23 = n - n11;
            final double n24 = n23 * n23;
            return n22 * (n24 * n23) + n21 * n24 + n16 * n23 + n17;
        }
    }
    
    private class kTimer
    {
        private long StartTime;
        
        public kTimer() {
            this.StartTime = System.currentTimeMillis();
        }
        
        public int getTime() {
            return (int)((System.currentTimeMillis() - this.StartTime) / 10L);
        }
        
        public void reset() {
            this.StartTime = System.currentTimeMillis();
        }
    }
    
    private class kPixel
    {
        int u;
        int v;
        int r;
        int g;
        int b;
        double XI;
        double YI;
        double ZI;
        double XF;
        double YF;
        double ZF;
        
        public kPixel() {
            this.u = 0;
            this.v = 0;
            this.r = 0;
            this.g = 0;
            this.b = 0;
        }
    }
    
    public class helpfade
    {
        double Frd1;
        double Frv1;
        double Frd2;
        double Frv2;
        double Fgd1;
        double Fgv1;
        double Fgd2;
        double Fgv2;
        double Fbd1;
        double Fbv1;
        double Fbd2;
        double Fbv2;
    }
    
    abstract class kParameterX
    {
        int CodeParam;
        double a;
        double b;
        double c;
        int dtlength;
        kTable spltable;
        public Object kOBJ;
        
        public abstract Object Out();
        
        public abstract double doubleout(final double p0);
    }
    
    private class kParameterXCTE extends kParameterX
    {
        kParameterXCTE() {
        }
        
        public Object Out() {
            return null;
        }
        
        public double doubleout(final double n) {
            return super.a;
        }
    }
    
    private class kParameterXAFF extends kParameterX
    {
        kParameterXAFF() {
        }
        
        public Object Out() {
            return null;
        }
        
        public double doubleout(final double n) {
            return super.a + super.b * n;
        }
    }
    
    private class kParameterXRND extends kParameterX
    {
        kParameterXRND() {
        }
        
        public Object Out() {
            return null;
        }
        
        public double doubleout(final double n) {
            return super.a + Math.random() * (super.b - super.a);
        }
    }
    
    private class kParameterXSPLINE extends kParameterX
    {
        kParameterXSPLINE() {
        }
        
        public Object Out() {
            return null;
        }
        
        public double doubleout(final double n) {
            return super.spltable.spline(n - super.a);
        }
    }
    
    private class kParameterXSIN extends kParameterX
    {
        kParameterXSIN() {
        }
        
        public Object Out() {
            return null;
        }
        
        public double doubleout(final double n) {
            return super.a + Math.sin(super.b * n) * super.c;
        }
    }
    
    private class kParameterXCOS extends kParameterX
    {
        kParameterXCOS() {
        }
        
        public Object Out() {
            return null;
        }
        
        public double doubleout(final double n) {
            return super.a + Math.cos(super.b * n) * super.c;
        }
    }
    
    private class kParameterXOB extends kParameterX
    {
        kParameterXOB() {
        }
        
        public Object Out() {
            if (super.kOBJ == null) {
                return kraycasting.this.RenderContext;
            }
            return super.kOBJ;
        }
        
        public double doubleout(final double n) {
            return 0.0;
        }
    }
    
    private class kEffectX
    {
        int keffectcode;
        int knbparam;
        private kParameterX[] kParameterXEffectX;
        helpfade hhf;
        
        kEffectX() {
            this.hhf = new helpfade();
        }
        
        static /* synthetic */ void access$1(final kEffectX kEffectX, final kParameterX[] kParameterXEffectX) {
            kEffectX.kParameterXEffectX = kParameterXEffectX;
        }
        
        public void rendah(final int n) {
            final double doubleValue = new Double(n);
            if (this.keffectcode == 1) {
                kraycasting.this.rcs.Position3D(this.kParameterXEffectX[0].doubleout(doubleValue), this.kParameterXEffectX[1].doubleout(doubleValue), 0.0, 0.0, 0.0, 0.0, 0.0, (kRenderContext)this.kParameterXEffectX[9].Out());
                kraycasting.this.rcs.findtwirl(this.kParameterXEffectX[2].doubleout(doubleValue), this.kParameterXEffectX[3].doubleout(doubleValue), this.kParameterXEffectX[4].doubleout(doubleValue), this.kParameterXEffectX[5].doubleout(doubleValue), this.kParameterXEffectX[6].doubleout(doubleValue), this.kParameterXEffectX[7].doubleout(doubleValue), (kRenderContext)this.kParameterXEffectX[9].Out());
                kraycasting.this.rcs.draw((kTexture)this.kParameterXEffectX[8].Out(), (kRenderContext)this.kParameterXEffectX[9].Out());
            }
            if (this.keffectcode == 2) {
                kraycasting.this.rcs.SetzoomP(this.kParameterXEffectX[1].doubleout(doubleValue), this.kParameterXEffectX[2].doubleout(doubleValue), this.kParameterXEffectX[3].doubleout(doubleValue), this.kParameterXEffectX[4].doubleout(doubleValue), (kRenderContext)this.kParameterXEffectX[5].Out(), (kTexture)this.kParameterXEffectX[0].Out(), this.kParameterXEffectX[6].doubleout(doubleValue));
            }
            if (this.keffectcode == 3) {
                this.hhf.Frd1 = this.kParameterXEffectX[5].doubleout(doubleValue);
                this.hhf.Frv1 = this.kParameterXEffectX[6].doubleout(doubleValue);
                this.hhf.Frd2 = this.kParameterXEffectX[7].doubleout(doubleValue);
                this.hhf.Frv2 = this.kParameterXEffectX[8].doubleout(doubleValue);
                this.hhf.Fgd1 = this.kParameterXEffectX[9].doubleout(doubleValue);
                this.hhf.Fgv1 = this.kParameterXEffectX[10].doubleout(doubleValue);
                this.hhf.Fgd2 = this.kParameterXEffectX[11].doubleout(doubleValue);
                this.hhf.Fgv2 = this.kParameterXEffectX[12].doubleout(doubleValue);
                this.hhf.Fbd1 = this.kParameterXEffectX[13].doubleout(doubleValue);
                this.hhf.Fbv1 = this.kParameterXEffectX[14].doubleout(doubleValue);
                this.hhf.Fbd2 = this.kParameterXEffectX[15].doubleout(doubleValue);
                this.hhf.Fbv2 = this.kParameterXEffectX[16].doubleout(doubleValue);
                kraycasting.this.rcs.Position3D(0.0, 0.0, ((kcamera)this.kParameterXEffectX[0].Out()).Z, ((kcamera)this.kParameterXEffectX[0].Out()).O1, ((kcamera)this.kParameterXEffectX[0].Out()).O2, ((kcamera)this.kParameterXEffectX[0].Out()).O3, ((kcamera)this.kParameterXEffectX[0].Out()).FOV, (kRenderContext)this.kParameterXEffectX[4].Out());
                kraycasting.this.rcs.findtunnel(this.kParameterXEffectX[1].doubleout(doubleValue), this.kParameterXEffectX[2].doubleout(doubleValue), (kRenderContext)this.kParameterXEffectX[4].Out(), this.hhf);
                kraycasting.this.rcs.draw((kTexture)this.kParameterXEffectX[3].Out(), (kRenderContext)this.kParameterXEffectX[4].Out());
            }
            if (this.keffectcode == 4) {
                this.hhf.Frd1 = this.kParameterXEffectX[7].doubleout(doubleValue);
                this.hhf.Frv1 = this.kParameterXEffectX[8].doubleout(doubleValue);
                this.hhf.Frd2 = this.kParameterXEffectX[9].doubleout(doubleValue);
                this.hhf.Frv2 = this.kParameterXEffectX[10].doubleout(doubleValue);
                this.hhf.Fgd1 = this.kParameterXEffectX[11].doubleout(doubleValue);
                this.hhf.Fgv1 = this.kParameterXEffectX[12].doubleout(doubleValue);
                this.hhf.Fgd2 = this.kParameterXEffectX[13].doubleout(doubleValue);
                this.hhf.Fgv2 = this.kParameterXEffectX[14].doubleout(doubleValue);
                this.hhf.Fbd1 = this.kParameterXEffectX[15].doubleout(doubleValue);
                this.hhf.Fbv1 = this.kParameterXEffectX[16].doubleout(doubleValue);
                this.hhf.Fbd2 = this.kParameterXEffectX[17].doubleout(doubleValue);
                this.hhf.Fbv2 = this.kParameterXEffectX[18].doubleout(doubleValue);
                kraycasting.this.rcs.Position3D(((kcamera)this.kParameterXEffectX[0].Out()).X, ((kcamera)this.kParameterXEffectX[0].Out()).Y, ((kcamera)this.kParameterXEffectX[0].Out()).Z, ((kcamera)this.kParameterXEffectX[0].Out()).O1, ((kcamera)this.kParameterXEffectX[0].Out()).O2, ((kcamera)this.kParameterXEffectX[0].Out()).O3, ((kcamera)this.kParameterXEffectX[0].Out()).FOV, (kRenderContext)this.kParameterXEffectX[4].Out());
                kraycasting.this.rcs.findfloor(this.kParameterXEffectX[1].doubleout(doubleValue), this.kParameterXEffectX[2].doubleout(doubleValue), this.kParameterXEffectX[5].doubleout(doubleValue), this.kParameterXEffectX[6].doubleout(doubleValue), (kRenderContext)this.kParameterXEffectX[4].Out(), this.hhf);
                kraycasting.this.rcs.draw((kTexture)this.kParameterXEffectX[3].Out(), (kRenderContext)this.kParameterXEffectX[4].Out());
            }
            if (this.keffectcode == 5) {
                kraycasting.this.rcs.Setzoom3D(this.kParameterXEffectX[0].doubleout(doubleValue), this.kParameterXEffectX[1].doubleout(doubleValue), this.kParameterXEffectX[2].doubleout(doubleValue), this.kParameterXEffectX[3].doubleout(doubleValue), this.kParameterXEffectX[4].doubleout(doubleValue), this.kParameterXEffectX[5].doubleout(doubleValue), (kRenderContext)this.kParameterXEffectX[7].Out(), (kTexture)this.kParameterXEffectX[6].Out(), this.kParameterXEffectX[8].doubleout(doubleValue));
            }
            if (this.keffectcode == 6) {
                ((kRenderContext)this.kParameterXEffectX[0].Out()).SetRect(kraycasting.this.RenderContext, this.kParameterXEffectX[1].doubleout(doubleValue), this.kParameterXEffectX[2].doubleout(doubleValue), this.kParameterXEffectX[3].doubleout(doubleValue), this.kParameterXEffectX[4].doubleout(doubleValue));
            }
            if (this.keffectcode == 7) {
                kraycasting.this.rcs.flare3D(this.kParameterXEffectX[0].doubleout(doubleValue), this.kParameterXEffectX[1].doubleout(doubleValue), this.kParameterXEffectX[2].doubleout(doubleValue), this.kParameterXEffectX[3].doubleout(doubleValue), this.kParameterXEffectX[4].doubleout(doubleValue), this.kParameterXEffectX[5].doubleout(doubleValue), (kRenderContext)this.kParameterXEffectX[7].Out(), (kTexture)this.kParameterXEffectX[6].Out());
            }
            if (this.keffectcode == 8) {
                kraycasting.this.rcs.MotionBlur((kRenderContext)this.kParameterXEffectX[0].Out());
            }
            if (this.keffectcode == 9) {
                final double doubleout = this.kParameterXEffectX[4].doubleout(doubleValue);
                if (doubleout == 0.0) {
                    ((kparticle)this.kParameterXEffectX[0].Out()).mvtgalax();
                }
                else if (doubleout == 1.0) {
                    ((kparticle)this.kParameterXEffectX[0].Out()).mvtfloor(this.kParameterXEffectX[9].doubleout(doubleValue), this.kParameterXEffectX[10].doubleout(doubleValue), this.kParameterXEffectX[11].doubleout(doubleValue), this.kParameterXEffectX[12].doubleout(doubleValue));
                }
                final double doubleout2 = this.kParameterXEffectX[5].doubleout(doubleValue);
                if (doubleout2 == 0.0) {
                    ((kparticle)this.kParameterXEffectX[0].Out()).drawflare((kcamera)this.kParameterXEffectX[6].Out(), (kTexture)this.kParameterXEffectX[2].Out(), (kRenderContext)this.kParameterXEffectX[1].Out(), this.kParameterXEffectX[7].doubleout(doubleValue), this.kParameterXEffectX[8].doubleout(doubleValue));
                }
                if (doubleout2 == 2.0) {
                    ((kparticle)this.kParameterXEffectX[0].Out()).drawsprit((kcamera)this.kParameterXEffectX[6].Out(), (kTexture)this.kParameterXEffectX[2].Out(), (kRenderContext)this.kParameterXEffectX[1].Out(), this.kParameterXEffectX[7].doubleout(doubleValue), this.kParameterXEffectX[8].doubleout(doubleValue));
                }
            }
            if (this.keffectcode == 10) {
                ((kcamera)this.kParameterXEffectX[0].Out()).set(this.kParameterXEffectX[1].doubleout(doubleValue), this.kParameterXEffectX[2].doubleout(doubleValue), this.kParameterXEffectX[3].doubleout(doubleValue), this.kParameterXEffectX[4].doubleout(doubleValue), this.kParameterXEffectX[5].doubleout(doubleValue), this.kParameterXEffectX[6].doubleout(doubleValue), this.kParameterXEffectX[7].doubleout(doubleValue));
            }
            if (this.keffectcode == 11) {
                ((kcamera)this.kParameterXEffectX[0].Out()).target(this.kParameterXEffectX[1].doubleout(doubleValue), this.kParameterXEffectX[2].doubleout(doubleValue), this.kParameterXEffectX[3].doubleout(doubleValue), this.kParameterXEffectX[4].doubleout(doubleValue), this.kParameterXEffectX[5].doubleout(doubleValue), this.kParameterXEffectX[6].doubleout(doubleValue), this.kParameterXEffectX[7].doubleout(doubleValue), this.kParameterXEffectX[8].doubleout(doubleValue));
            }
            if (this.keffectcode == 12) {
                if (doubleValue < this.kParameterXEffectX[1].doubleout(doubleValue)) {
                    ((kSound)this.kParameterXEffectX[0].Out()).playedonce = false;
                }
                else if (!((kSound)this.kParameterXEffectX[0].Out()).playedonce) {
                    ((kSound)this.kParameterXEffectX[0].Out()).PlayOne();
                }
            }
        }
    }
    
    private class kSoundCommand
    {
        int commandcode;
        kSound snd;
        
        kSoundCommand() {
        }
        
        public void MakeSoundCommand() {
            if (this.commandcode == 0) {
                this.snd.StopOne();
            }
            if (this.commandcode == 1) {
                this.snd.PlayOne();
            }
            if (this.commandcode == 2) {
                this.snd.LoopOne();
            }
        }
    }
    
    private class kPartX
    {
        int NBEffect;
        kEffectX[] kEffectXPartX;
        int TimeLength;
        int TimeStart;
        int NBSoundCommands;
        kSoundCommand[] SoundCommands;
        
        kPartX() {
        }
        
        public void rendah(final int n) {
            final int n2 = n - this.TimeLength + this.TimeStart;
            for (int i = 0; i < this.NBEffect; ++i) {
                this.kEffectXPartX[i].rendah(n2);
            }
            if (kraycasting.this.THEkScript.lastpartplayed != this && this.SoundCommands != null) {
                for (int j = 0; j < this.NBSoundCommands; ++j) {
                    this.SoundCommands[j].MakeSoundCommand();
                }
            }
            kraycasting.this.THEkScript.lastpartplayed = this;
        }
    }
    
    private class kScript
    {
        int NBPart;
        int EndDate;
        int EndPref;
        private kPartX[] kPartXscript;
        kPartX lastpartplayed;
        
        public kScript(final String s) {
            final String[] splitSTag = kraycasting.this.SplitSTag(s, "KIMAGES");
            final String[] splitSTag2 = kraycasting.this.SplitSTag(s, "KSOUNDS");
            final String[] splitSTag3 = kraycasting.this.SplitSTag(s, "KRECT");
            final String[] splitSTag4 = kraycasting.this.SplitSTag(s, "KTABLE");
            final String[] splitSTag5 = kraycasting.this.SplitSTag(s, "KSCRIPT");
            final String[] splitSTag6 = kraycasting.this.SplitSTag(s, "KTICLE");
            final String[] splitSTag7 = kraycasting.this.SplitSTag(s, "KCAM");
            System.out.println("table");
            kraycasting.access$4(kraycasting.this, new Hashtable());
            for (int i = 0; i < splitSTag4.length; ++i) {
                kraycasting.this.TableHash.put(kraycasting.this.FindInto(splitSTag4[i], 0, "|").trim(), new kTable(splitSTag4[i].trim()));
            }
            System.out.println("image");
            kraycasting.access$6(kraycasting.this, new Hashtable());
            for (int j = 0; j < splitSTag.length; ++j) {
                kraycasting.this.ImageHash.put(kraycasting.this.FindInto(splitSTag[j], 0, "|").trim(), new kTexture(kraycasting.this.getImage(kraycasting.this.getDocumentBase(), kraycasting.this.FindInto(splitSTag[j], 1, "|").trim())));
            }
            System.out.println("particl");
            kraycasting.access$8(kraycasting.this, new Hashtable());
            for (int k = 0; k < splitSTag6.length; ++k) {
                final Hashtable access$7 = kraycasting.this.partcHash;
                final String trim = kraycasting.this.FindInto(splitSTag6[k], 0, "|").trim();
                kraycasting.this.getClass();
                access$7.put(trim, new kparticle(new Integer(kraycasting.this.FindInto(splitSTag6[k], 1, "|").trim()), new Double(kraycasting.this.FindInto(splitSTag6[k], 2, "|").trim()), new Double(kraycasting.this.FindInto(splitSTag6[k], 3, "|").trim()), new Double(kraycasting.this.FindInto(splitSTag6[k], 4, "|").trim()), new Double(kraycasting.this.FindInto(splitSTag6[k], 5, "|").trim()), new Double(kraycasting.this.FindInto(splitSTag6[k], 6, "|").trim()), new Double(kraycasting.this.FindInto(splitSTag6[k], 7, "|").trim()), new Double(kraycasting.this.FindInto(splitSTag6[k], 8, "|").trim()), new Double(kraycasting.this.FindInto(splitSTag6[k], 9, "|").trim())));
            }
            System.out.println("camera");
            kraycasting.access$10(kraycasting.this, new Hashtable());
            for (int l = 0; l < splitSTag7.length; ++l) {
                final Hashtable access$8 = kraycasting.this.CamerHash;
                final String trim2 = kraycasting.this.FindInto(splitSTag7[l], 0, "|").trim();
                kraycasting.this.getClass();
                access$8.put(trim2, new kcamera(new Integer(kraycasting.this.FindInto(splitSTag7[l], 1, "|").trim()), new Integer(kraycasting.this.FindInto(splitSTag7[l], 2, "|").trim()), new Integer(kraycasting.this.FindInto(splitSTag7[l], 3, "|").trim()), new Integer(kraycasting.this.FindInto(splitSTag7[l], 4, "|").trim()), new Integer(kraycasting.this.FindInto(splitSTag7[l], 5, "|").trim()), new Integer(kraycasting.this.FindInto(splitSTag7[l], 6, "|").trim()), new Integer(kraycasting.this.FindInto(splitSTag7[l], 7, "|").trim())));
            }
            System.out.println("rect");
            kraycasting.access$12(kraycasting.this, new Hashtable());
            for (int n = 0; n < splitSTag3.length; ++n) {
                final Hashtable access$9 = kraycasting.this.RectaHash;
                final String trim3 = kraycasting.this.FindInto(splitSTag3[n], 0, "|").trim();
                kraycasting.this.getClass();
                access$9.put(trim3, new kRenderContext(kraycasting.this.RenderContext, new Double(kraycasting.this.FindInto(splitSTag3[n], 1, "|").trim()), new Double(kraycasting.this.FindInto(splitSTag3[n], 2, "|").trim()), new Double(kraycasting.this.FindInto(splitSTag3[n], 3, "|").trim()), new Double(kraycasting.this.FindInto(splitSTag3[n], 4, "|").trim())));
            }
            System.out.println("sound");
            kraycasting.access$14(kraycasting.this, new Hashtable());
            for (int n2 = 0; n2 < splitSTag2.length; ++n2) {
                kraycasting.this.SoundHash.put(kraycasting.this.FindInto(splitSTag2[n2], 0, "|").trim(), new kSound(kraycasting.this.getAudioClip(kraycasting.this.getDocumentBase(), kraycasting.this.FindInto(splitSTag2[n2], 1, "|").trim())));
            }
            final String[] splitSTag8 = kraycasting.this.SplitSTag(splitSTag5[0], "KPART");
            this.NBPart = splitSTag8.length;
            this.kPartXscript = new kPartX[this.NBPart + 1];
            final String[] splitSTag9 = kraycasting.this.SplitSTag(splitSTag5[0], "KEND");
            this.EndPref = 0;
            if (splitSTag9[0].trim().toLowerCase().compareTo("loop") == 0) {
                this.EndPref = 1;
            }
            int n3;
            for (n3 = 0; n3 < splitSTag8.length; ++n3) {
                final String[] splitSTag10 = kraycasting.this.SplitSTag(splitSTag8[n3], "D");
                final String[] splitSTag11 = kraycasting.this.SplitSTag(splitSTag8[n3], "S");
                this.kPartXscript[n3] = new kPartX();
                if (splitSTag11 != null) {
                    this.kPartXscript[n3].NBSoundCommands = splitSTag11.length;
                    this.kPartXscript[n3].SoundCommands = new kSoundCommand[this.kPartXscript[n3].NBSoundCommands];
                    for (int n4 = 0; n4 < this.kPartXscript[n3].NBSoundCommands; ++n4) {
                        int commandcode = 0;
                        final String lowerCase = kraycasting.this.FindInto(splitSTag11[n4], 1, ",").trim().toLowerCase();
                        if (lowerCase.compareTo("play") == 0) {
                            commandcode = 1;
                        }
                        if (lowerCase.compareTo("loop") == 0) {
                            commandcode = 2;
                        }
                        this.kPartXscript[n3].SoundCommands[n4] = new kSoundCommand();
                        this.kPartXscript[n3].SoundCommands[n4].commandcode = commandcode;
                        this.kPartXscript[n3].SoundCommands[n4].snd = (kSound)kraycasting.this.SoundHash.get(kraycasting.this.FindInto(splitSTag11[n4], 0, ","));
                    }
                }
                this.kPartXscript[n3].TimeLength = new Integer(kraycasting.this.FindInto(splitSTag10[0], 0, ",").trim());
                this.kPartXscript[n3].TimeStart = new Integer(kraycasting.this.FindInto(splitSTag10[0], 1, ",").trim());
                final String[] splitSTag12 = kraycasting.this.SplitSTag(splitSTag8[n3], "Fx");
                this.kPartXscript[n3].NBEffect = splitSTag12.length;
                this.kPartXscript[n3].kEffectXPartX = new kEffectX[this.kPartXscript[n3].NBEffect];
                for (int n5 = 0; n5 < splitSTag12.length; ++n5) {
                    this.kPartXscript[n3].kEffectXPartX[n5] = new kEffectX();
                    final String[] splitSTag13 = kraycasting.this.SplitSTag(splitSTag12[n5], "Pa");
                    this.kPartXscript[n3].kEffectXPartX[n5].knbparam = splitSTag13.length;
                    kEffectX.access$1(this.kPartXscript[n3].kEffectXPartX[n5], new kParameterX[this.kPartXscript[n3].kEffectXPartX[n5].knbparam]);
                    final String lowerCase2 = kraycasting.this.FindInto(splitSTag13[0], 0, "|").trim().toLowerCase();
                    int keffectcode = 0;
                    if (lowerCase2.compareTo("twirl") == 0) {
                        keffectcode = 1;
                    }
                    if (lowerCase2.compareTo("sprit") == 0) {
                        keffectcode = 2;
                    }
                    if (lowerCase2.compareTo("tunnel") == 0) {
                        keffectcode = 3;
                    }
                    if (lowerCase2.compareTo("ground") == 0) {
                        keffectcode = 4;
                    }
                    if (lowerCase2.compareTo("sprit3d") == 0) {
                        keffectcode = 5;
                    }
                    if (lowerCase2.compareTo("setrect") == 0) {
                        keffectcode = 6;
                    }
                    if (lowerCase2.compareTo("flare3d") == 0) {
                        keffectcode = 7;
                    }
                    if (lowerCase2.compareTo("motionblur") == 0) {
                        keffectcode = 8;
                    }
                    if (lowerCase2.compareTo("particle") == 0) {
                        keffectcode = 9;
                    }
                    if (lowerCase2.compareTo("setcamcoord") == 0) {
                        keffectcode = 10;
                    }
                    if (lowerCase2.compareTo("setcamtarget") == 0) {
                        keffectcode = 11;
                    }
                    if (lowerCase2.compareTo("evesound") == 0) {
                        keffectcode = 12;
                    }
                    this.kPartXscript[n3].kEffectXPartX[n5].keffectcode = keffectcode;
                    System.out.println(" ");
                    for (int n6 = 1; n6 < this.kPartXscript[n3].kEffectXPartX[n5].knbparam; ++n6) {
                        final String lowerCase3 = kraycasting.this.FindInto(splitSTag13[n6], 0, "|").trim().toLowerCase();
                        System.out.print("   Pa:" + lowerCase3);
                        if (lowerCase3.compareTo("ima") == 0) {
                            final kParameterXOB kParameterXOB = new kParameterXOB();
                            kParameterXOB.CodeParam = 1;
                            kParameterXOB.kOBJ = kraycasting.this.ImageHash.get(kraycasting.this.FindInto(splitSTag13[n6], 1, "|"));
                            this.kPartXscript[n3].kEffectXPartX[n5].kParameterXEffectX[n6 - 1] = kParameterXOB;
                        }
                        if (lowerCase3.compareTo("cos") == 0) {
                            final kParameterXCOS kParameterXCOS = new kParameterXCOS();
                            kParameterXCOS.CodeParam = 2;
                            kParameterXCOS.a = new Double(kraycasting.this.FindInto(splitSTag13[n6], 1, "|"));
                            kParameterXCOS.b = new Double(kraycasting.this.FindInto(splitSTag13[n6], 2, "|"));
                            kParameterXCOS.c = new Double(kraycasting.this.FindInto(splitSTag13[n6], 3, "|"));
                            this.kPartXscript[n3].kEffectXPartX[n5].kParameterXEffectX[n6 - 1] = kParameterXCOS;
                        }
                        if (lowerCase3.compareTo("sin") == 0) {
                            final kParameterXSIN kParameterXSIN = new kParameterXSIN();
                            kParameterXSIN.CodeParam = 3;
                            kParameterXSIN.a = new Double(kraycasting.this.FindInto(splitSTag13[n6], 1, "|"));
                            kParameterXSIN.b = new Double(kraycasting.this.FindInto(splitSTag13[n6], 2, "|"));
                            kParameterXSIN.c = new Double(kraycasting.this.FindInto(splitSTag13[n6], 3, "|"));
                            this.kPartXscript[n3].kEffectXPartX[n5].kParameterXEffectX[n6 - 1] = kParameterXSIN;
                        }
                        if (lowerCase3.compareTo("cte") == 0) {
                            final kParameterXCTE kParameterXCTE = new kParameterXCTE();
                            kParameterXCTE.CodeParam = 4;
                            kParameterXCTE.a = new Double(kraycasting.this.FindInto(splitSTag13[n6], 1, "|"));
                            this.kPartXscript[n3].kEffectXPartX[n5].kParameterXEffectX[n6 - 1] = kParameterXCTE;
                        }
                        if (lowerCase3.compareTo("rnd") == 0) {
                            final kParameterXRND kParameterXRND = new kParameterXRND();
                            kParameterXRND.CodeParam = 5;
                            kParameterXRND.a = new Double(kraycasting.this.FindInto(splitSTag13[n6], 1, "|"));
                            kParameterXRND.b = new Double(kraycasting.this.FindInto(splitSTag13[n6], 2, "|"));
                            this.kPartXscript[n3].kEffectXPartX[n5].kParameterXEffectX[n6 - 1] = kParameterXRND;
                        }
                        if (lowerCase3.compareTo("spl") == 0) {
                            final kParameterXSPLINE kParameterXSPLINE = new kParameterXSPLINE();
                            kParameterXSPLINE.CodeParam = 6;
                            kParameterXSPLINE.spltable = kraycasting.this.TableHash.get(kraycasting.this.FindInto(splitSTag13[n6], 1, "|"));
                            kParameterXSPLINE.a = new Double(kraycasting.this.FindInto(splitSTag13[n6], 2, "|"));
                            this.kPartXscript[n3].kEffectXPartX[n5].kParameterXEffectX[n6 - 1] = kParameterXSPLINE;
                        }
                        if (lowerCase3.compareTo("aff") == 0) {
                            final kParameterXAFF kParameterXAFF = new kParameterXAFF();
                            kParameterXAFF.CodeParam = 7;
                            kParameterXAFF.a = new Double(kraycasting.this.FindInto(splitSTag13[n6], 1, "|"));
                            kParameterXAFF.b = new Double(kraycasting.this.FindInto(splitSTag13[n6], 2, "|"));
                            this.kPartXscript[n3].kEffectXPartX[n5].kParameterXEffectX[n6 - 1] = kParameterXAFF;
                        }
                        if (lowerCase3.compareTo("rec") == 0) {
                            final kParameterXOB kParameterXOB2 = new kParameterXOB();
                            kParameterXOB2.CodeParam = 8;
                            kParameterXOB2.kOBJ = kraycasting.this.RectaHash.get(kraycasting.this.FindInto(splitSTag13[n6], 1, "|"));
                            this.kPartXscript[n3].kEffectXPartX[n5].kParameterXEffectX[n6 - 1] = kParameterXOB2;
                        }
                        if (lowerCase3.compareTo("par") == 0) {
                            final kParameterXOB kParameterXOB3 = new kParameterXOB();
                            kParameterXOB3.CodeParam = 9;
                            kParameterXOB3.kOBJ = kraycasting.this.partcHash.get(kraycasting.this.FindInto(splitSTag13[n6], 1, "|"));
                            this.kPartXscript[n3].kEffectXPartX[n5].kParameterXEffectX[n6 - 1] = kParameterXOB3;
                        }
                        if (lowerCase3.compareTo("cam") == 0) {
                            final kParameterXOB kParameterXOB4 = new kParameterXOB();
                            kParameterXOB4.CodeParam = 10;
                            kParameterXOB4.kOBJ = kraycasting.this.CamerHash.get(kraycasting.this.FindInto(splitSTag13[n6], 1, "|"));
                            this.kPartXscript[n3].kEffectXPartX[n5].kParameterXEffectX[n6 - 1] = kParameterXOB4;
                        }
                        if (lowerCase3.compareTo("snd") == 0) {
                            final kParameterXOB kParameterXOB5 = new kParameterXOB();
                            kParameterXOB5.CodeParam = 11;
                            kParameterXOB5.kOBJ = kraycasting.this.SoundHash.get(kraycasting.this.FindInto(splitSTag13[n6], 1, "|"));
                            this.kPartXscript[n3].kEffectXPartX[n5].kParameterXEffectX[n6 - 1] = kParameterXOB5;
                        }
                    }
                }
            }
            this.kPartXscript[n3] = new kPartX();
            int endDate = 0;
            for (int n7 = 0; n7 < this.NBPart; ++n7) {
                final int timeLength = this.kPartXscript[n7].TimeLength;
                this.kPartXscript[n7].TimeLength = endDate;
                endDate += timeLength;
            }
            this.kPartXscript[n3].TimeLength = endDate;
            this.EndDate = endDate;
        }
        
        public kPartX part(final int n) {
            int n2 = 0;
            for (int i = 0; i < this.NBPart; ++i) {
                if (this.kPartXscript[i + 1].TimeLength >= n && this.kPartXscript[i].TimeLength < n) {
                    n2 = i;
                }
            }
            if (n >= this.EndDate) {
                if (this.EndPref == 1) {
                    kraycasting.this.kTime.reset();
                }
                else {
                    kraycasting.access$17(kraycasting.this, true);
                }
            }
            return this.kPartXscript[n2];
        }
    }
    
    private class raycasting
    {
        private kRenderContext OneRec;
        private kRenderContext Rootrc;
        
        public raycasting(final kRenderContext rootrc) {
            this.Rootrc = rootrc;
            this.OneRec = new kRenderContext(rootrc);
        }
        
        public void MotionBlur(final kRenderContext kRenderContext) {
            final int n = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final int n2 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            final int modulo = kRenderContext.Modulo;
            int n3 = kRenderContext.ClipX1 + kRenderContext.ClipY1 * kRenderContext.Modulo;
            final int[] currentChunk = kRenderContext.RC_RootContext.CurrentChunk;
            final int[] chunk2 = kRenderContext.RC_RootContext.Chunk2;
            if (n == 0) {
                return;
            }
            if (n2 == 0) {
                return;
            }
            for (int i = 0; i < n2; ++i) {
                int n4 = n3;
                for (int j = 0; j < n; ++j) {
                    currentChunk[n4++] = (0xFF000000 | (chunk2[n4] & 0xFCFCFF) + (currentChunk[n4] & 0xFCFCFF) >> 1);
                }
                n3 += modulo;
            }
        }
        
        public void Position3D(final double n, final double n2, final double n3, final double n4, final double n5, final double n6, final double n7, final kRenderContext kRenderContext) {
            if (!kRenderContext.drawable) {
                return;
            }
            final double cos = Math.cos(n4);
            final double sin = Math.sin(n4);
            final double cos2 = Math.cos(n5);
            final double sin2 = Math.sin(n5);
            final double cos3 = Math.cos(n6);
            final double sin3 = Math.sin(n6);
            kraycasting.kRenderContext.access$2(kRenderContext, n);
            kraycasting.kRenderContext.access$4(kRenderContext, n2);
            kraycasting.kRenderContext.access$6(kRenderContext, n3);
            for (int i = 0; i < kRenderContext.MaxPreY; ++i) {
                for (int j = 0; j < kRenderContext.MaxPreX; ++j) {
                    final double xi = kRenderContext.Pretable[j][i].XI;
                    final double yi = kRenderContext.Pretable[j][i].YI;
                    final double n8 = cos * xi - sin * yi;
                    final double n9 = sin * xi + cos * yi;
                    final double n10 = n8;
                    final double yf = cos2 * n9 - sin2 * n7;
                    final double n11 = sin2 * n9 + cos2 * n7;
                    kRenderContext.Pretable[j][i].XF = sin3 * n11 + cos3 * n10;
                    kRenderContext.Pretable[j][i].YF = yf;
                    kRenderContext.Pretable[j][i].ZF = cos3 * n11 - sin3 * n10;
                }
            }
        }
        
        public void Setzoom3D(final double n, final double n2, final double n3, final double n4, final double n5, final double n6, final kRenderContext kRenderContext, final kTexture kTexture, final double n7) {
            if (!kRenderContext.drawable) {
                return;
            }
            final double n8 = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final double n9 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            final double n10 = n6 / n3;
            if (n3 <= 0.01) {
                return;
            }
            final double n11 = kTexture.Modulo / n8;
            final double n12 = kTexture.Height / n9;
            final double n13 = n * n10;
            final double n14 = n2 * n10;
            double n15 = 0.0;
            double n16 = 0.0;
            double n17 = kTexture.Modulo - 1;
            double n18 = kTexture.Height - 1;
            final double n19 = n11 * 0.5 * n4 * n10;
            final double n20 = n12 * 0.5 * n5 * n10;
            final double n21 = n13 - n19;
            final double n22 = n14 - n20;
            final double n23 = n13 + n19;
            final double n24 = n14 + n20;
            double n25 = n21 * n8 + kRenderContext.ClipX1 + n8 * 0.5;
            double n26 = n22 * n9 + kRenderContext.ClipY1 + n9 * 0.5;
            double n27 = n23 * n8 + kRenderContext.ClipX1 + n8 * 0.5;
            double n28 = n24 * n9 + kRenderContext.ClipY1 + n9 * 0.5;
            if (n25 > n27) {
                final double n29 = n27;
                n27 = n25;
                n25 = n29;
                final double n30 = n17;
                n17 = n15;
                n15 = n30;
            }
            if (n26 > n28) {
                final double n31 = n28;
                n28 = n26;
                n26 = n31;
                final double n32 = n18;
                n18 = n16;
                n16 = n32;
            }
            if (n27 > kRenderContext.ClipX2) {
                n17 = (kRenderContext.ClipX2 - n25) * (n17 - n15) / (n27 - n25);
                n27 = kRenderContext.ClipX2;
            }
            if (n25 < kRenderContext.ClipX1) {
                n15 = (kRenderContext.ClipX1 - n25) * (n17 - n15) / (n27 - n25);
                n25 = kRenderContext.ClipX1;
            }
            if (n28 > kRenderContext.ClipY2) {
                n18 = (kRenderContext.ClipY2 - n26) * (n18 - n16) / (n28 - n26);
                n28 = kRenderContext.ClipY2;
            }
            if (n26 < kRenderContext.ClipY1) {
                n16 = (kRenderContext.ClipY1 - n26) * (n18 - n16) / (n28 - n26);
                n26 = kRenderContext.ClipY1;
            }
            this.Rootrc.Pretable[0][0].u = (int)(n15 * 65536.0);
            this.Rootrc.Pretable[0][1].u = this.Rootrc.Pretable[0][0].u;
            this.Rootrc.Pretable[1][0].u = (int)(n17 * 65536.0);
            this.Rootrc.Pretable[1][1].u = this.Rootrc.Pretable[1][0].u;
            this.Rootrc.Pretable[0][0].v = (int)(n16 * 65536.0);
            this.Rootrc.Pretable[1][0].v = this.Rootrc.Pretable[0][0].v;
            this.Rootrc.Pretable[0][1].v = (int)(n18 * 65536.0);
            this.Rootrc.Pretable[1][1].v = this.Rootrc.Pretable[0][1].v;
            this.OneRec.SetSub(kRenderContext, (int)n25, (int)n26, (int)n27, (int)n28);
            this.drawrecUVSA(kTexture, this.OneRec, this.Rootrc.Pretable[0][0], this.Rootrc.Pretable[1][0], this.Rootrc.Pretable[0][1], this.Rootrc.Pretable[1][1], (int)n7);
        }
        
        public void SetzoomP(double n, double n2, double n3, double n4, final kRenderContext kRenderContext, final kTexture kTexture, final double n5) {
            if (!kRenderContext.drawable) {
                return;
            }
            final double n6 = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final double n7 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            double n8 = 0.0;
            double n9 = 0.0;
            double n10 = kTexture.Modulo - 1;
            double n11 = kTexture.Height - 1;
            n = n * n6 + kRenderContext.ClipX1;
            n2 = n2 * n7 + kRenderContext.ClipY1;
            n3 = n3 * n6 + kRenderContext.ClipX1;
            n4 = n4 * n7 + kRenderContext.ClipY1;
            if (n > n3) {
                final double n12 = n3;
                n3 = n;
                n = n12;
                final double n13 = n10;
                n10 = n8;
                n8 = n13;
            }
            if (n2 > n4) {
                final double n14 = n4;
                n4 = n2;
                n2 = n14;
                final double n15 = n11;
                n11 = n9;
                n9 = n15;
            }
            if (n3 > kRenderContext.ClipX2) {
                n10 = (kRenderContext.ClipX2 - n) * (n10 - n8) / (n3 - n);
                n3 = kRenderContext.ClipX2;
            }
            if (n < kRenderContext.ClipX1) {
                n8 = (kRenderContext.ClipX1 - n) * (n10 - n8) / (n3 - n);
                n = kRenderContext.ClipX1;
            }
            if (n4 > kRenderContext.ClipY2) {
                n11 = (kRenderContext.ClipY2 - n2) * (n11 - n9) / (n4 - n2);
                n4 = kRenderContext.ClipY2;
            }
            if (n2 < kRenderContext.ClipY1) {
                n9 = (kRenderContext.ClipY1 - n2) * (n11 - n9) / (n4 - n2);
                n2 = kRenderContext.ClipY1;
            }
            this.Rootrc.Pretable[0][0].u = (int)(n8 * 65536.0);
            this.Rootrc.Pretable[0][1].u = this.Rootrc.Pretable[0][0].u;
            this.Rootrc.Pretable[1][0].u = (int)(n10 * 65536.0);
            this.Rootrc.Pretable[1][1].u = this.Rootrc.Pretable[1][0].u;
            this.Rootrc.Pretable[0][0].v = (int)(n9 * 65536.0);
            this.Rootrc.Pretable[1][0].v = this.Rootrc.Pretable[0][0].v;
            this.Rootrc.Pretable[0][1].v = (int)(n11 * 65536.0);
            this.Rootrc.Pretable[1][1].v = this.Rootrc.Pretable[0][1].v;
            this.OneRec.SetSub(kRenderContext, (int)n, (int)n2, (int)n3, (int)n4);
            this.drawrecUVSA(kTexture, this.OneRec, this.Rootrc.Pretable[0][0], this.Rootrc.Pretable[1][0], this.Rootrc.Pretable[0][1], this.Rootrc.Pretable[1][1], (int)n5);
        }
        
        void colorclip(final helpfade helpfade, final kPixel kPixel, final double n) {
            kPixel.r = (int)helpfade.Frv1;
            if (n > helpfade.Frd2) {
                kPixel.r = (int)helpfade.Frv2;
            }
            else if (n > helpfade.Frd1) {
                kPixel.r = (int)(helpfade.Frv1 + (helpfade.Frv2 - helpfade.Frv1) * (n - helpfade.Frd1) / (helpfade.Frd2 - helpfade.Frd1));
            }
            kPixel.g = (int)helpfade.Fgv1;
            if (n > helpfade.Fgd2) {
                kPixel.g = (int)helpfade.Fgv2;
            }
            else if (n > helpfade.Fgd1) {
                kPixel.g = (int)(helpfade.Fgv1 + (helpfade.Fgv2 - helpfade.Fgv1) * (n - helpfade.Fgd1) / (helpfade.Fgd2 - helpfade.Fgd1));
            }
            kPixel.b = (int)helpfade.Fbv1;
            if (n > helpfade.Fbd2) {
                kPixel.b = (int)helpfade.Fbv2;
            }
            else if (n > helpfade.Fbd1) {
                kPixel.b = (int)(helpfade.Fbv1 + (helpfade.Fbv2 - helpfade.Fbv1) * (n - helpfade.Fbd1) / (helpfade.Fbd2 - helpfade.Fbd1));
            }
        }
        
        public void draw(final kTexture kTexture, final kRenderContext kRenderContext) {
            final int n = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final int n2 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            if (!kRenderContext.drawable) {
                return;
            }
            for (int i = 0; i < n2; i += 16) {
                int n3;
                if (i + 16 < n2) {
                    n3 = 16;
                }
                else {
                    n3 = n2 - i;
                }
                for (int j = 0; j < n; j += 16) {
                    int n4;
                    if (j + 16 < n) {
                        n4 = 16;
                    }
                    else {
                        n4 = n - j;
                    }
                    this.OneRec.SetSub(kRenderContext, j + kRenderContext.ClipX1, i + kRenderContext.ClipY1, j + n4 + kRenderContext.ClipX1, i + n3 + kRenderContext.ClipY1);
                    this.drawrecUVRGB(kTexture, this.OneRec, kRenderContext.Pretable[j >> 4][i >> 4], kRenderContext.Pretable[(j >> 4) + 1][i >> 4], kRenderContext.Pretable[j >> 4][(i >> 4) + 1], kRenderContext.Pretable[(j >> 4) + 1][(i >> 4) + 1]);
                }
            }
        }
        
        public void drawrecUV(final kTexture kTexture, final kRenderContext kRenderContext, final kPixel kPixel, final kPixel kPixel2, final kPixel kPixel3, final kPixel kPixel4) {
            final int n = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final int n2 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            final int modulo = kRenderContext.Modulo;
            int n3 = kRenderContext.ClipX1 + kRenderContext.ClipY1 * kRenderContext.Modulo;
            final int[] currentChunk = kRenderContext.RC_RootContext.CurrentChunk;
            final int[] pixels = kTexture.pixels;
            int u = kPixel.u;
            int v = kPixel.v;
            final int n4 = (kPixel3.u - u) / n2;
            final int n5 = (kPixel3.v - v) / n2;
            int u2 = kPixel2.u;
            int v2 = kPixel2.v;
            final int n6 = (kPixel4.u - u2) / n2;
            final int n7 = (kPixel4.v - v2) / n2;
            for (int i = 0; i < n2; ++i) {
                int n8 = n3;
                int n9 = u;
                int n10 = v;
                final int n11 = (u2 - n9) / n;
                final int n12 = (v2 - n10) / n;
                for (int j = 0; j < n; ++j) {
                    currentChunk[n8++] = pixels[(n10 >> 8 & 0xFF00) | (n9 >> 16 & 0xFF)];
                    n9 += n11;
                    n10 += n12;
                }
                u += n4;
                v += n5;
                u2 += n6;
                v2 += n7;
                n3 += modulo;
            }
        }
        
        public void drawrecUVL(final kTexture kTexture, final kRenderContext kRenderContext, final kPixel kPixel, final kPixel kPixel2, final kPixel kPixel3, final kPixel kPixel4) {
            final int n = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final int n2 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            final int modulo = kRenderContext.Modulo;
            int n3 = kRenderContext.ClipX1 + kRenderContext.ClipY1 * kRenderContext.Modulo;
            final int[] currentChunk = kRenderContext.RC_RootContext.CurrentChunk;
            final int[] pixels = kTexture.pixels;
            final int n4 = 0;
            final int n5 = 255;
            int u = kPixel.u;
            int v = kPixel.v;
            int r = kPixel.r;
            final int n6 = (kPixel3.u - u) / n2;
            final int n7 = (kPixel3.v - v) / n2;
            final int n8 = (kPixel3.r - r) / n2;
            int u2 = kPixel2.u;
            int v2 = kPixel2.v;
            int r2 = kPixel2.r;
            final int n9 = (kPixel4.u - u2) / n2;
            final int n10 = (kPixel4.v - v2) / n2;
            final int n11 = (kPixel4.r - r2) / n2;
            for (int i = 0; i < n2; ++i) {
                int n12 = n3;
                int n13 = u;
                int n14 = v;
                int n15 = r;
                final int n16 = (u2 - n13) / n;
                final int n17 = (v2 - n14) / n;
                final int n18 = (r2 - n15) / n;
                for (int j = 0; j < n; ++j) {
                    final int n19 = pixels[(n14 >> 8 & 0xFF00) | (n13 >> 16 & 0xFF)];
                    final int n20 = n15 >> 16;
                    int n21 = ((n19 & 0xFF0000) >> 16) + n20;
                    int n22 = ((n19 & 0xFF00) >> 8) + n20;
                    int n23 = (n19 & 0xFF) + n20;
                    if (n21 > n5) {
                        n21 = n5;
                    }
                    else if (n21 < n4) {
                        n21 = n4;
                    }
                    if (n22 > n5) {
                        n22 = n5;
                    }
                    else if (n22 < n4) {
                        n22 = n4;
                    }
                    if (n23 > n5) {
                        n23 = n5;
                    }
                    else if (n23 < n4) {
                        n23 = n4;
                    }
                    currentChunk[n12++] = (0xFF000000 | n21 << 16 | n22 << 8 | n23);
                    n13 += n16;
                    n14 += n17;
                    n15 += n18;
                }
                u += n6;
                v += n7;
                r += n8;
                u2 += n9;
                v2 += n10;
                r2 += n11;
                n3 += modulo;
            }
        }
        
        public void drawrecUVM(final kTexture kTexture, final kRenderContext kRenderContext, final kPixel kPixel, final kPixel kPixel2, final kPixel kPixel3, final kPixel kPixel4) {
            final int n = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final int n2 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            final int modulo = kRenderContext.Modulo;
            int n3 = kRenderContext.ClipX1 + kRenderContext.ClipY1 * kRenderContext.Modulo;
            final int[] currentChunk = kRenderContext.RC_RootContext.CurrentChunk;
            final int[] pixels = kTexture.pixels;
            final int n4 = 255;
            final int modulo2 = kTexture.Modulo;
            if (n == 0) {
                return;
            }
            if (n2 == 0) {
                return;
            }
            int u = kPixel.u;
            int v = kPixel.v;
            final int n5 = (kPixel3.u - u) / n2;
            final int n6 = (kPixel3.v - v) / n2;
            int u2 = kPixel2.u;
            int v2 = kPixel2.v;
            final int n7 = (kPixel4.u - u2) / n2;
            final int n8 = (kPixel4.v - v2) / n2;
            for (int i = 0; i < n2; ++i) {
                int n9 = n3;
                int n10 = u;
                int n11 = v;
                final int n12 = (u2 - n10) / n;
                final int n13 = (v2 - n11) / n;
                for (int j = 0; j < n; ++j) {
                    final int n14 = pixels[(n11 >> 16) * modulo2 + (n10 >> 16)];
                    final int n15 = currentChunk[n9];
                    int n16 = ((n14 & 0xFF0000) >> 16) + ((n15 & 0xFF0000) >> 16);
                    int n17 = ((n14 & 0xFF00) >> 8) + ((n15 & 0xFF00) >> 8);
                    int n18 = (n14 & 0xFF) + (n15 & 0xFF);
                    if (n16 > n4) {
                        n16 = n4;
                    }
                    if (n17 > n4) {
                        n17 = n4;
                    }
                    if (n18 > n4) {
                        n18 = n4;
                    }
                    currentChunk[n9++] = (0xFF000000 | n16 << 16 | n17 << 8 | n18);
                    n10 += n12;
                    n11 += n13;
                }
                u += n5;
                v += n6;
                u2 += n7;
                v2 += n8;
                n3 += modulo;
            }
        }
        
        public void drawrecUVRGB(final kTexture kTexture, final kRenderContext kRenderContext, final kPixel kPixel, final kPixel kPixel2, final kPixel kPixel3, final kPixel kPixel4) {
            final int n = 0;
            final int n2 = 255;
            final int n3 = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final int n4 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            final int modulo = kRenderContext.Modulo;
            int n5 = kRenderContext.ClipX1 + kRenderContext.ClipY1 * kRenderContext.Modulo;
            final int[] currentChunk = kRenderContext.RC_RootContext.CurrentChunk;
            final int[] pixels = kTexture.pixels;
            int u = kPixel.u;
            int v = kPixel.v;
            int r = kPixel.r;
            int g = kPixel.g;
            int b = kPixel.b;
            final int n6 = (kPixel3.u - u) / n4;
            final int n7 = (kPixel3.v - v) / n4;
            final int n8 = (kPixel3.r - r) / n4;
            final int n9 = (kPixel3.g - g) / n4;
            final int n10 = (kPixel3.b - b) / n4;
            int u2 = kPixel2.u;
            int v2 = kPixel2.v;
            int r2 = kPixel2.r;
            int g2 = kPixel2.g;
            int b2 = kPixel2.b;
            final int n11 = (kPixel4.u - u2) / n4;
            final int n12 = (kPixel4.v - v2) / n4;
            final int n13 = (kPixel4.r - r2) / n4;
            final int n14 = (kPixel4.g - g2) / n4;
            final int n15 = (kPixel4.b - b2) / n4;
            for (int i = 0; i < n4; ++i) {
                int n16 = n5;
                int n17 = u;
                int n18 = v;
                int n19 = r;
                int n20 = g;
                int n21 = b;
                final int n22 = (u2 - n17) / n3;
                final int n23 = (v2 - n18) / n3;
                final int n24 = (r2 - n19) / n3;
                final int n25 = (g2 - n20) / n3;
                final int n26 = (b2 - n21) / n3;
                for (int j = 0; j < n3; ++j) {
                    final int n27 = pixels[(n18 >> 8 & 0xFF00) | (n17 >> 16 & 0xFF)];
                    final int n28 = (n19 & 0xFFFF0000) >> 16;
                    final int n29 = (n20 & 0xFFFF0000) >> 16;
                    final int n30 = (n21 & 0xFFFF0000) >> 16;
                    int n31 = ((n27 & 0xFF0000) >> 16) + n28;
                    int n32 = ((n27 & 0xFF00) >> 8) + n29;
                    int n33 = (n27 & 0xFF) + n30;
                    if (n31 > n2) {
                        n31 = n2;
                    }
                    else if (n31 < n) {
                        n31 = n;
                    }
                    if (n32 > n2) {
                        n32 = n2;
                    }
                    else if (n32 < n) {
                        n32 = n;
                    }
                    if (n33 > n2) {
                        n33 = n2;
                    }
                    else if (n33 < n) {
                        n33 = n;
                    }
                    currentChunk[n16++] = (0xFF000000 | n31 << 16 | n32 << 8 | n33);
                    n17 += n22;
                    n18 += n23;
                    n19 += n24;
                    n20 += n25;
                    n21 += n26;
                }
                u += n6;
                v += n7;
                r += n8;
                g += n9;
                b += n10;
                u2 += n11;
                v2 += n12;
                r2 += n13;
                g2 += n14;
                b2 += n15;
                n5 += modulo;
            }
        }
        
        public void drawrecUVS(final kTexture kTexture, final kRenderContext kRenderContext, final kPixel kPixel, final kPixel kPixel2, final kPixel kPixel3, final kPixel kPixel4) {
            final int n = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final int n2 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            final int modulo = kRenderContext.Modulo;
            int n3 = kRenderContext.ClipX1 + kRenderContext.ClipY1 * kRenderContext.Modulo;
            final int[] currentChunk = kRenderContext.RC_RootContext.CurrentChunk;
            final int[] pixels = kTexture.pixels;
            final int modulo2 = kTexture.Modulo;
            if (n == 0) {
                return;
            }
            if (n2 == 0) {
                return;
            }
            int u = kPixel.u;
            int v = kPixel.v;
            final int n4 = (kPixel3.u - u) / n2;
            final int n5 = (kPixel3.v - v) / n2;
            int u2 = kPixel2.u;
            int v2 = kPixel2.v;
            final int n6 = (kPixel4.u - u2) / n2;
            final int n7 = (kPixel4.v - v2) / n2;
            for (int i = 0; i < n2; ++i) {
                int n8 = n3;
                int n9 = u;
                int n10 = v;
                final int n11 = (u2 - n9) / n;
                final int n12 = (v2 - n10) / n;
                for (int j = 0; j < n; ++j) {
                    final int n13 = pixels[(n10 >> 16) * modulo2 + (n9 >> 16)];
                    if (n13 != -1) {
                        currentChunk[n8] = n13;
                    }
                    ++n8;
                    n9 += n11;
                    n10 += n12;
                }
                u += n4;
                v += n5;
                u2 += n6;
                v2 += n7;
                n3 += modulo;
            }
        }
        
        public void drawrecUVSA(final kTexture kTexture, final kRenderContext kRenderContext, final kPixel kPixel, final kPixel kPixel2, final kPixel kPixel3, final kPixel kPixel4, final int n) {
            if (n <= 0) {
                return;
            }
            if (n >= 255) {
                this.drawrecUVS(kTexture, kRenderContext, kPixel, kPixel2, kPixel3, kPixel4);
                return;
            }
            final int n2 = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final int n3 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            final int modulo = kRenderContext.Modulo;
            int n4 = kRenderContext.ClipX1 + kRenderContext.ClipY1 * kRenderContext.Modulo;
            final int[] currentChunk = kRenderContext.RC_RootContext.CurrentChunk;
            final int[] pixels = kTexture.pixels;
            final int modulo2 = kTexture.Modulo;
            final int n5 = 255 - n;
            if (n2 == 0) {
                return;
            }
            if (n3 == 0) {
                return;
            }
            int u = kPixel.u;
            int v = kPixel.v;
            final int n6 = (kPixel3.u - u) / n3;
            final int n7 = (kPixel3.v - v) / n3;
            int u2 = kPixel2.u;
            int v2 = kPixel2.v;
            final int n8 = (kPixel4.u - u2) / n3;
            final int n9 = (kPixel4.v - v2) / n3;
            for (int i = 0; i < n3; ++i) {
                int n10 = n4;
                int n11 = u;
                int n12 = v;
                final int n13 = (u2 - n11) / n2;
                final int n14 = (v2 - n12) / n2;
                for (int j = 0; j < n2; ++j) {
                    final int n15 = pixels[(n12 >> 16) * modulo2 + (n11 >> 16)];
                    if (n15 != -1) {
                        final int n16 = currentChunk[n10];
                        currentChunk[n10] = (0xFF000000 | ((n16 & 0xFF0000) >> 16) * n5 + ((n15 & 0xFF0000) >> 16) * n >> 8 << 16 | ((n16 & 0xFF00) >> 8) * n5 + ((n15 & 0xFF00) >> 8) * n >> 8 << 8 | (n16 & 0xFF) * n5 + (n15 & 0xFF) * n >> 8);
                    }
                    ++n10;
                    n11 += n13;
                    n12 += n14;
                }
                u += n6;
                v += n7;
                u2 += n8;
                v2 += n9;
                n4 += modulo;
            }
        }
        
        public void findfloor(double n, double n2, double n3, double n4, final kRenderContext kRenderContext, final helpfade helpfade) {
            if (!kRenderContext.drawable) {
                return;
            }
            final double n5 = 65536.0;
            helpfade.Frv1 *= n5;
            helpfade.Frv2 *= n5;
            helpfade.Fgv1 *= n5;
            helpfade.Fgv2 *= n5;
            helpfade.Fbv1 *= n5;
            helpfade.Fbv2 *= n5;
            n3 *= 65536.0;
            n4 *= 65536.0;
            n += kRenderContext.PosY;
            n2 -= kRenderContext.PosY;
            for (int i = 0; i < kRenderContext.MaxPreY; ++i) {
                for (int j = 0; j < kRenderContext.MaxPreX; ++j) {
                    final kPixel kPixel = kRenderContext.Pretable[j][i];
                    final double xf = kPixel.XF;
                    double yf = kPixel.YF;
                    final double zf = kPixel.ZF;
                    if (yf == 0.0) {
                        yf = 1.0E-8;
                    }
                    final double n6 = 1.0 / yf;
                    if (yf < 0.0) {
                        final double n7 = zf * n * n6;
                        final double n8 = xf * n * n6;
                        kPixel.u = (int)((n7 - kRenderContext.PosZ) * n3);
                        kPixel.v = (int)((n8 - kRenderContext.PosX) * n4);
                        this.colorclip(helpfade, kPixel, Math.sqrt(n7 * n7 + n8 * n8 + n * n));
                    }
                    else {
                        final double n9 = zf * n2 * n6;
                        final double n10 = xf * n2 * n6;
                        kPixel.u = (int)((-n9 - kRenderContext.PosZ) * n3);
                        kPixel.v = (int)((-n10 - kRenderContext.PosX) * n4);
                        this.colorclip(helpfade, kPixel, Math.sqrt(n9 * n9 + n10 * n10 + n2 * n2));
                    }
                }
            }
        }
        
        private void findtunnel(final double n, double n2, final kRenderContext kRenderContext, final helpfade helpfade) {
            if (!kRenderContext.drawable) {
                return;
            }
            final double n3 = 65536.0;
            helpfade.Frv1 *= n3;
            helpfade.Frv2 *= n3;
            helpfade.Fgv1 *= n3;
            helpfade.Fgv2 *= n3;
            helpfade.Fbv1 *= n3;
            helpfade.Fbv2 *= n3;
            n2 *= 65536.0;
            final double n4 = 5340353.715440872;
            for (int i = 0; i < kRenderContext.MaxPreY; ++i) {
                for (int j = 0; j < kRenderContext.MaxPreX; ++j) {
                    final kPixel kPixel = kRenderContext.Pretable[j][i];
                    final double xf = kPixel.XF;
                    final double abs = Math.abs(kPixel.YF);
                    final double zf = kPixel.ZF;
                    double sqrt = Math.sqrt(xf * xf + abs * abs);
                    if (sqrt == 0.0) {
                        sqrt = 1.0E-7;
                    }
                    final double atan2 = Math.atan2(xf, abs);
                    final double n5 = Math.cos(atan2) * n;
                    final double n6 = Math.sin(atan2) * n;
                    final double n7 = zf * n / sqrt;
                    kPixel.u = (int)((n7 + kRenderContext.PosZ) * n2);
                    kPixel.v = (int)(atan2 * n4) + 8388608;
                    this.colorclip(helpfade, kPixel, Math.sqrt(n7 * n7 + n5 * n5 + n6 * n6));
                }
            }
        }
        
        private void findtwirl(final double n, final double n2, final double n3, final double n4, final double n5, final double n6, final kRenderContext kRenderContext) {
            if (!kRenderContext.drawable) {
                return;
            }
            final int n7 = (int)(kRenderContext.PosZ * 65536.0) & 0xFFFFFF;
            for (int i = 0; i < kRenderContext.MaxPreY; ++i) {
                for (int j = 0; j < kRenderContext.MaxPreX; ++j) {
                    final kPixel kPixel = kRenderContext.Pretable[j][i];
                    final double n8 = kPixel.XF * 256.0;
                    final double x = kPixel.YF * 256.0;
                    final double atan2 = Math.atan2(n8, x);
                    final double sqrt = Math.sqrt(n8 * n8 + x * x);
                    final double n9 = atan2 + n3 * Math.sin((n + sqrt * 3.141592653589793 / 128.0) * n2);
                    final double n10 = sqrt + n6 * Math.sin((n4 + sqrt * 3.141592653589793 / 128.0) * n5);
                    kPixel.u = (int)(Math.sin(n9) * n10 * 65536.0 + kRenderContext.PosX * 65536.0);
                    kPixel.v = (int)(Math.cos(n9) * n10 * 65536.0 + kRenderContext.PosY * 65536.0);
                    final double x2 = x - 8.0;
                    final double atan3 = Math.atan2(n8, x2);
                    final double sqrt2 = Math.sqrt(n8 * n8 + x2 * x2);
                    final int n11 = kPixel.u - (int)(Math.sin(atan3 + n3 * Math.sin((n + sqrt2 * 3.141592653589793 / 128.0) * n2)) * (sqrt2 + n6 * Math.sin((n4 + sqrt2 * 3.141592653589793 / 128.0) * n5)) * 65536.0 + kRenderContext.PosX * 65536.0);
                    kPixel.r = n11 * 26;
                    kPixel.g = n11 * 24;
                    kPixel.b = n11 * 23;
                }
            }
        }
        
        public void flare3D(final double n, final double n2, final double n3, final double n4, final double n5, final double n6, final kRenderContext kRenderContext, final kTexture kTexture) {
            if (!kRenderContext.drawable) {
                return;
            }
            final double n7 = kRenderContext.ClipX2 - kRenderContext.ClipX1;
            final double n8 = kRenderContext.ClipY2 - kRenderContext.ClipY1;
            final double n9 = n6 / n3;
            if (n3 <= 0.01) {
                return;
            }
            final double n10 = kTexture.Modulo / n7;
            final double n11 = kTexture.Height / n8;
            final double n12 = n * n9;
            final double n13 = n2 * n9;
            double n14 = 0.0;
            double n15 = 0.0;
            double n16 = kTexture.Modulo - 1;
            double n17 = kTexture.Height - 1;
            final double n18 = n10 * 0.5 * n4 * n9;
            final double n19 = n11 * 0.5 * n5 * n9;
            final double n20 = n12 - n18;
            final double n21 = n13 - n19;
            final double n22 = n12 + n18;
            final double n23 = n13 + n19;
            double n24 = n20 * n7 + kRenderContext.ClipX1 + n7 * 0.5;
            double n25 = n21 * n8 + kRenderContext.ClipY1 + n8 * 0.5;
            double n26 = n22 * n7 + kRenderContext.ClipX1 + n7 * 0.5;
            double n27 = n23 * n8 + kRenderContext.ClipY1 + n8 * 0.5;
            if (n24 > n26) {
                final double n28 = n26;
                n26 = n24;
                n24 = n28;
                final double n29 = n16;
                n16 = n14;
                n14 = n29;
            }
            if (n25 > n27) {
                final double n30 = n27;
                n27 = n25;
                n25 = n30;
                final double n31 = n17;
                n17 = n15;
                n15 = n31;
            }
            if (n26 > kRenderContext.ClipX2) {
                n16 = (kRenderContext.ClipX2 - n24) * (n16 - n14) / (n26 - n24);
                n26 = kRenderContext.ClipX2;
            }
            if (n24 < kRenderContext.ClipX1) {
                n14 = (kRenderContext.ClipX1 - n24) * (n16 - n14) / (n26 - n24);
                n24 = kRenderContext.ClipX1;
            }
            if (n27 > kRenderContext.ClipY2) {
                n17 = (kRenderContext.ClipY2 - n25) * (n17 - n15) / (n27 - n25);
                n27 = kRenderContext.ClipY2;
            }
            if (n25 < kRenderContext.ClipY1) {
                n15 = (kRenderContext.ClipY1 - n25) * (n17 - n15) / (n27 - n25);
                n25 = kRenderContext.ClipY1;
            }
            this.Rootrc.Pretable[0][0].u = (int)(n14 * 65536.0);
            this.Rootrc.Pretable[0][1].u = this.Rootrc.Pretable[0][0].u;
            this.Rootrc.Pretable[1][0].u = (int)(n16 * 65536.0);
            this.Rootrc.Pretable[1][1].u = this.Rootrc.Pretable[1][0].u;
            this.Rootrc.Pretable[0][0].v = (int)(n15 * 65536.0);
            this.Rootrc.Pretable[1][0].v = this.Rootrc.Pretable[0][0].v;
            this.Rootrc.Pretable[0][1].v = (int)(n17 * 65536.0);
            this.Rootrc.Pretable[1][1].v = this.Rootrc.Pretable[0][1].v;
            this.OneRec.SetSub(kRenderContext, (int)n24, (int)n25, (int)n26, (int)n27);
            this.drawrecUVM(kTexture, this.OneRec, this.Rootrc.Pretable[0][0], this.Rootrc.Pretable[1][0], this.Rootrc.Pretable[0][1], this.Rootrc.Pretable[1][1]);
        }
    }
    
    private class k2Dpos
    {
        double Angle;
        double Zoom;
        double ZoomRun;
        double Utexture;
        double Vtexture;
        double Xcentre;
        double Ycentre;
        
        public k2Dpos() {
            this.Zoom = 1.0;
        }
    }
    
    private class kParameterXIMAGE extends kParameterX
    {
        kParameterXIMAGE() {
        }
        
        public kRenderContext RecOut() {
            return null;
        }
        
        public kTexture TextureOut() {
            return super.kTXT;
        }
        
        public double doubleout(final double n) {
            return 0.0;
        }
    }
    
    private class kParameterXRC extends kParameterX
    {
        kParameterXRC() {
        }
        
        public kRenderContext RecOut() {
            if (super.kREC == null) {
                return kraycasting.this.RenderContext;
            }
            return super.kREC;
        }
        
        public kTexture TextureOut() {
            return null;
        }
        
        public double doubleout(final double n) {
            return 0.0;
        }
    }
    
    public class kSound
    {
        private AudioClip OurAudio;
        private boolean soundIsOn;
        
        public kSound(final String name) {
            this.OurAudio = kraycasting.this.getAudioClip(kraycasting.this.getDocumentBase(), name);
            this.soundIsOn = false;
        }
        
        public void LoopOne() {
            if (this.OurAudio != null) {
                this.OurAudio.loop();
                this.soundIsOn = true;
            }
        }
        
        public void PlayOne() {
            if (this.OurAudio != null) {
                this.OurAudio.play();
                this.soundIsOn = false;
            }
        }
        
        public void StopOne() {
            if (this.OurAudio != null) {
                this.OurAudio.stop();
                this.soundIsOn = false;
            }
        }
    }
    
    private class rotozoom
    {
        rotozoom() {
        }
        
        public void draw(final kTexture kTexture, final kRenderContext kRenderContext, final k2Dpos k2Dpos) {
            final int clipX2 = kRenderContext.ClipX2;
            final int clipY2 = kRenderContext.ClipY2;
            final int[] currentChunk = kraycasting.kRenderContext.access$0(kRenderContext).CurrentChunk;
            final int[] pixels = kTexture.pixels;
            int n = 0;
            final double cos = Math.cos(k2Dpos.Angle);
            final double sin = Math.sin(k2Dpos.Angle);
            final double n2 = k2Dpos.Zoom * 65536.0;
            final double n3 = cos * n2;
            final double n4 = sin * n2;
            final int n5 = (int)(-n3);
            final int n6 = (int)(-n4);
            int n7 = (int)(-n4 * k2Dpos.Xcentre + n3 * k2Dpos.Ycentre + k2Dpos.Utexture * 65536.0);
            int n8 = (int)(n3 * k2Dpos.Xcentre + n4 * k2Dpos.Ycentre + k2Dpos.Vtexture * 65536.0);
            final int n9 = (int)n4;
            final int n10 = (int)(-n3);
            for (int i = 0; i < clipY2; ++i) {
                int n11 = n7;
                int n12 = n8;
                for (int j = 0; j < clipX2; ++j) {
                    currentChunk[n++] = pixels[(n11 >> 8 & 0xFF00) | (n12 >> 16 & 0xFF)];
                    n11 += n9;
                    n12 += n10;
                }
                n7 += n5;
                n8 += n6;
            }
        }
    }
}
