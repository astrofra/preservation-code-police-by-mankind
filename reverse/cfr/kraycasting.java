/*
 * Decompiled with CFR 0.152.
 */
import java.applet.Applet;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.image.MemoryImageSource;
import java.awt.image.PixelGrabber;
import java.util.Hashtable;

public class kraycasting
extends Applet
implements Runnable {
    private kScript THEkScript;
    private kRenderContext RenderContext;
    private raycasting rcs;
    private Image applet_image;
    private Thread mainthread = null;
    private boolean boolend = false;
    private kTimer kTime;
    private byte[] ct;
    private Hashtable partcHash;
    private Hashtable ImageHash;
    private Hashtable RectaHash;
    private Hashtable SoundHash;
    private Hashtable TableHash;
    private Hashtable CamerHash;

    int FindHowMany(String string, String string2) {
        int n = 1;
        int n2 = string.length();
        int n3 = 0;
        int n4 = 0;
        while (n3 != -1 && n4 < n2) {
            n3 = string.indexOf(string2, n4);
            if (n3 == -1) continue;
            ++n;
            n4 = n3 + 1;
        }
        return n;
    }

    String FindInto(String string, int n, String string2) {
        int n2 = 2;
        int n3 = 1;
        int n4 = string.length();
        int n5 = 0;
        int n6 = 0;
        while (n5 != -1 && n6 < n4) {
            n5 = string.indexOf(string2, n6);
            if (n5 == -1) continue;
            ++n2;
            n6 = n5 + 1;
        }
        int[] nArray = new int[n2];
        nArray[0] = -1;
        nArray[n2 - 1] = n4;
        n6 = 0;
        n5 = 0;
        while (n5 != -1 && n6 < n4) {
            n5 = string.indexOf(string2, n6);
            if (n5 == -1) continue;
            nArray[n3] = n5;
            ++n3;
            n6 = n5 + 1;
        }
        return string.substring(nArray[n] + 1, nArray[n + 1]);
    }

    String[] SplitSTag(String string, String string2) {
        String string3 = "<" + string2 + ">";
        String string4 = "</" + string2 + ">";
        int n = 0;
        int n2 = string.length();
        int n3 = 0;
        int n4 = 0;
        int n5 = 0;
        while (n5 != -1 && n4 < n2) {
            n5 = string.indexOf(string3, n4);
            if (n5 == -1) continue;
            ++n3;
            n4 = n5 + string3.length();
        }
        if (n3 == 0) {
            return null;
        }
        String[] stringArray = new String[n3];
        n3 = 0;
        n4 = 0;
        n5 = 0;
        while (n5 != -1 && n4 < n2) {
            n5 = string.indexOf(string3, n4);
            if (n5 == -1) continue;
            n4 = n5 + string3.length();
            n = string.indexOf(string4, n4);
            stringArray[n3] = string.substring(n4, n);
            ++n3;
        }
        return stringArray;
    }

    static /* synthetic */ boolean access$16(kraycasting kraycasting2) {
        return kraycasting2.boolend;
    }

    public void init() {
        Integer n;
        int n2;
        this.RenderContext = new kRenderContext(this);
        this.applet_image = this.RenderContext.createImage();
        this.THEkScript = new kScript(this.getParameter("SCRIPT"));
        this.rcs = new raycasting(this.RenderContext);
        this.ct = new byte[8192];
        int n3 = 0;
        while (n3 < 64) {
            n2 = 0;
            while (n2 < 64) {
                n = new Integer(n2 * (n3 + 1) >> 6);
                this.ct[n3 << 6 | n2] = n.byteValue();
                ++n2;
            }
            ++n3;
        }
        n3 = 64;
        while (n3 < 128) {
            n2 = 0;
            while (n2 < 64) {
                n = new Integer(n2 + ((63 - n2) * (n3 - 64) >> 6));
                this.ct[n3 << 6 | n2] = n.byteValue();
                ++n2;
            }
            ++n3;
        }
        this.kTime = new kTimer();
    }

    public void run() {
        while (!this.boolend) {
            int n = this.kTime.getTime();
            kPartX kPartX2 = this.THEkScript.part(n);
            kPartX2.rendah(n);
            this.applet_image = this.RenderContext.createImage();
            this.repaint();
            try {
                Thread.sleep(20L);
            }
            catch (InterruptedException interruptedException) {}
        }
    }

    public void start() {
        this.mainthread = new Thread(this);
        this.mainthread.start();
    }

    public void stop() {
        kSound.StopAll();
        this.boolend = true;
    }

    public void update(Graphics graphics) {
        graphics.drawImage(this.applet_image, 0, 0, this);
    }

    public class kTexture {
        int Modulo;
        int Height;
        int[] pixels;

        public kTexture(Image image) {
            try {
                while ((this.Modulo = image.getWidth(null)) < 0) {
                }
                this.Height = image.getHeight(null);
                this.pixels = new int[this.Modulo * this.Height];
                PixelGrabber pixelGrabber = new PixelGrabber(image, 0, 0, this.Modulo, this.Height, this.pixels, 0, this.Modulo);
                pixelGrabber.grabPixels();
            }
            catch (InterruptedException interruptedException) {}
        }
    }

    private class kRenderContext {
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

        public kRenderContext(Applet applet) {
            int n = 0;
            int n2 = 0;
            int n3 = 0;
            this.app = applet;
            this.ClipX1 = 0;
            this.ClipY1 = 0;
            this.ClipX2 = applet.getSize().width;
            this.ClipY2 = applet.getSize().height;
            int n4 = this.ClipX2 - this.ClipX1;
            int n5 = this.ClipY2 - this.ClipY1;
            this.drawable = true;
            double d = 1.0 / (double)n4;
            double d2 = 1.0 / (double)n5;
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
            int n6 = n4 >> 1;
            int n7 = n5 >> 1;
            n2 = 16;
            int n8 = 0;
            int n9 = 0;
            while (n9 < n5 + n2) {
                if (n9 + 16 > n5) {
                    n2 = n5 - n9;
                }
                n3 = 16;
                n = 0;
                int n10 = 0;
                while (n10 < n4 + n3) {
                    if (n10 + 16 > n4) {
                        n3 = n4 - n10;
                    }
                    this.Pretable[n][n8] = new kPixel();
                    this.Pretable[n][n8].XI = (double)(n10 - n6) * d;
                    this.Pretable[n][n8].YI = (double)(n9 - n7) * d2;
                    ++n;
                    n10 += n3;
                }
                ++n8;
                n9 += n2;
            }
            this.MaxPreX = n;
            this.MaxPreY = n8;
        }

        public kRenderContext(kRenderContext kRenderContext2) {
            this.ClipX1 = kRenderContext2.ClipX1;
            this.ClipY1 = kRenderContext2.ClipY1;
            this.ClipX2 = kRenderContext2.ClipX2;
            this.ClipY2 = kRenderContext2.ClipY2;
            this.Modulo = kRenderContext2.Modulo;
            this.Height = kRenderContext2.Height;
            this.app = kRenderContext2.app;
            this.RC_RootContext = kRenderContext2;
            this.RC_FatherContext = kRenderContext2;
            this.drawable = kRenderContext2.drawable;
        }

        public kRenderContext(kRenderContext kRenderContext2, double d, double d2, double d3, double d4) {
            this.RC_RootContext = kRenderContext2.RC_RootContext;
            this.Pretable = new kPixel[this.RC_RootContext.MaxPreX][this.RC_RootContext.MaxPreY];
            this.SetRect(kRenderContext2, d, d2, d3, d4);
        }

        public void SetRect(kRenderContext kRenderContext2, double d, double d2, double d3, double d4) {
            int n = 0;
            int n2 = 0;
            int n3 = 0;
            double d5 = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            double d6 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            this.drawable = true;
            if (d < 0.0) {
                d = 0.0;
            }
            if (d2 < 0.0) {
                d2 = 0.0;
            }
            if (d3 > 1.0) {
                d3 = 1.0;
            }
            if (d4 > 1.0) {
                d4 = 1.0;
            }
            this.ClipX1 = kRenderContext2.ClipX1 + (int)(d5 * d);
            this.ClipY1 = kRenderContext2.ClipY1 + (int)(d6 * d2);
            this.ClipX2 = kRenderContext2.ClipX1 + (int)(d5 * d3);
            this.ClipY2 = kRenderContext2.ClipY1 + (int)(d6 * d4);
            this.Modulo = kRenderContext2.Modulo;
            this.Height = kRenderContext2.Height;
            this.app = kRenderContext2.app;
            int n4 = this.ClipX2 - this.ClipX1;
            int n5 = this.ClipY2 - this.ClipY1;
            double d7 = 1.0 / (double)n4;
            double d8 = 1.0 / (double)n5;
            if (n4 <= 0 || n5 <= 0 || this.ClipX1 < kRenderContext2.ClipX1 || this.ClipY1 < kRenderContext2.ClipY1 || this.ClipX2 > kRenderContext2.ClipX2 || this.ClipY2 > kRenderContext2.ClipY2) {
                this.drawable = false;
                return;
            }
            this.MaxX = n4;
            this.MaxY = n5;
            this.MaxPreX = (this.MaxX >> 4) + 2;
            this.MaxPreY = (this.MaxY >> 4) + 2;
            int n6 = n4 >> 1;
            int n7 = n5 >> 1;
            n2 = 16;
            int n8 = 0;
            int n9 = 0;
            while (n9 < n5 + n2) {
                if (n9 + 16 > n5) {
                    n2 = n5 - n9;
                }
                n3 = 16;
                n = 0;
                int n10 = 0;
                while (n10 < n4 + n3) {
                    if (n10 + 16 > n4) {
                        n3 = n4 - n10;
                    }
                    this.Pretable[n][n8] = new kPixel();
                    this.Pretable[n][n8].XI = (double)(n10 - n6) * d7;
                    this.Pretable[n][n8].YI = (double)(n9 - n7) * d8;
                    ++n;
                    n10 += n3;
                }
                ++n8;
                n9 += n2;
            }
            this.MaxPreX = n;
            this.MaxPreY = n8;
        }

        public void SetSub(kRenderContext kRenderContext2, int n, int n2, int n3, int n4) {
            this.ClipX1 = n;
            this.ClipY1 = n2;
            this.ClipX2 = n3;
            this.ClipY2 = n4;
            this.CurrentChunk = kRenderContext2.CurrentChunk;
            this.RC_RootContext = kRenderContext2.RC_RootContext;
        }

        public Image createImage() {
            MemoryImageSource memoryImageSource = this.RC_imageMIS2;
            this.RC_imageMIS2 = this.RC_imageMIS1;
            this.RC_imageMIS1 = memoryImageSource;
            int[] nArray = this.CurrentChunk;
            this.CurrentChunk = this.Chunk2;
            this.Chunk2 = nArray;
            return this.app.createImage(this.RC_imageMIS2);
        }
    }

    public class kcamera {
        public double X;
        public double Y;
        public double Z;
        public double O1;
        public double O2;
        public double O3;
        public double FOV;

        public kcamera(double d, double d2, double d3, double d4, double d5, double d6, double d7) {
            this.X = d;
            this.Y = d2;
            this.Z = d3;
            this.FOV = d7;
            this.O1 = d4;
            this.O2 = d5;
            this.O3 = d6;
        }

        public void set(double d, double d2, double d3, double d4, double d5, double d6, double d7) {
            this.X = d;
            this.Y = d2;
            this.Z = d3;
            this.FOV = d7;
            this.O1 = d4;
            this.O2 = d5;
            this.O3 = d6;
        }

        public void target(double d, double d2, double d3, double d4, double d5, double d6, double d7, double d8) {
            this.X = d;
            this.Y = d2;
            this.Z = d3;
            this.FOV = d7;
            this.O1 = d8;
            d4 -= d;
            d5 -= d2;
            if ((d6 -= d3) == 0.0) {
                d6 = 1.0E-6;
            }
            double d9 = d4 / d6;
            d9 = Math.atan(d9);
            if (d6 > 0.0) {
                d9 += Math.PI;
            }
            this.O3 = d9 += Math.PI;
            double d10 = d6 * Math.cos(d9) + d4 * Math.sin(d9);
            if (d10 == 0.0) {
                d10 = 1.0E-6;
            }
            d9 = d5 / d10;
            d9 = Math.atan(d9);
            if (d10 > 0.0) {
                d9 += Math.PI;
            }
            this.O2 = -(d9 += Math.PI);
        }
    }

    private class kparticle {
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

        public kparticle(int n, double d, double d2, double d3, double d4, double d5, double d6, double d7, double d8) {
            if (n == 0) {
                n = 1;
            }
            this.nbparticle = n;
            this.posx = new double[n];
            this.posy = new double[n];
            this.posz = new double[n];
            this.Vx = new double[n];
            this.Vy = new double[n];
            this.Vz = new double[n];
            this.Ax = new double[n];
            this.Ay = new double[n];
            this.Az = new double[n];
            this.force = new double[n];
            this.reset(d, d2, d3, d4, d5, d6, d7, d8);
        }

        public void drawflare(kcamera kcamera2, kTexture kTexture2, kRenderContext kRenderContext2, double d, double d2) {
            double d3 = Math.cos(kcamera2.O1);
            double d4 = Math.sin(kcamera2.O1);
            double d5 = Math.cos(kcamera2.O2);
            double d6 = Math.sin(kcamera2.O2);
            double d7 = Math.cos(kcamera2.O3);
            double d8 = Math.sin(kcamera2.O3);
            int n = 0;
            while (n < this.nbparticle) {
                double d9 = this.posx[n] - kcamera2.X;
                double d10 = this.posy[n] - kcamera2.Y;
                double d11 = this.posz[n] - kcamera2.Z;
                double d12 = -d8 * d11 + d7 * d9;
                double d13 = d10;
                double d14 = d7 * d11 + d8 * d9;
                double d15 = d12;
                double d16 = d5 * d13 + d6 * d14;
                double d17 = -d6 * d13 + d5 * d14;
                d9 = d3 * d15 + d4 * d16;
                d10 = -d4 * d15 + d3 * d16;
                d11 = d17;
                kraycasting.this.rcs.flare3D(d9, d10, d11, d, d2, kcamera2.FOV, kRenderContext2, kTexture2);
                ++n;
            }
        }

        public void drawsprit(kcamera kcamera2, kTexture kTexture2, kRenderContext kRenderContext2, double d, double d2) {
            double d3 = Math.cos(kcamera2.O1);
            double d4 = Math.sin(kcamera2.O1);
            double d5 = Math.cos(kcamera2.O2);
            double d6 = Math.sin(kcamera2.O2);
            double d7 = Math.cos(kcamera2.O3);
            double d8 = Math.sin(kcamera2.O3);
            int n = 0;
            while (n < this.nbparticle) {
                double d9 = this.posx[n] - kcamera2.X;
                double d10 = this.posy[n] - kcamera2.Y;
                double d11 = this.posz[n] - kcamera2.Z;
                double d12 = -d8 * d11 + d7 * d9;
                double d13 = d10;
                double d14 = d7 * d11 + d8 * d9;
                double d15 = d12;
                double d16 = d5 * d13 + d6 * d14;
                double d17 = -d6 * d13 + d5 * d14;
                d9 = d3 * d15 + d4 * d16;
                d10 = -d4 * d15 + d3 * d16;
                d11 = d17;
                kraycasting.this.rcs.Setzoom3D(d9, d10, d11, d, d2, kcamera2.FOV, kRenderContext2, kTexture2, 256.0);
                ++n;
            }
        }

        public void move() {
            double d = 0.0;
            double d2 = 0.0;
            double d3 = 0.0;
            double d4 = 1.0 / (double)this.nbparticle;
            int n = 0;
            while (n < this.nbparticle) {
                int n2 = n;
                this.Vx[n2] = this.Vx[n2] + this.Ax[n];
                int n3 = n;
                this.Vy[n3] = this.Vy[n3] + this.Ay[n];
                int n4 = n;
                this.Vz[n4] = this.Vz[n4] + this.Az[n];
                int n5 = n;
                this.posx[n5] = this.posx[n5] + this.Vx[n];
                int n6 = n;
                this.posy[n6] = this.posy[n6] + this.Vy[n];
                int n7 = n;
                this.posz[n7] = this.posz[n7] + this.Vz[n];
                d += this.posx[n];
                d2 += this.posy[n];
                d3 += this.posz[n];
                ++n;
            }
            this.aveX = d * d4;
            this.aveY = d2 * d4;
            this.aveZ = d3 * d4;
        }

        public void mvtfloor(double d, double d2, double d3, double d4) {
            double d5 = this.aveX;
            double d6 = this.aveY;
            double d7 = this.aveZ;
            int n = 0;
            while (n < this.nbparticle) {
                double d8 = this.posx[n];
                double d9 = this.posy[n];
                double d10 = this.posz[n];
                if (d9 < d) {
                    this.posy[n] = d;
                    int n2 = n;
                    this.Vy[n2] = this.Vy[n2] * -1.0;
                }
                if (d9 > d2) {
                    this.posy[n] = d2;
                    int n3 = n;
                    this.Vy[n3] = this.Vy[n3] * -1.0;
                }
                int n4 = n;
                this.Vx[n4] = this.Vx[n4] * d4;
                int n5 = n;
                this.Vy[n5] = this.Vy[n5] * d4;
                int n6 = n;
                this.Vz[n6] = this.Vz[n6] * d4;
                this.Ax[n] = 0.0;
                this.Ay[n] = d3;
                this.Az[n] = 0.0;
                ++n;
            }
            this.move();
        }

        public void mvtgalax() {
            double d = this.aveX;
            double d2 = this.aveY;
            double d3 = this.aveZ;
            int n = 0;
            while (n < this.nbparticle) {
                double d4 = this.posx[n];
                double d5 = this.posy[n];
                double d6 = this.posz[n];
                double d7 = Math.pow(d4 * d4 + d5 * d5 + d6 * d6, 1.5) * 0.001;
                this.Ax[n] = -d4 * d7;
                this.Ay[n] = -d5 * d7;
                this.Az[n] = -d6 * d7;
                ++n;
            }
            this.move();
        }

        public void reset(double d, double d2, double d3, double d4, double d5, double d6, double d7, double d8) {
            double d9 = 0.0;
            double d10 = 0.0;
            double d11 = 0.0;
            double d12 = 1.0 / (double)this.nbparticle;
            double d13 = 0.0;
            double d14 = 0.0;
            double d15 = Math.PI * 2 * d * d12;
            double d16 = Math.PI * 2 * d2 * d12;
            int n = 0;
            while (n < this.nbparticle) {
                double d17 = d3 + Math.random() * (d4 - d3);
                double d18 = 0.0;
                double d19 = Math.cos(d13) * d17;
                double d20 = Math.sin(d13) * d17;
                this.posx[n] = Math.cos(d14) * d18 - Math.sin(d14) * d20;
                this.posy[n] = d19;
                this.posz[n] = Math.sin(d14) * d18 + Math.cos(d14) * d20;
                this.force[n] = 1.0;
                d17 = 0.05;
                this.Vx[n] = this.posz[n] * d17;
                this.Vy[n] = this.posx[n] * d17;
                this.Vz[n] = this.posy[n] * d17;
                d9 += this.posx[n];
                d10 += this.posy[n];
                d11 += this.posz[n];
                d13 += d15;
                d14 += d16;
                ++n;
            }
            this.aveX = d9 * d12;
            this.aveY = d10 * d12;
            this.aveZ = d11 * d12;
        }
    }

    private class kTable {
        double[] datetable;
        double[] floattable;
        int lengthtable;

        public kTable(String string) {
            String string2 = null;
            this.lengthtable = kraycasting.this.FindHowMany(string, "|") - 1;
            this.datetable = new double[this.lengthtable];
            this.floattable = new double[this.lengthtable];
            int n = 0;
            while (n < this.lengthtable) {
                string2 = kraycasting.this.FindInto(string, 1 + n, "|").trim();
                this.floattable[n] = new Double(kraycasting.this.FindInto(string2, 1, ",").trim());
                this.datetable[n] = new Double(kraycasting.this.FindInto(string2, 0, ",").trim());
                ++n;
            }
        }

        public double spline(double d) {
            int n = this.lengthtable - 1;
            if (d <= this.datetable[0]) {
                return this.floattable[0];
            }
            int n2 = 0;
            int n3 = n;
            int n4 = n2 + n3 >> 1;
            int n5 = n3;
            if (d >= this.datetable[n5]) {
                return this.floattable[n5];
            }
            n5 = n4;
            int n6 = n4 + 1;
            while (!(d >= this.datetable[n5]) || !(d < this.datetable[n6])) {
                if (d < this.datetable[n5]) {
                    n3 = n4;
                } else {
                    n2 = n4;
                }
                n5 = n4 = n2 + n3 >> 1;
                n6 = n4 + 1;
            }
            int n7 = n4 == 0 ? n5 : n5 - 1;
            int n8 = n4 + 1 == n ? n6 : n6 + 1;
            double d2 = this.datetable[n5];
            double d3 = this.datetable[n6];
            double d4 = d3 - d2;
            double d5 = (this.floattable[n6] - this.floattable[n7]) / (d3 - this.datetable[n7]);
            double d6 = (this.floattable[n8] - this.floattable[n5]) / (this.datetable[n8] - d2);
            double d7 = d5;
            double d8 = this.floattable[n5];
            double d9 = this.floattable[n6];
            double d10 = d4;
            double d11 = d10 * d10;
            double d12 = (-(d10 * d6) - d7 * 2.0 * d10 + 3.0 * (d9 - d8)) / d11;
            double d13 = (d6 - 2.0 * d12 * d10 - d7) / (3.0 * d11);
            d11 = d - d2;
            d9 = d11 * d11;
            double d14 = d9 * d11;
            d10 = d13 * d14 + d12 * d9 + d7 * d11 + d8;
            return d10;
        }
    }

    private class kTimer {
        private long StartTime = System.currentTimeMillis();

        public int getTime() {
            return (int)((System.currentTimeMillis() - this.StartTime) / 10L);
        }

        public void reset() {
            this.StartTime = System.currentTimeMillis();
        }
    }

    private class kPixel {
        int u = 0;
        int v = 0;
        int r = 0;
        int g = 0;
        int b = 0;
        double XI;
        double YI;
        double ZI;
        double XF;
        double YF;
        double ZF;
    }

    public class helpfade {
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

    abstract class kParameterX {
        int CodeParam;
        double a;
        double b;
        double c;
        int dtlength;
        kTable spltable;
        public Object kOBJ;

        kParameterX() {
        }

        public abstract Object Out();

        public abstract double doubleout(double var1);
    }

    private class kParameterXCTE
    extends kParameterX {
        kParameterXCTE() {
        }

        public Object Out() {
            return null;
        }

        public double doubleout(double d) {
            return this.a;
        }
    }

    private class kParameterXAFF
    extends kParameterX {
        kParameterXAFF() {
        }

        public Object Out() {
            return null;
        }

        public double doubleout(double d) {
            return this.a + this.b * d;
        }
    }

    private class kParameterXRND
    extends kParameterX {
        kParameterXRND() {
        }

        public Object Out() {
            return null;
        }

        public double doubleout(double d) {
            return this.a + Math.random() * (this.b - this.a);
        }
    }

    private class kParameterXSPLINE
    extends kParameterX {
        kParameterXSPLINE() {
        }

        public Object Out() {
            return null;
        }

        public double doubleout(double d) {
            return this.spltable.spline(d - this.a);
        }
    }

    private class kParameterXSIN
    extends kParameterX {
        kParameterXSIN() {
        }

        public Object Out() {
            return null;
        }

        public double doubleout(double d) {
            return this.a + Math.sin(this.b * d) * this.c;
        }
    }

    private class kParameterXCOS
    extends kParameterX {
        kParameterXCOS() {
        }

        public Object Out() {
            return null;
        }

        public double doubleout(double d) {
            return this.a + Math.cos(this.b * d) * this.c;
        }
    }

    private class kParameterXOB
    extends kParameterX {
        kParameterXOB() {
        }

        public Object Out() {
            if (this.kOBJ == null) {
                return kraycasting.this.RenderContext;
            }
            return this.kOBJ;
        }

        public double doubleout(double d) {
            return 0.0;
        }
    }

    private class kEffectX {
        int keffectcode;
        int knbparam;
        private kParameterX[] kParameterXEffectX;
        helpfade hhf;

        kEffectX() {
            this.hhf = new helpfade();
        }

        public void rendah(int n) {
            double d;
            double d2 = new Double(n);
            if (this.keffectcode == 1) {
                kraycasting.this.rcs.Position3D(this.kParameterXEffectX[0].doubleout(d2), this.kParameterXEffectX[1].doubleout(d2), 0.0, 0.0, 0.0, 0.0, 0.0, (kRenderContext)this.kParameterXEffectX[9].Out());
                kraycasting.this.rcs.findtwirl(this.kParameterXEffectX[2].doubleout(d2), this.kParameterXEffectX[3].doubleout(d2), this.kParameterXEffectX[4].doubleout(d2), this.kParameterXEffectX[5].doubleout(d2), this.kParameterXEffectX[6].doubleout(d2), this.kParameterXEffectX[7].doubleout(d2), (kRenderContext)this.kParameterXEffectX[9].Out());
                kraycasting.this.rcs.draw((kTexture)this.kParameterXEffectX[8].Out(), (kRenderContext)this.kParameterXEffectX[9].Out());
            }
            if (this.keffectcode == 2) {
                kraycasting.this.rcs.SetzoomP(this.kParameterXEffectX[1].doubleout(d2), this.kParameterXEffectX[2].doubleout(d2), this.kParameterXEffectX[3].doubleout(d2), this.kParameterXEffectX[4].doubleout(d2), (kRenderContext)this.kParameterXEffectX[5].Out(), (kTexture)this.kParameterXEffectX[0].Out(), this.kParameterXEffectX[6].doubleout(d2));
            }
            if (this.keffectcode == 3) {
                this.hhf.Frd1 = this.kParameterXEffectX[5].doubleout(d2);
                this.hhf.Frv1 = this.kParameterXEffectX[6].doubleout(d2);
                this.hhf.Frd2 = this.kParameterXEffectX[7].doubleout(d2);
                this.hhf.Frv2 = this.kParameterXEffectX[8].doubleout(d2);
                this.hhf.Fgd1 = this.kParameterXEffectX[9].doubleout(d2);
                this.hhf.Fgv1 = this.kParameterXEffectX[10].doubleout(d2);
                this.hhf.Fgd2 = this.kParameterXEffectX[11].doubleout(d2);
                this.hhf.Fgv2 = this.kParameterXEffectX[12].doubleout(d2);
                this.hhf.Fbd1 = this.kParameterXEffectX[13].doubleout(d2);
                this.hhf.Fbv1 = this.kParameterXEffectX[14].doubleout(d2);
                this.hhf.Fbd2 = this.kParameterXEffectX[15].doubleout(d2);
                this.hhf.Fbv2 = this.kParameterXEffectX[16].doubleout(d2);
                kraycasting.this.rcs.Position3D(0.0, 0.0, ((kcamera)this.kParameterXEffectX[0].Out()).Z, ((kcamera)this.kParameterXEffectX[0].Out()).O1, ((kcamera)this.kParameterXEffectX[0].Out()).O2, ((kcamera)this.kParameterXEffectX[0].Out()).O3, ((kcamera)this.kParameterXEffectX[0].Out()).FOV, (kRenderContext)this.kParameterXEffectX[4].Out());
                kraycasting.this.rcs.findtunnel(this.kParameterXEffectX[1].doubleout(d2), this.kParameterXEffectX[2].doubleout(d2), (kRenderContext)this.kParameterXEffectX[4].Out(), this.hhf);
                kraycasting.this.rcs.draw((kTexture)this.kParameterXEffectX[3].Out(), (kRenderContext)this.kParameterXEffectX[4].Out());
            }
            if (this.keffectcode == 4) {
                this.hhf.Frd1 = this.kParameterXEffectX[7].doubleout(d2);
                this.hhf.Frv1 = this.kParameterXEffectX[8].doubleout(d2);
                this.hhf.Frd2 = this.kParameterXEffectX[9].doubleout(d2);
                this.hhf.Frv2 = this.kParameterXEffectX[10].doubleout(d2);
                this.hhf.Fgd1 = this.kParameterXEffectX[11].doubleout(d2);
                this.hhf.Fgv1 = this.kParameterXEffectX[12].doubleout(d2);
                this.hhf.Fgd2 = this.kParameterXEffectX[13].doubleout(d2);
                this.hhf.Fgv2 = this.kParameterXEffectX[14].doubleout(d2);
                this.hhf.Fbd1 = this.kParameterXEffectX[15].doubleout(d2);
                this.hhf.Fbv1 = this.kParameterXEffectX[16].doubleout(d2);
                this.hhf.Fbd2 = this.kParameterXEffectX[17].doubleout(d2);
                this.hhf.Fbv2 = this.kParameterXEffectX[18].doubleout(d2);
                kraycasting.this.rcs.Position3D(((kcamera)this.kParameterXEffectX[0].Out()).X, ((kcamera)this.kParameterXEffectX[0].Out()).Y, ((kcamera)this.kParameterXEffectX[0].Out()).Z, ((kcamera)this.kParameterXEffectX[0].Out()).O1, ((kcamera)this.kParameterXEffectX[0].Out()).O2, ((kcamera)this.kParameterXEffectX[0].Out()).O3, ((kcamera)this.kParameterXEffectX[0].Out()).FOV, (kRenderContext)this.kParameterXEffectX[4].Out());
                kraycasting.this.rcs.findfloor(this.kParameterXEffectX[1].doubleout(d2), this.kParameterXEffectX[2].doubleout(d2), this.kParameterXEffectX[5].doubleout(d2), this.kParameterXEffectX[6].doubleout(d2), (kRenderContext)this.kParameterXEffectX[4].Out(), this.hhf);
                kraycasting.this.rcs.draw((kTexture)this.kParameterXEffectX[3].Out(), (kRenderContext)this.kParameterXEffectX[4].Out());
            }
            if (this.keffectcode == 5) {
                kraycasting.this.rcs.Setzoom3D(this.kParameterXEffectX[0].doubleout(d2), this.kParameterXEffectX[1].doubleout(d2), this.kParameterXEffectX[2].doubleout(d2), this.kParameterXEffectX[3].doubleout(d2), this.kParameterXEffectX[4].doubleout(d2), this.kParameterXEffectX[5].doubleout(d2), (kRenderContext)this.kParameterXEffectX[7].Out(), (kTexture)this.kParameterXEffectX[6].Out(), this.kParameterXEffectX[8].doubleout(d2));
            }
            if (this.keffectcode == 6) {
                ((kRenderContext)this.kParameterXEffectX[0].Out()).SetRect(kraycasting.this.RenderContext, this.kParameterXEffectX[1].doubleout(d2), this.kParameterXEffectX[2].doubleout(d2), this.kParameterXEffectX[3].doubleout(d2), this.kParameterXEffectX[4].doubleout(d2));
            }
            if (this.keffectcode == 7) {
                kraycasting.this.rcs.flare3D(this.kParameterXEffectX[0].doubleout(d2), this.kParameterXEffectX[1].doubleout(d2), this.kParameterXEffectX[2].doubleout(d2), this.kParameterXEffectX[3].doubleout(d2), this.kParameterXEffectX[4].doubleout(d2), this.kParameterXEffectX[5].doubleout(d2), (kRenderContext)this.kParameterXEffectX[7].Out(), (kTexture)this.kParameterXEffectX[6].Out());
            }
            if (this.keffectcode == 8) {
                kraycasting.this.rcs.MotionBlur((kRenderContext)this.kParameterXEffectX[0].Out());
            }
            if (this.keffectcode == 9) {
                double d3 = this.kParameterXEffectX[4].doubleout(d2);
                if (d3 == 0.0) {
                    ((kparticle)this.kParameterXEffectX[0].Out()).mvtgalax();
                } else if (d3 == 1.0) {
                    ((kparticle)this.kParameterXEffectX[0].Out()).mvtfloor(this.kParameterXEffectX[9].doubleout(d2), this.kParameterXEffectX[10].doubleout(d2), this.kParameterXEffectX[11].doubleout(d2), this.kParameterXEffectX[12].doubleout(d2));
                }
                d = this.kParameterXEffectX[5].doubleout(d2);
                if (d == 0.0) {
                    ((kparticle)this.kParameterXEffectX[0].Out()).drawflare((kcamera)this.kParameterXEffectX[6].Out(), (kTexture)this.kParameterXEffectX[2].Out(), (kRenderContext)this.kParameterXEffectX[1].Out(), this.kParameterXEffectX[7].doubleout(d2), this.kParameterXEffectX[8].doubleout(d2));
                }
                if (d == 2.0) {
                    ((kparticle)this.kParameterXEffectX[0].Out()).drawsprit((kcamera)this.kParameterXEffectX[6].Out(), (kTexture)this.kParameterXEffectX[2].Out(), (kRenderContext)this.kParameterXEffectX[1].Out(), this.kParameterXEffectX[7].doubleout(d2), this.kParameterXEffectX[8].doubleout(d2));
                }
            }
            if (this.keffectcode == 10) {
                ((kcamera)this.kParameterXEffectX[0].Out()).set(this.kParameterXEffectX[1].doubleout(d2), this.kParameterXEffectX[2].doubleout(d2), this.kParameterXEffectX[3].doubleout(d2), this.kParameterXEffectX[4].doubleout(d2), this.kParameterXEffectX[5].doubleout(d2), this.kParameterXEffectX[6].doubleout(d2), this.kParameterXEffectX[7].doubleout(d2));
            }
            if (this.keffectcode == 11) {
                ((kcamera)this.kParameterXEffectX[0].Out()).target(this.kParameterXEffectX[1].doubleout(d2), this.kParameterXEffectX[2].doubleout(d2), this.kParameterXEffectX[3].doubleout(d2), this.kParameterXEffectX[4].doubleout(d2), this.kParameterXEffectX[5].doubleout(d2), this.kParameterXEffectX[6].doubleout(d2), this.kParameterXEffectX[7].doubleout(d2), this.kParameterXEffectX[8].doubleout(d2));
            }
            if (this.keffectcode == 12) {
                d = this.kParameterXEffectX[1].doubleout(d2);
                if (d2 < d) {
                    ((kSound)this.kParameterXEffectX[0].Out()).playedonce = false;
                } else if (!((kSound)this.kParameterXEffectX[0].Out()).playedonce) {
                    ((kSound)this.kParameterXEffectX[0].Out()).PlayOne();
                }
            }
        }
    }

    private class kSoundCommand {
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

    private class kPartX {
        int NBEffect;
        kEffectX[] kEffectXPartX;
        int TimeLength;
        int TimeStart;
        int NBSoundCommands;
        kSoundCommand[] SoundCommands;

        kPartX() {
        }

        public void rendah(int n) {
            int n2 = n - this.TimeLength + this.TimeStart;
            int n3 = 0;
            while (n3 < this.NBEffect) {
                this.kEffectXPartX[n3].rendah(n2);
                ++n3;
            }
            if (((kraycasting)kraycasting.this).THEkScript.lastpartplayed != this && this.SoundCommands != null) {
                n3 = 0;
                while (n3 < this.NBSoundCommands) {
                    this.SoundCommands[n3].MakeSoundCommand();
                    ++n3;
                }
            }
            ((kraycasting)kraycasting.this).THEkScript.lastpartplayed = this;
        }
    }

    private class kScript {
        int NBPart;
        int EndDate;
        int EndPref;
        private kPartX[] kPartXscript;
        kPartX lastpartplayed;

        public kScript(String string) {
            int n;
            int n2;
            String[] stringArray = kraycasting.this.SplitSTag(string, "KIMAGES");
            String[] stringArray2 = kraycasting.this.SplitSTag(string, "KSOUNDS");
            String[] stringArray3 = kraycasting.this.SplitSTag(string, "KRECT");
            String[] stringArray4 = kraycasting.this.SplitSTag(string, "KTABLE");
            String[] stringArray5 = kraycasting.this.SplitSTag(string, "KSCRIPT");
            String[] stringArray6 = kraycasting.this.SplitSTag(string, "KTICLE");
            String[] stringArray7 = kraycasting.this.SplitSTag(string, "KCAM");
            System.out.println("table");
            kraycasting.this.TableHash = new Hashtable();
            int n3 = 0;
            while (n3 < stringArray4.length) {
                Hashtable hashtable = kraycasting.this.TableHash;
                String string2 = kraycasting.this.FindInto(stringArray4[n3], 0, "|").trim();
                kraycasting kraycasting3 = kraycasting.this;
                kraycasting3.getClass();
                hashtable.put(string2, kraycasting3.new kTable(stringArray4[n3].trim()));
                ++n3;
            }
            System.out.println("image");
            kraycasting.this.ImageHash = new Hashtable();
            n3 = 0;
            while (n3 < stringArray.length) {
                Hashtable hashtable = kraycasting.this.ImageHash;
                String string3 = kraycasting.this.FindInto(stringArray[n3], 0, "|").trim();
                kraycasting kraycasting4 = kraycasting.this;
                kraycasting4.getClass();
                hashtable.put(string3, kraycasting4.new kTexture(kraycasting.this.getImage(kraycasting.this.getDocumentBase(), kraycasting.this.FindInto(stringArray[n3], 1, "|").trim())));
                ++n3;
            }
            System.out.println("particl");
            kraycasting.this.partcHash = new Hashtable();
            n3 = 0;
            while (n3 < stringArray6.length) {
                Hashtable hashtable = kraycasting.this.partcHash;
                String string4 = kraycasting.this.FindInto(stringArray6[n3], 0, "|").trim();
                kraycasting kraycasting5 = kraycasting.this;
                kraycasting5.getClass();
                hashtable.put(string4, kraycasting5.new kparticle(new Integer(kraycasting.this.FindInto(stringArray6[n3], 1, "|").trim()), new Double(kraycasting.this.FindInto(stringArray6[n3], 2, "|").trim()), new Double(kraycasting.this.FindInto(stringArray6[n3], 3, "|").trim()), new Double(kraycasting.this.FindInto(stringArray6[n3], 4, "|").trim()), new Double(kraycasting.this.FindInto(stringArray6[n3], 5, "|").trim()), new Double(kraycasting.this.FindInto(stringArray6[n3], 6, "|").trim()), new Double(kraycasting.this.FindInto(stringArray6[n3], 7, "|").trim()), new Double(kraycasting.this.FindInto(stringArray6[n3], 8, "|").trim()), new Double(kraycasting.this.FindInto(stringArray6[n3], 9, "|").trim())));
                ++n3;
            }
            System.out.println("camera");
            kraycasting.this.CamerHash = new Hashtable();
            n3 = 0;
            while (n3 < stringArray7.length) {
                Hashtable hashtable = kraycasting.this.CamerHash;
                String string5 = kraycasting.this.FindInto(stringArray7[n3], 0, "|").trim();
                kraycasting kraycasting6 = kraycasting.this;
                kraycasting6.getClass();
                hashtable.put(string5, kraycasting6.new kcamera(new Integer(kraycasting.this.FindInto(stringArray7[n3], 1, "|").trim()).intValue(), new Integer(kraycasting.this.FindInto(stringArray7[n3], 2, "|").trim()).intValue(), new Integer(kraycasting.this.FindInto(stringArray7[n3], 3, "|").trim()).intValue(), new Integer(kraycasting.this.FindInto(stringArray7[n3], 4, "|").trim()).intValue(), new Integer(kraycasting.this.FindInto(stringArray7[n3], 5, "|").trim()).intValue(), new Integer(kraycasting.this.FindInto(stringArray7[n3], 6, "|").trim()).intValue(), new Integer(kraycasting.this.FindInto(stringArray7[n3], 7, "|").trim()).intValue()));
                ++n3;
            }
            System.out.println("rect");
            kraycasting.this.RectaHash = new Hashtable();
            n3 = 0;
            while (n3 < stringArray3.length) {
                Hashtable hashtable = kraycasting.this.RectaHash;
                String string6 = kraycasting.this.FindInto(stringArray3[n3], 0, "|").trim();
                kraycasting kraycasting7 = kraycasting.this;
                kraycasting7.getClass();
                hashtable.put(string6, kraycasting7.new kRenderContext(kraycasting.this.RenderContext, new Double(kraycasting.this.FindInto(stringArray3[n3], 1, "|").trim()), new Double(kraycasting.this.FindInto(stringArray3[n3], 2, "|").trim()), new Double(kraycasting.this.FindInto(stringArray3[n3], 3, "|").trim()), new Double(kraycasting.this.FindInto(stringArray3[n3], 4, "|").trim())));
                ++n3;
            }
            System.out.println("sound");
            kraycasting.this.SoundHash = new Hashtable();
            n3 = 0;
            while (n3 < stringArray2.length) {
                kraycasting.this.SoundHash.put(kraycasting.this.FindInto(stringArray2[n3], 0, "|").trim(), new kSound(kraycasting.this.getAudioClip(kraycasting.this.getDocumentBase(), kraycasting.this.FindInto(stringArray2[n3], 1, "|").trim())));
                ++n3;
            }
            String[] stringArray8 = kraycasting.this.SplitSTag(stringArray5[0], "KPART");
            this.NBPart = stringArray8.length;
            this.kPartXscript = new kPartX[this.NBPart + 1];
            String[] stringArray9 = kraycasting.this.SplitSTag(stringArray5[0], "KEND");
            this.EndPref = 0;
            if (stringArray9[0].trim().toLowerCase().compareTo("loop") == 0) {
                this.EndPref = 1;
            }
            int n4 = 0;
            while (n4 < stringArray8.length) {
                String string7;
                stringArray9 = kraycasting.this.SplitSTag(stringArray8[n4], "D");
                String[] stringArray10 = kraycasting.this.SplitSTag(stringArray8[n4], "S");
                this.kPartXscript[n4] = new kPartX();
                if (stringArray10 != null) {
                    this.kPartXscript[n4].NBSoundCommands = stringArray10.length;
                    this.kPartXscript[n4].SoundCommands = new kSoundCommand[this.kPartXscript[n4].NBSoundCommands];
                    n2 = 0;
                    while (n2 < this.kPartXscript[n4].NBSoundCommands) {
                        n = 0;
                        string7 = kraycasting.this.FindInto(stringArray10[n2], 1, ",").trim().toLowerCase();
                        if (string7.compareTo("play") == 0) {
                            n = 1;
                        }
                        if (string7.compareTo("loop") == 0) {
                            n = 2;
                        }
                        this.kPartXscript[n4].SoundCommands[n2] = new kSoundCommand();
                        this.kPartXscript[n4].SoundCommands[n2].commandcode = n;
                        this.kPartXscript[n4].SoundCommands[n2].snd = (kSound)kraycasting.this.SoundHash.get(kraycasting.this.FindInto(stringArray10[n2], 0, ","));
                        ++n2;
                    }
                }
                this.kPartXscript[n4].TimeLength = new Integer(kraycasting.this.FindInto(stringArray9[0], 0, ",").trim());
                this.kPartXscript[n4].TimeStart = new Integer(kraycasting.this.FindInto(stringArray9[0], 1, ",").trim());
                String[] stringArray11 = kraycasting.this.SplitSTag(stringArray8[n4], "Fx");
                this.kPartXscript[n4].NBEffect = stringArray11.length;
                this.kPartXscript[n4].kEffectXPartX = new kEffectX[this.kPartXscript[n4].NBEffect];
                n2 = 0;
                while (n2 < stringArray11.length) {
                    this.kPartXscript[n4].kEffectXPartX[n2] = new kEffectX();
                    String[] stringArray12 = kraycasting.this.SplitSTag(stringArray11[n2], "Pa");
                    this.kPartXscript[n4].kEffectXPartX[n2].knbparam = stringArray12.length;
                    this.kPartXscript[n4].kEffectXPartX[n2].kParameterXEffectX = new kParameterX[this.kPartXscript[n4].kEffectXPartX[n2].knbparam];
                    string7 = kraycasting.this.FindInto(stringArray12[0], 0, "|").trim().toLowerCase();
                    n = 0;
                    if (string7.compareTo("twirl") == 0) {
                        n = 1;
                    }
                    if (string7.compareTo("sprit") == 0) {
                        n = 2;
                    }
                    if (string7.compareTo("tunnel") == 0) {
                        n = 3;
                    }
                    if (string7.compareTo("ground") == 0) {
                        n = 4;
                    }
                    if (string7.compareTo("sprit3d") == 0) {
                        n = 5;
                    }
                    if (string7.compareTo("setrect") == 0) {
                        n = 6;
                    }
                    if (string7.compareTo("flare3d") == 0) {
                        n = 7;
                    }
                    if (string7.compareTo("motionblur") == 0) {
                        n = 8;
                    }
                    if (string7.compareTo("particle") == 0) {
                        n = 9;
                    }
                    if (string7.compareTo("setcamcoord") == 0) {
                        n = 10;
                    }
                    if (string7.compareTo("setcamtarget") == 0) {
                        n = 11;
                    }
                    if (string7.compareTo("evesound") == 0) {
                        n = 12;
                    }
                    this.kPartXscript[n4].kEffectXPartX[n2].keffectcode = n;
                    System.out.println(" ");
                    n3 = 1;
                    while (n3 < this.kPartXscript[n4].kEffectXPartX[n2].knbparam) {
                        kParameterX kParameterX2;
                        string7 = kraycasting.this.FindInto(stringArray12[n3], 0, "|").trim().toLowerCase();
                        System.out.print("   Pa:" + string7);
                        if (string7.compareTo("ima") == 0) {
                            kParameterX2 = new kParameterXOB();
                            kParameterX2.CodeParam = 1;
                            kParameterX2.kOBJ = (kTexture)kraycasting.this.ImageHash.get(kraycasting.this.FindInto(stringArray12[n3], 1, "|"));
                            ((kEffectX)this.kPartXscript[n4].kEffectXPartX[n2]).kParameterXEffectX[n3 - 1] = kParameterX2;
                        }
                        if (string7.compareTo("cos") == 0) {
                            kParameterX2 = new kParameterXCOS();
                            kParameterX2.CodeParam = 2;
                            kParameterX2.a = new Double(kraycasting.this.FindInto(stringArray12[n3], 1, "|"));
                            kParameterX2.b = new Double(kraycasting.this.FindInto(stringArray12[n3], 2, "|"));
                            kParameterX2.c = new Double(kraycasting.this.FindInto(stringArray12[n3], 3, "|"));
                            ((kEffectX)this.kPartXscript[n4].kEffectXPartX[n2]).kParameterXEffectX[n3 - 1] = kParameterX2;
                        }
                        if (string7.compareTo("sin") == 0) {
                            kParameterX2 = new kParameterXSIN();
                            kParameterX2.CodeParam = 3;
                            kParameterX2.a = new Double(kraycasting.this.FindInto(stringArray12[n3], 1, "|"));
                            kParameterX2.b = new Double(kraycasting.this.FindInto(stringArray12[n3], 2, "|"));
                            kParameterX2.c = new Double(kraycasting.this.FindInto(stringArray12[n3], 3, "|"));
                            ((kEffectX)this.kPartXscript[n4].kEffectXPartX[n2]).kParameterXEffectX[n3 - 1] = kParameterX2;
                        }
                        if (string7.compareTo("cte") == 0) {
                            kParameterX2 = new kParameterXCTE();
                            kParameterX2.CodeParam = 4;
                            kParameterX2.a = new Double(kraycasting.this.FindInto(stringArray12[n3], 1, "|"));
                            ((kEffectX)this.kPartXscript[n4].kEffectXPartX[n2]).kParameterXEffectX[n3 - 1] = kParameterX2;
                        }
                        if (string7.compareTo("rnd") == 0) {
                            kParameterX2 = new kParameterXRND();
                            kParameterX2.CodeParam = 5;
                            kParameterX2.a = new Double(kraycasting.this.FindInto(stringArray12[n3], 1, "|"));
                            kParameterX2.b = new Double(kraycasting.this.FindInto(stringArray12[n3], 2, "|"));
                            ((kEffectX)this.kPartXscript[n4].kEffectXPartX[n2]).kParameterXEffectX[n3 - 1] = kParameterX2;
                        }
                        if (string7.compareTo("spl") == 0) {
                            kParameterX2 = new kParameterXSPLINE();
                            kParameterX2.CodeParam = 6;
                            kParameterX2.spltable = (kTable)kraycasting.this.TableHash.get(kraycasting.this.FindInto(stringArray12[n3], 1, "|"));
                            kParameterX2.a = new Double(kraycasting.this.FindInto(stringArray12[n3], 2, "|"));
                            ((kEffectX)this.kPartXscript[n4].kEffectXPartX[n2]).kParameterXEffectX[n3 - 1] = kParameterX2;
                        }
                        if (string7.compareTo("aff") == 0) {
                            kParameterX2 = new kParameterXAFF();
                            kParameterX2.CodeParam = 7;
                            kParameterX2.a = new Double(kraycasting.this.FindInto(stringArray12[n3], 1, "|"));
                            kParameterX2.b = new Double(kraycasting.this.FindInto(stringArray12[n3], 2, "|"));
                            ((kEffectX)this.kPartXscript[n4].kEffectXPartX[n2]).kParameterXEffectX[n3 - 1] = kParameterX2;
                        }
                        if (string7.compareTo("rec") == 0) {
                            kParameterX2 = new kParameterXOB();
                            kParameterX2.CodeParam = 8;
                            kParameterX2.kOBJ = (kRenderContext)kraycasting.this.RectaHash.get(kraycasting.this.FindInto(stringArray12[n3], 1, "|"));
                            ((kEffectX)this.kPartXscript[n4].kEffectXPartX[n2]).kParameterXEffectX[n3 - 1] = kParameterX2;
                        }
                        if (string7.compareTo("par") == 0) {
                            kParameterX2 = new kParameterXOB();
                            kParameterX2.CodeParam = 9;
                            kParameterX2.kOBJ = (kparticle)kraycasting.this.partcHash.get(kraycasting.this.FindInto(stringArray12[n3], 1, "|"));
                            ((kEffectX)this.kPartXscript[n4].kEffectXPartX[n2]).kParameterXEffectX[n3 - 1] = kParameterX2;
                        }
                        if (string7.compareTo("cam") == 0) {
                            kParameterX2 = new kParameterXOB();
                            kParameterX2.CodeParam = 10;
                            kParameterX2.kOBJ = (kcamera)kraycasting.this.CamerHash.get(kraycasting.this.FindInto(stringArray12[n3], 1, "|"));
                            ((kEffectX)this.kPartXscript[n4].kEffectXPartX[n2]).kParameterXEffectX[n3 - 1] = kParameterX2;
                        }
                        if (string7.compareTo("snd") == 0) {
                            kParameterX2 = new kParameterXOB();
                            kParameterX2.CodeParam = 11;
                            kParameterX2.kOBJ = (kSound)kraycasting.this.SoundHash.get(kraycasting.this.FindInto(stringArray12[n3], 1, "|"));
                            ((kEffectX)this.kPartXscript[n4].kEffectXPartX[n2]).kParameterXEffectX[n3 - 1] = kParameterX2;
                        }
                        ++n3;
                    }
                    ++n2;
                }
                ++n4;
            }
            this.kPartXscript[n4] = new kPartX();
            n = 0;
            n3 = 0;
            while (n3 < this.NBPart) {
                n2 = this.kPartXscript[n3].TimeLength;
                this.kPartXscript[n3].TimeLength = n;
                n += n2;
                ++n3;
            }
            this.kPartXscript[n4].TimeLength = n;
            this.EndDate = n;
        }

        public kPartX part(int n) {
            int n2 = 0;
            int n3 = 0;
            while (n3 < this.NBPart) {
                if (this.kPartXscript[n3 + 1].TimeLength >= n && this.kPartXscript[n3].TimeLength < n) {
                    n2 = n3;
                }
                ++n3;
            }
            if (n >= this.EndDate) {
                if (this.EndPref == 1) {
                    kraycasting.this.kTime.reset();
                } else {
                    kraycasting.this.boolend = true;
                }
            }
            return this.kPartXscript[n2];
        }
    }

    private class raycasting {
        private kRenderContext OneRec;
        private kRenderContext Rootrc;

        public raycasting(kRenderContext kRenderContext2) {
            this.Rootrc = kRenderContext2;
            kraycasting kraycasting3 = kraycasting.this;
            kraycasting3.getClass();
            this.OneRec = kraycasting3.new kRenderContext(kRenderContext2);
        }

        public void MotionBlur(kRenderContext kRenderContext2) {
            int n = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            int n2 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            int n3 = kRenderContext2.Modulo;
            int n4 = kRenderContext2.ClipX1 + kRenderContext2.ClipY1 * kRenderContext2.Modulo;
            int[] nArray = ((kRenderContext)kRenderContext2).RC_RootContext.CurrentChunk;
            int[] nArray2 = ((kRenderContext)kRenderContext2).RC_RootContext.Chunk2;
            if (n == 0) {
                return;
            }
            if (n2 == 0) {
                return;
            }
            int n5 = 0;
            while (n5 < n2) {
                int n6 = n4;
                int n7 = 0;
                while (n7 < n) {
                    int n8 = nArray2[n6] & 0xFCFCFF;
                    int n9 = nArray[n6] & 0xFCFCFF;
                    nArray[n6++] = 0xFF000000 | n8 + n9 >> 1;
                    ++n7;
                }
                n4 += n3;
                ++n5;
            }
        }

        public void Position3D(double d, double d2, double d3, double d4, double d5, double d6, double d7, kRenderContext kRenderContext2) {
            if (!kRenderContext2.drawable) {
                return;
            }
            double d8 = Math.cos(d4);
            double d9 = Math.sin(d4);
            double d10 = Math.cos(d5);
            double d11 = Math.sin(d5);
            double d12 = Math.cos(d6);
            double d13 = Math.sin(d6);
            kRenderContext2.PosX = d;
            kRenderContext2.PosY = d2;
            kRenderContext2.PosZ = d3;
            int n = 0;
            while (n < kRenderContext2.MaxPreY) {
                int n2 = 0;
                while (n2 < kRenderContext2.MaxPreX) {
                    double d14 = ((kRenderContext)kRenderContext2).Pretable[n2][n].XI;
                    double d15 = ((kRenderContext)kRenderContext2).Pretable[n2][n].YI;
                    double d16 = d8 * d14 - d9 * d15;
                    double d17 = d9 * d14 + d8 * d15;
                    double d18 = d7;
                    double d19 = d16;
                    double d20 = d10 * d17 - d11 * d18;
                    double d21 = d11 * d17 + d10 * d18;
                    ((kRenderContext)kRenderContext2).Pretable[n2][n].XF = d13 * d21 + d12 * d19;
                    ((kRenderContext)kRenderContext2).Pretable[n2][n].YF = d20;
                    ((kRenderContext)kRenderContext2).Pretable[n2][n].ZF = d12 * d21 - d13 * d19;
                    ++n2;
                }
                ++n;
            }
        }

        public void Setzoom3D(double d, double d2, double d3, double d4, double d5, double d6, kRenderContext kRenderContext2, kTexture kTexture2, double d7) {
            double d8;
            double d9;
            double d10;
            if (!kRenderContext2.drawable) {
                return;
            }
            double d11 = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            double d12 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            double d13 = d6 / d3;
            if (d3 <= 0.01) {
                return;
            }
            double d14 = (double)kTexture2.Modulo / d11;
            double d15 = (double)kTexture2.Height / d12;
            double d16 = d10 = d * d13;
            double d17 = d9 = d2 * d13;
            double d18 = 0.0;
            double d19 = 0.0;
            double d20 = kTexture2.Modulo - 1;
            double d21 = kTexture2.Height - 1;
            d14 = d14 * 0.5 * d4 * d13;
            d15 = d15 * 0.5 * d5 * d13;
            d10 -= d14;
            d9 -= d15;
            d16 += d14;
            d17 += d15;
            d10 = d10 * d11 + (double)kRenderContext2.ClipX1 + d11 * 0.5;
            d9 = d9 * d12 + (double)kRenderContext2.ClipY1 + d12 * 0.5;
            d16 = d16 * d11 + (double)kRenderContext2.ClipX1 + d11 * 0.5;
            d17 = d17 * d12 + (double)kRenderContext2.ClipY1 + d12 * 0.5;
            if (d10 > d16) {
                d8 = d16;
                d16 = d10;
                d10 = d8;
                d8 = d20;
                d20 = d18;
                d18 = d8;
            }
            if (d9 > d17) {
                d8 = d17;
                d17 = d9;
                d9 = d8;
                d8 = d21;
                d21 = d19;
                d19 = d8;
            }
            if (d16 > (double)kRenderContext2.ClipX2) {
                d20 = ((double)kRenderContext2.ClipX2 - d10) * (d20 - d18) / (d16 - d10);
                d16 = kRenderContext2.ClipX2;
            }
            if (d10 < (double)kRenderContext2.ClipX1) {
                d18 = ((double)kRenderContext2.ClipX1 - d10) * (d20 - d18) / (d16 - d10);
                d10 = kRenderContext2.ClipX1;
            }
            if (d17 > (double)kRenderContext2.ClipY2) {
                d21 = ((double)kRenderContext2.ClipY2 - d9) * (d21 - d19) / (d17 - d9);
                d17 = kRenderContext2.ClipY2;
            }
            if (d9 < (double)kRenderContext2.ClipY1) {
                d19 = ((double)kRenderContext2.ClipY1 - d9) * (d21 - d19) / (d17 - d9);
                d9 = kRenderContext2.ClipY1;
            }
            ((kRenderContext)this.Rootrc).Pretable[0][1].u = ((kRenderContext)this.Rootrc).Pretable[0][0].u = (int)(d18 * 65536.0);
            ((kRenderContext)this.Rootrc).Pretable[1][1].u = ((kRenderContext)this.Rootrc).Pretable[1][0].u = (int)(d20 * 65536.0);
            ((kRenderContext)this.Rootrc).Pretable[1][0].v = ((kRenderContext)this.Rootrc).Pretable[0][0].v = (int)(d19 * 65536.0);
            ((kRenderContext)this.Rootrc).Pretable[1][1].v = ((kRenderContext)this.Rootrc).Pretable[0][1].v = (int)(d21 * 65536.0);
            this.OneRec.SetSub(kRenderContext2, (int)d10, (int)d9, (int)d16, (int)d17);
            this.drawrecUVSA(kTexture2, this.OneRec, this.Rootrc.Pretable[0][0], this.Rootrc.Pretable[1][0], this.Rootrc.Pretable[0][1], this.Rootrc.Pretable[1][1], (int)d7);
        }

        public void SetzoomP(double d, double d2, double d3, double d4, kRenderContext kRenderContext2, kTexture kTexture2, double d5) {
            double d6;
            if (!kRenderContext2.drawable) {
                return;
            }
            double d7 = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            double d8 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            double d9 = 0.0;
            double d10 = 0.0;
            double d11 = kTexture2.Modulo - 1;
            double d12 = kTexture2.Height - 1;
            d = d * d7 + (double)kRenderContext2.ClipX1;
            d2 = d2 * d8 + (double)kRenderContext2.ClipY1;
            d3 = d3 * d7 + (double)kRenderContext2.ClipX1;
            d4 = d4 * d8 + (double)kRenderContext2.ClipY1;
            if (d > d3) {
                d6 = d3;
                d3 = d;
                d = d6;
                d6 = d11;
                d11 = d9;
                d9 = d6;
            }
            if (d2 > d4) {
                d6 = d4;
                d4 = d2;
                d2 = d6;
                d6 = d12;
                d12 = d10;
                d10 = d6;
            }
            if (d3 > (double)kRenderContext2.ClipX2) {
                d11 = ((double)kRenderContext2.ClipX2 - d) * (d11 - d9) / (d3 - d);
                d3 = kRenderContext2.ClipX2;
            }
            if (d < (double)kRenderContext2.ClipX1) {
                d9 = ((double)kRenderContext2.ClipX1 - d) * (d11 - d9) / (d3 - d);
                d = kRenderContext2.ClipX1;
            }
            if (d4 > (double)kRenderContext2.ClipY2) {
                d12 = ((double)kRenderContext2.ClipY2 - d2) * (d12 - d10) / (d4 - d2);
                d4 = kRenderContext2.ClipY2;
            }
            if (d2 < (double)kRenderContext2.ClipY1) {
                d10 = ((double)kRenderContext2.ClipY1 - d2) * (d12 - d10) / (d4 - d2);
                d2 = kRenderContext2.ClipY1;
            }
            ((kRenderContext)this.Rootrc).Pretable[0][1].u = ((kRenderContext)this.Rootrc).Pretable[0][0].u = (int)(d9 * 65536.0);
            ((kRenderContext)this.Rootrc).Pretable[1][1].u = ((kRenderContext)this.Rootrc).Pretable[1][0].u = (int)(d11 * 65536.0);
            ((kRenderContext)this.Rootrc).Pretable[1][0].v = ((kRenderContext)this.Rootrc).Pretable[0][0].v = (int)(d10 * 65536.0);
            ((kRenderContext)this.Rootrc).Pretable[1][1].v = ((kRenderContext)this.Rootrc).Pretable[0][1].v = (int)(d12 * 65536.0);
            this.OneRec.SetSub(kRenderContext2, (int)d, (int)d2, (int)d3, (int)d4);
            this.drawrecUVSA(kTexture2, this.OneRec, this.Rootrc.Pretable[0][0], this.Rootrc.Pretable[1][0], this.Rootrc.Pretable[0][1], this.Rootrc.Pretable[1][1], (int)d5);
        }

        void colorclip(helpfade helpfade2, kPixel kPixel2, double d) {
            kPixel2.r = (int)helpfade2.Frv1;
            if (d > helpfade2.Frd2) {
                kPixel2.r = (int)helpfade2.Frv2;
            } else if (d > helpfade2.Frd1) {
                kPixel2.r = (int)(helpfade2.Frv1 + (helpfade2.Frv2 - helpfade2.Frv1) * (d - helpfade2.Frd1) / (helpfade2.Frd2 - helpfade2.Frd1));
            }
            kPixel2.g = (int)helpfade2.Fgv1;
            if (d > helpfade2.Fgd2) {
                kPixel2.g = (int)helpfade2.Fgv2;
            } else if (d > helpfade2.Fgd1) {
                kPixel2.g = (int)(helpfade2.Fgv1 + (helpfade2.Fgv2 - helpfade2.Fgv1) * (d - helpfade2.Fgd1) / (helpfade2.Fgd2 - helpfade2.Fgd1));
            }
            kPixel2.b = (int)helpfade2.Fbv1;
            if (d > helpfade2.Fbd2) {
                kPixel2.b = (int)helpfade2.Fbv2;
            } else if (d > helpfade2.Fbd1) {
                kPixel2.b = (int)(helpfade2.Fbv1 + (helpfade2.Fbv2 - helpfade2.Fbv1) * (d - helpfade2.Fbd1) / (helpfade2.Fbd2 - helpfade2.Fbd1));
            }
        }

        public void draw(kTexture kTexture2, kRenderContext kRenderContext2) {
            int n = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            int n2 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            int n3 = 0;
            int n4 = 0;
            if (!kRenderContext2.drawable) {
                return;
            }
            int n5 = 0;
            while (n5 < n2) {
                n3 = n5 + 16 < n2 ? 16 : n2 - n5;
                int n6 = 0;
                while (n6 < n) {
                    n4 = n6 + 16 < n ? 16 : n - n6;
                    this.OneRec.SetSub(kRenderContext2, n6 + kRenderContext2.ClipX1, n5 + kRenderContext2.ClipY1, n6 + n4 + kRenderContext2.ClipX1, n5 + n3 + kRenderContext2.ClipY1);
                    this.drawrecUVRGB(kTexture2, this.OneRec, kRenderContext2.Pretable[n6 >> 4][n5 >> 4], kRenderContext2.Pretable[(n6 >> 4) + 1][n5 >> 4], kRenderContext2.Pretable[n6 >> 4][(n5 >> 4) + 1], kRenderContext2.Pretable[(n6 >> 4) + 1][(n5 >> 4) + 1]);
                    n6 += 16;
                }
                n5 += 16;
            }
        }

        public void drawrecUV(kTexture kTexture2, kRenderContext kRenderContext2, kPixel kPixel2, kPixel kPixel3, kPixel kPixel4, kPixel kPixel5) {
            int n = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            int n2 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            int n3 = kRenderContext2.Modulo;
            int n4 = kRenderContext2.ClipX1 + kRenderContext2.ClipY1 * kRenderContext2.Modulo;
            int[] nArray = ((kRenderContext)kRenderContext2).RC_RootContext.CurrentChunk;
            int[] nArray2 = kTexture2.pixels;
            int n5 = kPixel2.u;
            int n6 = kPixel2.v;
            int n7 = (kPixel4.u - n5) / n2;
            int n8 = (kPixel4.v - n6) / n2;
            int n9 = kPixel3.u;
            int n10 = kPixel3.v;
            int n11 = (kPixel5.u - n9) / n2;
            int n12 = (kPixel5.v - n10) / n2;
            int n13 = 0;
            while (n13 < n2) {
                int n14 = n4;
                int n15 = n5;
                int n16 = n6;
                int n17 = (n9 - n15) / n;
                int n18 = (n10 - n16) / n;
                int n19 = 0;
                while (n19 < n) {
                    nArray[n14++] = nArray2[n16 >> 8 & 0xFF00 | n15 >> 16 & 0xFF];
                    n15 += n17;
                    n16 += n18;
                    ++n19;
                }
                n5 += n7;
                n6 += n8;
                n9 += n11;
                n10 += n12;
                n4 += n3;
                ++n13;
            }
        }

        public void drawrecUVL(kTexture kTexture2, kRenderContext kRenderContext2, kPixel kPixel2, kPixel kPixel3, kPixel kPixel4, kPixel kPixel5) {
            int n = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            int n2 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            int n3 = kRenderContext2.Modulo;
            int n4 = kRenderContext2.ClipX1 + kRenderContext2.ClipY1 * kRenderContext2.Modulo;
            int[] nArray = ((kRenderContext)kRenderContext2).RC_RootContext.CurrentChunk;
            int[] nArray2 = kTexture2.pixels;
            int n5 = 0;
            int n6 = 255;
            int n7 = kPixel2.u;
            int n8 = kPixel2.v;
            int n9 = kPixel2.r;
            int n10 = (kPixel4.u - n7) / n2;
            int n11 = (kPixel4.v - n8) / n2;
            int n12 = (kPixel4.r - n9) / n2;
            int n13 = kPixel3.u;
            int n14 = kPixel3.v;
            int n15 = kPixel3.r;
            int n16 = (kPixel5.u - n13) / n2;
            int n17 = (kPixel5.v - n14) / n2;
            int n18 = (kPixel5.r - n15) / n2;
            int n19 = 0;
            while (n19 < n2) {
                int n20 = n4;
                int n21 = n7;
                int n22 = n8;
                int n23 = n9;
                int n24 = (n13 - n21) / n;
                int n25 = (n14 - n22) / n;
                int n26 = (n15 - n23) / n;
                int n27 = 0;
                while (n27 < n) {
                    int n28 = nArray2[n22 >> 8 & 0xFF00 | n21 >> 16 & 0xFF];
                    int n29 = n23 >> 16;
                    int n30 = ((n28 & 0xFF0000) >> 16) + n29;
                    int n31 = ((n28 & 0xFF00) >> 8) + n29;
                    n28 = (n28 & 0xFF) + n29;
                    if (n30 > n6) {
                        n30 = n6;
                    } else if (n30 < n5) {
                        n30 = n5;
                    }
                    if (n31 > n6) {
                        n31 = n6;
                    } else if (n31 < n5) {
                        n31 = n5;
                    }
                    if (n28 > n6) {
                        n28 = n6;
                    } else if (n28 < n5) {
                        n28 = n5;
                    }
                    nArray[n20++] = 0xFF000000 | n30 << 16 | n31 << 8 | n28;
                    n21 += n24;
                    n22 += n25;
                    n23 += n26;
                    ++n27;
                }
                n7 += n10;
                n8 += n11;
                n9 += n12;
                n13 += n16;
                n14 += n17;
                n15 += n18;
                n4 += n3;
                ++n19;
            }
        }

        public void drawrecUVM(kTexture kTexture2, kRenderContext kRenderContext2, kPixel kPixel2, kPixel kPixel3, kPixel kPixel4, kPixel kPixel5) {
            int n = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            int n2 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            int n3 = kRenderContext2.Modulo;
            int n4 = kRenderContext2.ClipX1 + kRenderContext2.ClipY1 * kRenderContext2.Modulo;
            int[] nArray = ((kRenderContext)kRenderContext2).RC_RootContext.CurrentChunk;
            int[] nArray2 = kTexture2.pixels;
            boolean bl = false;
            int n5 = 255;
            int n6 = kTexture2.Modulo;
            if (n == 0) {
                return;
            }
            if (n2 == 0) {
                return;
            }
            int n7 = kPixel2.u;
            int n8 = kPixel2.v;
            int n9 = (kPixel4.u - n7) / n2;
            int n10 = (kPixel4.v - n8) / n2;
            int n11 = kPixel3.u;
            int n12 = kPixel3.v;
            int n13 = (kPixel5.u - n11) / n2;
            int n14 = (kPixel5.v - n12) / n2;
            int n15 = 0;
            while (n15 < n2) {
                int n16 = n4;
                int n17 = n7;
                int n18 = n8;
                int n19 = (n11 - n17) / n;
                int n20 = (n12 - n18) / n;
                int n21 = 0;
                while (n21 < n) {
                    int n22 = nArray2[(n18 >> 16) * n6 + (n17 >> 16)];
                    int n23 = nArray[n16];
                    int n24 = ((n22 & 0xFF0000) >> 16) + ((n23 & 0xFF0000) >> 16);
                    int n25 = ((n22 & 0xFF00) >> 8) + ((n23 & 0xFF00) >> 8);
                    n22 = (n22 & 0xFF) + (n23 & 0xFF);
                    if (n24 > n5) {
                        n24 = n5;
                    }
                    if (n25 > n5) {
                        n25 = n5;
                    }
                    if (n22 > n5) {
                        n22 = n5;
                    }
                    nArray[n16++] = 0xFF000000 | n24 << 16 | n25 << 8 | n22;
                    n17 += n19;
                    n18 += n20;
                    ++n21;
                }
                n7 += n9;
                n8 += n10;
                n11 += n13;
                n12 += n14;
                n4 += n3;
                ++n15;
            }
        }

        public void drawrecUVRGB(kTexture kTexture2, kRenderContext kRenderContext2, kPixel kPixel2, kPixel kPixel3, kPixel kPixel4, kPixel kPixel5) {
            int n = 0;
            int n2 = 255;
            int n3 = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            int n4 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            int n5 = kRenderContext2.Modulo;
            int n6 = kRenderContext2.ClipX1 + kRenderContext2.ClipY1 * kRenderContext2.Modulo;
            int[] nArray = ((kRenderContext)kRenderContext2).RC_RootContext.CurrentChunk;
            int[] nArray2 = kTexture2.pixels;
            int n7 = kPixel2.u;
            int n8 = kPixel2.v;
            int n9 = kPixel2.r;
            int n10 = kPixel2.g;
            int n11 = kPixel2.b;
            int n12 = (kPixel4.u - n7) / n4;
            int n13 = (kPixel4.v - n8) / n4;
            int n14 = (kPixel4.r - n9) / n4;
            int n15 = (kPixel4.g - n10) / n4;
            int n16 = (kPixel4.b - n11) / n4;
            int n17 = kPixel3.u;
            int n18 = kPixel3.v;
            int n19 = kPixel3.r;
            int n20 = kPixel3.g;
            int n21 = kPixel3.b;
            int n22 = (kPixel5.u - n17) / n4;
            int n23 = (kPixel5.v - n18) / n4;
            int n24 = (kPixel5.r - n19) / n4;
            int n25 = (kPixel5.g - n20) / n4;
            int n26 = (kPixel5.b - n21) / n4;
            int n27 = 0;
            while (n27 < n4) {
                int n28 = n6;
                int n29 = n7;
                int n30 = n8;
                int n31 = n9;
                int n32 = n10;
                int n33 = n11;
                int n34 = (n17 - n29) / n3;
                int n35 = (n18 - n30) / n3;
                int n36 = (n19 - n31) / n3;
                int n37 = (n20 - n32) / n3;
                int n38 = (n21 - n33) / n3;
                int n39 = 0;
                while (n39 < n3) {
                    int n40 = nArray2[n30 >> 8 & 0xFF00 | n29 >> 16 & 0xFF];
                    int n41 = (n31 & 0xFFFF0000) >> 16;
                    int n42 = (n32 & 0xFFFF0000) >> 16;
                    int n43 = (n33 & 0xFFFF0000) >> 16;
                    int n44 = ((n40 & 0xFF0000) >> 16) + n41;
                    int n45 = ((n40 & 0xFF00) >> 8) + n42;
                    n40 = (n40 & 0xFF) + n43;
                    if (n44 > n2) {
                        n44 = n2;
                    } else if (n44 < n) {
                        n44 = n;
                    }
                    if (n45 > n2) {
                        n45 = n2;
                    } else if (n45 < n) {
                        n45 = n;
                    }
                    if (n40 > n2) {
                        n40 = n2;
                    } else if (n40 < n) {
                        n40 = n;
                    }
                    nArray[n28++] = 0xFF000000 | n44 << 16 | n45 << 8 | n40;
                    n29 += n34;
                    n30 += n35;
                    n31 += n36;
                    n32 += n37;
                    n33 += n38;
                    ++n39;
                }
                n7 += n12;
                n8 += n13;
                n9 += n14;
                n10 += n15;
                n11 += n16;
                n17 += n22;
                n18 += n23;
                n19 += n24;
                n20 += n25;
                n21 += n26;
                n6 += n5;
                ++n27;
            }
        }

        public void drawrecUVS(kTexture kTexture2, kRenderContext kRenderContext2, kPixel kPixel2, kPixel kPixel3, kPixel kPixel4, kPixel kPixel5) {
            int n = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            int n2 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            int n3 = kRenderContext2.Modulo;
            int n4 = kRenderContext2.ClipX1 + kRenderContext2.ClipY1 * kRenderContext2.Modulo;
            int[] nArray = ((kRenderContext)kRenderContext2).RC_RootContext.CurrentChunk;
            int[] nArray2 = kTexture2.pixels;
            int n5 = kTexture2.Modulo;
            if (n == 0) {
                return;
            }
            if (n2 == 0) {
                return;
            }
            int n6 = kPixel2.u;
            int n7 = kPixel2.v;
            int n8 = (kPixel4.u - n6) / n2;
            int n9 = (kPixel4.v - n7) / n2;
            int n10 = kPixel3.u;
            int n11 = kPixel3.v;
            int n12 = (kPixel5.u - n10) / n2;
            int n13 = (kPixel5.v - n11) / n2;
            int n14 = 0;
            while (n14 < n2) {
                int n15 = n4;
                int n16 = n6;
                int n17 = n7;
                int n18 = (n10 - n16) / n;
                int n19 = (n11 - n17) / n;
                int n20 = 0;
                while (n20 < n) {
                    int n21 = nArray2[(n17 >> 16) * n5 + (n16 >> 16)];
                    if (n21 != -1) {
                        nArray[n15] = n21;
                    }
                    ++n15;
                    n16 += n18;
                    n17 += n19;
                    ++n20;
                }
                n6 += n8;
                n7 += n9;
                n10 += n12;
                n11 += n13;
                n4 += n3;
                ++n14;
            }
        }

        public void drawrecUVSA(kTexture kTexture2, kRenderContext kRenderContext2, kPixel kPixel2, kPixel kPixel3, kPixel kPixel4, kPixel kPixel5, int n) {
            if (n <= 0) {
                return;
            }
            if (n >= 255) {
                this.drawrecUVS(kTexture2, kRenderContext2, kPixel2, kPixel3, kPixel4, kPixel5);
                return;
            }
            int n2 = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            int n3 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            int n4 = kRenderContext2.Modulo;
            int n5 = kRenderContext2.ClipX1 + kRenderContext2.ClipY1 * kRenderContext2.Modulo;
            int[] nArray = ((kRenderContext)kRenderContext2).RC_RootContext.CurrentChunk;
            int[] nArray2 = kTexture2.pixels;
            int n6 = kTexture2.Modulo;
            int n7 = 255 - n;
            if (n2 == 0) {
                return;
            }
            if (n3 == 0) {
                return;
            }
            int n8 = kPixel2.u;
            int n9 = kPixel2.v;
            int n10 = (kPixel4.u - n8) / n3;
            int n11 = (kPixel4.v - n9) / n3;
            int n12 = kPixel3.u;
            int n13 = kPixel3.v;
            int n14 = (kPixel5.u - n12) / n3;
            int n15 = (kPixel5.v - n13) / n3;
            int n16 = 0;
            while (n16 < n3) {
                int n17 = n5;
                int n18 = n8;
                int n19 = n9;
                int n20 = (n12 - n18) / n2;
                int n21 = (n13 - n19) / n2;
                int n22 = 0;
                while (n22 < n2) {
                    int n23 = nArray2[(n19 >> 16) * n6 + (n18 >> 16)];
                    if (n23 != -1) {
                        int n24 = nArray[n17];
                        int n25 = (n23 & 0xFF0000) >> 16;
                        int n26 = (n23 & 0xFF00) >> 8;
                        n23 &= 0xFF;
                        int n27 = (n24 & 0xFF0000) >> 16;
                        int n28 = (n24 & 0xFF00) >> 8;
                        n24 &= 0xFF;
                        n27 = n27 * n7 + n25 * n >> 8;
                        n28 = n28 * n7 + n26 * n >> 8;
                        n24 = n24 * n7 + n23 * n >> 8;
                        nArray[n17] = 0xFF000000 | n27 << 16 | n28 << 8 | n24;
                    }
                    ++n17;
                    n18 += n20;
                    n19 += n21;
                    ++n22;
                }
                n8 += n10;
                n9 += n11;
                n12 += n14;
                n13 += n15;
                n5 += n4;
                ++n16;
            }
        }

        public void findfloor(double d, double d2, double d3, double d4, kRenderContext kRenderContext2, helpfade helpfade2) {
            if (!kRenderContext2.drawable) {
                return;
            }
            double d5 = 65536.0;
            helpfade2.Frv1 *= d5;
            helpfade2.Frv2 *= d5;
            helpfade2.Fgv1 *= d5;
            helpfade2.Fgv2 *= d5;
            helpfade2.Fbv1 *= d5;
            helpfade2.Fbv2 *= d5;
            d3 *= 65536.0;
            d4 *= 65536.0;
            d += kRenderContext2.PosY;
            d2 -= kRenderContext2.PosY;
            int n = 0;
            while (n < kRenderContext2.MaxPreY) {
                int n2 = 0;
                while (n2 < kRenderContext2.MaxPreX) {
                    double d6;
                    double d7;
                    double d8;
                    kPixel kPixel2 = kRenderContext2.Pretable[n2][n];
                    double d9 = kPixel2.XF;
                    double d10 = kPixel2.YF;
                    double d11 = kPixel2.ZF;
                    if (d10 == 0.0) {
                        d10 = 1.0E-8;
                    }
                    double d12 = 1.0 / d10;
                    if (d10 < 0.0) {
                        d8 = d11 * d * d12;
                        d7 = d9 * d * d12;
                        kPixel2.u = (int)((d8 - kRenderContext2.PosZ) * d3);
                        kPixel2.v = (int)((d7 - kRenderContext2.PosX) * d4);
                        d6 = Math.sqrt(d8 * d8 + d7 * d7 + d * d);
                        this.colorclip(helpfade2, kPixel2, d6);
                    } else {
                        d8 = d11 * d2 * d12;
                        d7 = d9 * d2 * d12;
                        kPixel2.u = (int)((-d8 - kRenderContext2.PosZ) * d3);
                        kPixel2.v = (int)((-d7 - kRenderContext2.PosX) * d4);
                        d6 = Math.sqrt(d8 * d8 + d7 * d7 + d2 * d2);
                        this.colorclip(helpfade2, kPixel2, d6);
                    }
                    ++n2;
                }
                ++n;
            }
        }

        private void findtunnel(double d, double d2, kRenderContext kRenderContext2, helpfade helpfade2) {
            if (!kRenderContext2.drawable) {
                return;
            }
            double d3 = 65536.0;
            helpfade2.Frv1 *= d3;
            helpfade2.Frv2 *= d3;
            helpfade2.Fgv1 *= d3;
            helpfade2.Fgv2 *= d3;
            helpfade2.Fbv1 *= d3;
            helpfade2.Fbv2 *= d3;
            d2 *= 65536.0;
            double d4 = 5340353.715440872;
            int n = 0;
            while (n < kRenderContext2.MaxPreY) {
                int n2 = 0;
                while (n2 < kRenderContext2.MaxPreX) {
                    kPixel kPixel2 = kRenderContext2.Pretable[n2][n];
                    double d5 = kPixel2.XF;
                    double d6 = Math.abs(kPixel2.YF);
                    double d7 = kPixel2.ZF;
                    double d8 = Math.sqrt(d5 * d5 + d6 * d6);
                    if (d8 == 0.0) {
                        d8 = 1.0E-7;
                    }
                    double d9 = Math.atan2(d5, d6);
                    double d10 = Math.cos(d9) * d;
                    double d11 = Math.sin(d9) * d;
                    double d12 = d7 * d / d8;
                    kPixel2.u = (int)((d12 + kRenderContext2.PosZ) * d2);
                    kPixel2.v = (int)(d9 * d4) + 0x800000;
                    d8 = Math.sqrt(d12 * d12 + d10 * d10 + d11 * d11);
                    this.colorclip(helpfade2, kPixel2, d8);
                    ++n2;
                }
                ++n;
            }
        }

        private void findtwirl(double d, double d2, double d3, double d4, double d5, double d6, kRenderContext kRenderContext2) {
            if (!kRenderContext2.drawable) {
                return;
            }
            double d7 = 5.0;
            double d8 = 750.0;
            double d9 = d8 - d7;
            int n = (int)(kRenderContext2.PosZ * 65536.0) & 0xFFFFFF;
            double d10 = 5340353.715440872;
            int n2 = 0;
            while (n2 < kRenderContext2.MaxPreY) {
                int n3 = 0;
                while (n3 < kRenderContext2.MaxPreX) {
                    kPixel kPixel2 = kRenderContext2.Pretable[n3][n2];
                    double d11 = kPixel2.XF * 256.0;
                    double d12 = kPixel2.YF * 256.0;
                    double d13 = Math.atan2(d11, d12);
                    double d14 = Math.sqrt(d11 * d11 + d12 * d12);
                    d13 += d3 * Math.sin((d + d14 * Math.PI / 128.0) * d2);
                    d14 += d6 * Math.sin((d4 + d14 * Math.PI / 128.0) * d5);
                    kPixel2.u = (int)(Math.sin(d13) * d14 * 65536.0 + kRenderContext2.PosX * 65536.0);
                    kPixel2.v = (int)(Math.cos(d13) * d14 * 65536.0 + kRenderContext2.PosY * 65536.0);
                    double d15 = d12 - 8.0;
                    d13 = Math.atan2(d11, d15);
                    d14 = Math.sqrt(d11 * d11 + d15 * d15);
                    d13 += d3 * Math.sin((d + d14 * Math.PI / 128.0) * d2);
                    d14 += d6 * Math.sin((d4 + d14 * Math.PI / 128.0) * d5);
                    int n4 = kPixel2.u - (int)(Math.sin(d13) * d14 * 65536.0 + kRenderContext2.PosX * 65536.0);
                    kPixel2.r = n4 * 26;
                    kPixel2.g = n4 * 24;
                    kPixel2.b = n4 * 23;
                    ++n3;
                }
                ++n2;
            }
        }

        public void flare3D(double d, double d2, double d3, double d4, double d5, double d6, kRenderContext kRenderContext2, kTexture kTexture2) {
            double d7;
            double d8;
            double d9;
            if (!kRenderContext2.drawable) {
                return;
            }
            double d10 = kRenderContext2.ClipX2 - kRenderContext2.ClipX1;
            double d11 = kRenderContext2.ClipY2 - kRenderContext2.ClipY1;
            double d12 = d6 / d3;
            if (d3 <= 0.01) {
                return;
            }
            double d13 = (double)kTexture2.Modulo / d10;
            double d14 = (double)kTexture2.Height / d11;
            double d15 = d9 = d * d12;
            double d16 = d8 = d2 * d12;
            double d17 = 0.0;
            double d18 = 0.0;
            double d19 = kTexture2.Modulo - 1;
            double d20 = kTexture2.Height - 1;
            d13 = d13 * 0.5 * d4 * d12;
            d14 = d14 * 0.5 * d5 * d12;
            d9 -= d13;
            d8 -= d14;
            d15 += d13;
            d16 += d14;
            d9 = d9 * d10 + (double)kRenderContext2.ClipX1 + d10 * 0.5;
            d8 = d8 * d11 + (double)kRenderContext2.ClipY1 + d11 * 0.5;
            d15 = d15 * d10 + (double)kRenderContext2.ClipX1 + d10 * 0.5;
            d16 = d16 * d11 + (double)kRenderContext2.ClipY1 + d11 * 0.5;
            if (d9 > d15) {
                d7 = d15;
                d15 = d9;
                d9 = d7;
                d7 = d19;
                d19 = d17;
                d17 = d7;
            }
            if (d8 > d16) {
                d7 = d16;
                d16 = d8;
                d8 = d7;
                d7 = d20;
                d20 = d18;
                d18 = d7;
            }
            if (d15 > (double)kRenderContext2.ClipX2) {
                d19 = ((double)kRenderContext2.ClipX2 - d9) * (d19 - d17) / (d15 - d9);
                d15 = kRenderContext2.ClipX2;
            }
            if (d9 < (double)kRenderContext2.ClipX1) {
                d17 = ((double)kRenderContext2.ClipX1 - d9) * (d19 - d17) / (d15 - d9);
                d9 = kRenderContext2.ClipX1;
            }
            if (d16 > (double)kRenderContext2.ClipY2) {
                d20 = ((double)kRenderContext2.ClipY2 - d8) * (d20 - d18) / (d16 - d8);
                d16 = kRenderContext2.ClipY2;
            }
            if (d8 < (double)kRenderContext2.ClipY1) {
                d18 = ((double)kRenderContext2.ClipY1 - d8) * (d20 - d18) / (d16 - d8);
                d8 = kRenderContext2.ClipY1;
            }
            ((kRenderContext)this.Rootrc).Pretable[0][1].u = ((kRenderContext)this.Rootrc).Pretable[0][0].u = (int)(d17 * 65536.0);
            ((kRenderContext)this.Rootrc).Pretable[1][1].u = ((kRenderContext)this.Rootrc).Pretable[1][0].u = (int)(d19 * 65536.0);
            ((kRenderContext)this.Rootrc).Pretable[1][0].v = ((kRenderContext)this.Rootrc).Pretable[0][0].v = (int)(d18 * 65536.0);
            ((kRenderContext)this.Rootrc).Pretable[1][1].v = ((kRenderContext)this.Rootrc).Pretable[0][1].v = (int)(d20 * 65536.0);
            this.OneRec.SetSub(kRenderContext2, (int)d9, (int)d8, (int)d15, (int)d16);
            this.drawrecUVM(kTexture2, this.OneRec, this.Rootrc.Pretable[0][0], this.Rootrc.Pretable[1][0], this.Rootrc.Pretable[0][1], this.Rootrc.Pretable[1][1]);
        }
    }
}

