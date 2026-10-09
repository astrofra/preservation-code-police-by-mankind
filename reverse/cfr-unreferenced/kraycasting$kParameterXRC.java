/*
 * Decompiled with CFR 0.152.
 */
class kraycasting.kParameterXRC
extends kraycasting.kParameterX {
    kraycasting.kParameterXRC() {
        super(kraycasting.this);
    }

    public kraycasting.kRenderContext RecOut() {
        if (this.kREC == null) {
            return kraycasting.this.RenderContext;
        }
        return this.kREC;
    }

    public kraycasting.kTexture TextureOut() {
        return null;
    }

    public double doubleout(double d) {
        return 0.0;
    }
}
