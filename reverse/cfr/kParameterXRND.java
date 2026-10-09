/*
 * Decompiled with CFR 0.152.
 */
class kParameterXRND
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

