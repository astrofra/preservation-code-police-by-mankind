/*
 * Decompiled with CFR 0.152.
 */
class kraycasting.rotozoom {
    kraycasting.rotozoom() {
    }

    public void draw(kraycasting.kTexture kTexture2, kraycasting.kRenderContext kRenderContext2, kraycasting.k2Dpos k2Dpos2) {
        int n = kRenderContext2.ClipX2;
        int n2 = kRenderContext2.ClipY2;
        int[] nArray = kraycasting.kRenderContext.access$0((kraycasting.kRenderContext)kRenderContext2).CurrentChunk;
        int[] nArray2 = kTexture2.pixels;
        int n3 = 0;
        double d = Math.cos(k2Dpos2.Angle);
        double d2 = Math.sin(k2Dpos2.Angle);
        double d3 = k2Dpos2.Zoom * 65536.0;
        int n4 = (int)(-(d *= d3));
        int n5 = (int)(-(d2 *= d3));
        int n6 = (int)(-d2 * k2Dpos2.Xcentre + d * k2Dpos2.Ycentre + k2Dpos2.Utexture * 65536.0);
        int n7 = (int)(d * k2Dpos2.Xcentre + d2 * k2Dpos2.Ycentre + k2Dpos2.Vtexture * 65536.0);
        int n8 = (int)d2;
        int n9 = (int)(-d);
        int n10 = 0;
        while (n10 < n2) {
            int n11 = n6;
            int n12 = n7;
            int n13 = 0;
            while (n13 < n) {
                nArray[n3++] = nArray2[n11 >> 8 & 0xFF00 | n12 >> 16 & 0xFF];
                n11 += n8;
                n12 += n9;
                ++n13;
            }
            n6 += n4;
            n7 += n5;
            ++n10;
        }
    }
}
